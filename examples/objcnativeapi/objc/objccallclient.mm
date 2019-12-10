/*
 *  Copyright 2018 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "examples/objcnativeapi/objc/objccallclient.h"

#include <utility>

#import <WebRTC/RTCCameraPreviewView.h>
#import <WebRTC/RTCVideoCodecFactory.h>
#import <WebRTC/RTCVideoRenderer.h>

#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/peerconnectioninterface.h"
#include "media/engine/webrtcmediaengine.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "rtc_base/ptr_util.h"
#include "sdk/objc/Framework/Native/api/video_capturer.h"
#include "sdk/objc/Framework/Native/api/video_decoder_factory.h"
#include "sdk/objc/Framework/Native/api/video_encoder_factory.h"
#include "sdk/objc/Framework/Native/api/video_renderer.h"

namespace webrtz_examples {

namespace {

class CreateOfferObserver : public webrtz::CreateSessionDescriptionObserver {
 public:
  explicit CreateOfferObserver(rtc::scoped_refptr<webrtz::PeerConnectionInterface> pc);

  void OnSuccess(webrtz::SessionDescriptionInterface* desc) override;
  void OnFailure(const std::string& error) override;

 private:
  const rtc::scoped_refptr<webrtz::PeerConnectionInterface> pc_;
};

class SetRemoteSessionDescriptionObserver : public webrtz::SetRemoteDescriptionObserverInterface {
 public:
  void OnSetRemoteDescriptionComplete(webrtz::RTCError error) override;
};

class SetLocalSessionDescriptionObserver : public webrtz::SetSessionDescriptionObserver {
 public:
  void OnSuccess() override;
  void OnFailure(const std::string& error) override;
};

}  // namespace

ObjCCallClient::ObjCCallClient()
    : call_started_(false), pc_observer_(rtc::MakeUnique<PCObserver>(this)) {
  thread_checker_.DetachFromThread();
  CreatePeerConnectionFactory();
}

void ObjCCallClient::Call(RTCVideoCapturer* capturer, id<RTCVideoRenderer> remote_renderer) {
  RTC_DCHECK_RUN_ON(&thread_checker_);

  rtc::CritScope lock(&pc_mutex_);
  if (call_started_) {
    RTC_LOG(LS_WARNING) << "Call already started.";
    return;
  }
  call_started_ = true;

  remote_sink_ = webrtz::ObjCToNativeVideoRenderer(remote_renderer);

  video_source_ =
      webrtz::ObjCToNativeVideoCapturer(capturer, signaling_thread_.get(), worker_thread_.get());

  CreatePeerConnection();
  Connect();
}

void ObjCCallClient::Hangup() {
  RTC_DCHECK_RUN_ON(&thread_checker_);

  call_started_ = false;

  {
    rtc::CritScope lock(&pc_mutex_);
    if (pc_ != nullptr) {
      pc_->Close();
      pc_ = nullptr;
    }
  }

  remote_sink_ = nullptr;
  video_source_ = nullptr;
}

void ObjCCallClient::CreatePeerConnectionFactory() {
  network_thread_ = rtc::Thread::CreateWithSocketServer();
  network_thread_->SetName("network_thread", nullptr);
  RTC_CHECK(network_thread_->Start()) << "Failed to start thread";

  worker_thread_ = rtc::Thread::Create();
  worker_thread_->SetName("worker_thread", nullptr);
  RTC_CHECK(worker_thread_->Start()) << "Failed to start thread";

  signaling_thread_ = rtc::Thread::Create();
  signaling_thread_->SetName("signaling_thread", nullptr);
  RTC_CHECK(signaling_thread_->Start()) << "Failed to start thread";

  std::unique_ptr<webrtz::VideoDecoderFactory> videoDecoderFactory =
      webrtz::ObjCToNativeVideoDecoderFactory([[RTCDefaultVideoDecoderFactory alloc] init]);
  std::unique_ptr<webrtz::VideoEncoderFactory> videoEncoderFactory =
      webrtz::ObjCToNativeVideoEncoderFactory([[RTCDefaultVideoEncoderFactory alloc] init]);

  std::unique_ptr<cricket::MediaEngineInterface> media_engine =
      cricket::WebRtcMediaEngineFactory::Create(nullptr /* adm */,
                                                webrtz::CreateBuiltinAudioEncoderFactory(),
                                                webrtz::CreateBuiltinAudioDecoderFactory(),
                                                std::move(videoEncoderFactory),
                                                std::move(videoDecoderFactory),
                                                nullptr /* audio_mixer */,
                                                webrtz::AudioProcessingBuilder().Create());
  RTC_LOG(LS_INFO) << "Media engine created: " << media_engine.get();

  pcf_ = webrtz::CreateModularPeerConnectionFactory(network_thread_.get(),
                                                    worker_thread_.get(),
                                                    signaling_thread_.get(),
                                                    std::move(media_engine),
                                                    webrtz::CreateCallFactory(),
                                                    webrtz::CreateRtcEventLogFactory());
  RTC_LOG(LS_INFO) << "PeerConnectionFactory created: " << pcf_;
}

void ObjCCallClient::CreatePeerConnection() {
  rtc::CritScope lock(&pc_mutex_);
  webrtz::PeerConnectionInterface::RTCConfiguration config;
  config.sdp_semantics = webrtz::SdpSemantics::kUnifiedPlan;
  // DTLS SRTP has to be disabled for loopback to work.
  config.enable_dtls_srtp = false;
  pc_ = pcf_->CreatePeerConnection(
      config, nullptr /* port_allocator */, nullptr /* cert_generator */, pc_observer_.get());
  RTC_LOG(LS_INFO) << "PeerConnection created: " << pc_;

  rtc::scoped_refptr<webrtz::VideoTrackInterface> local_video_track =
      pcf_->CreateVideoTrack("video", video_source_);
  pc_->AddTransceiver(local_video_track);
  RTC_LOG(LS_INFO) << "Local video sink set up: " << local_video_track;

  for (const rtc::scoped_refptr<webrtz::RtpTransceiverInterface>& tranceiver :
       pc_->GetTransceivers()) {
    rtc::scoped_refptr<webrtz::MediaStreamTrackInterface> track = tranceiver->receiver()->track();
    if (track && track->kind() == webrtz::MediaStreamTrackInterface::kVideoKind) {
      static_cast<webrtz::VideoTrackInterface*>(track.get())
          ->AddOrUpdateSink(remote_sink_.get(), rtc::VideoSinkWants());
      RTC_LOG(LS_INFO) << "Remote video sink set up: " << track;
      break;
    }
  }
}

void ObjCCallClient::Connect() {
  rtc::CritScope lock(&pc_mutex_);
  pc_->CreateOffer(new rtc::RefCountedObject<CreateOfferObserver>(pc_),
                   webrtz::PeerConnectionInterface::RTCOfferAnswerOptions());
}

ObjCCallClient::PCObserver::PCObserver(ObjCCallClient* client) : client_(client) {}

void ObjCCallClient::PCObserver::OnSignalingChange(
    webrtz::PeerConnectionInterface::SignalingState new_state) {
  RTC_LOG(LS_INFO) << "OnSignalingChange: " << new_state;
}

void ObjCCallClient::PCObserver::OnDataChannel(
    rtc::scoped_refptr<webrtz::DataChannelInterface> data_channel) {
  RTC_LOG(LS_INFO) << "OnDataChannel";
}

void ObjCCallClient::PCObserver::OnRenegotiationNeeded() {
  RTC_LOG(LS_INFO) << "OnRenegotiationNeeded";
}

void ObjCCallClient::PCObserver::OnIceConnectionChange(
    webrtz::PeerConnectionInterface::IceConnectionState new_state) {
  RTC_LOG(LS_INFO) << "OnIceConnectionChange: " << new_state;
}

void ObjCCallClient::PCObserver::OnIceGatheringChange(
    webrtz::PeerConnectionInterface::IceGatheringState new_state) {
  RTC_LOG(LS_INFO) << "OnIceGatheringChange: " << new_state;
}

void ObjCCallClient::PCObserver::OnIceCandidate(const webrtz::IceCandidateInterface* candidate) {
  RTC_LOG(LS_INFO) << "OnIceCandidate: " << candidate->server_url();
  rtc::CritScope lock(&client_->pc_mutex_);
  RTC_DCHECK(client_->pc_ != nullptr);
  client_->pc_->AddIceCandidate(candidate);
}

CreateOfferObserver::CreateOfferObserver(rtc::scoped_refptr<webrtz::PeerConnectionInterface> pc)
    : pc_(pc) {}

void CreateOfferObserver::OnSuccess(webrtz::SessionDescriptionInterface* desc) {
  std::string sdp;
  desc->ToString(&sdp);
  RTC_LOG(LS_INFO) << "Created offer: " << sdp;

  // Ownership of desc was transferred to us, now we transfer it forward.
  pc_->SetLocalDescription(new rtc::RefCountedObject<SetLocalSessionDescriptionObserver>(), desc);

  // Generate a fake answer.
  std::unique_ptr<webrtz::SessionDescriptionInterface> answer(
      webrtz::CreateSessionDescription(webrtz::SdpType::kAnswer, sdp));
  pc_->SetRemoteDescription(std::move(answer),
                            new rtc::RefCountedObject<SetRemoteSessionDescriptionObserver>());
}

void CreateOfferObserver::OnFailure(const std::string& error) {
  RTC_LOG(LS_INFO) << "Failed to create offer: " << error;
}

void SetRemoteSessionDescriptionObserver::OnSetRemoteDescriptionComplete(webrtz::RTCError error) {
  RTC_LOG(LS_INFO) << "Set remote description: " << error.message();
}

void SetLocalSessionDescriptionObserver::OnSuccess() {
  RTC_LOG(LS_INFO) << "Set local description success!";
}

void SetLocalSessionDescriptionObserver::OnFailure(const std::string& error) {
  RTC_LOG(LS_INFO) << "Set local description failure: " << error;
}

}  // namespace webrtz_examples
