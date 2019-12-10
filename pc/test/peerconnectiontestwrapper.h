/*
 *  Copyright 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef PC_TEST_PEERCONNECTIONTESTWRAPPER_H_
#define PC_TEST_PEERCONNECTIONTESTWRAPPER_H_

#include <memory>
#include <string>
#include <vector>

#include "api/peerconnectioninterface.h"
#include "api/test/fakeconstraints.h"
#include "pc/test/fakeaudiocapturemodule.h"
#include "pc/test/fakevideotrackrenderer.h"
#include "rtc_base/sigslot.h"

class PeerConnectionTestWrapper
    : public webrtz::PeerConnectionObserver,
      public webrtz::CreateSessionDescriptionObserver,
      public sigslot::has_slots<> {
 public:
  static void Connect(PeerConnectionTestWrapper* caller,
                      PeerConnectionTestWrapper* callee);

  PeerConnectionTestWrapper(const std::string& name,
                            rtc::Thread* network_thread,
                            rtc::Thread* worker_thread);
  virtual ~PeerConnectionTestWrapper();

  bool CreatePc(
      const webrtz::MediaConstraintsInterface* constraints,
      const webrtz::PeerConnectionInterface::RTCConfiguration& config,
      rtc::scoped_refptr<webrtz::AudioEncoderFactory> audio_encoder_factory,
      rtc::scoped_refptr<webrtz::AudioDecoderFactory> audio_decoder_factory);

  webrtz::PeerConnectionInterface* pc() { return peer_connection_.get(); }

  rtc::scoped_refptr<webrtz::DataChannelInterface> CreateDataChannel(
      const std::string& label,
      const webrtz::DataChannelInit& init);

  // Implements PeerConnectionObserver.
  void OnSignalingChange(
     webrtz::PeerConnectionInterface::SignalingState new_state) override {}
  void OnAddTrack(
      rtc::scoped_refptr<webrtz::RtpReceiverInterface> receiver,
      const std::vector<rtc::scoped_refptr<webrtz::MediaStreamInterface>>&
          streams) override;
  void OnDataChannel(
      rtc::scoped_refptr<webrtz::DataChannelInterface> data_channel) override;
  void OnRenegotiationNeeded() override {}
  void OnIceConnectionChange(
      webrtz::PeerConnectionInterface::IceConnectionState new_state) override {}
  void OnIceGatheringChange(
      webrtz::PeerConnectionInterface::IceGatheringState new_state) override {}
  void OnIceCandidate(const webrtz::IceCandidateInterface* candidate) override;

  // Implements CreateSessionDescriptionObserver.
  void OnSuccess(webrtz::SessionDescriptionInterface* desc) override;
  void OnFailure(webrtz::RTCError) override {}

  void CreateOffer(const webrtz::MediaConstraintsInterface* constraints);
  void CreateAnswer(const webrtz::MediaConstraintsInterface* constraints);
  void ReceiveOfferSdp(const std::string& sdp);
  void ReceiveAnswerSdp(const std::string& sdp);
  void AddIceCandidate(const std::string& sdp_mid, int sdp_mline_index,
                       const std::string& candidate);
  void WaitForCallEstablished();
  void WaitForConnection();
  void WaitForAudio();
  void WaitForVideo();
  void GetAndAddUserMedia(
    bool audio, const webrtz::FakeConstraints& audio_constraints,
    bool video, const webrtz::FakeConstraints& video_constraints);

  // sigslots
  sigslot::signal1<std::string*> SignalOnIceCandidateCreated;
  sigslot::signal3<const std::string&,
                   int,
                   const std::string&> SignalOnIceCandidateReady;
  sigslot::signal1<std::string*> SignalOnSdpCreated;
  sigslot::signal1<const std::string&> SignalOnSdpReady;
  sigslot::signal1<webrtz::DataChannelInterface*> SignalOnDataChannel;

 private:
  void SetLocalDescription(webrtz::SdpType type, const std::string& sdp);
  void SetRemoteDescription(webrtz::SdpType type, const std::string& sdp);
  bool CheckForConnection();
  bool CheckForAudio();
  bool CheckForVideo();
  rtc::scoped_refptr<webrtz::MediaStreamInterface> GetUserMedia(
      bool audio, const webrtz::FakeConstraints& audio_constraints,
      bool video, const webrtz::FakeConstraints& video_constraints);

  std::string name_;
  rtc::Thread* const network_thread_;
  rtc::Thread* const worker_thread_;
  rtc::scoped_refptr<webrtz::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtz::PeerConnectionFactoryInterface>
      peer_connection_factory_;
  rtc::scoped_refptr<FakeAudioCaptureModule> fake_audio_capture_module_;
  std::unique_ptr<webrtz::FakeVideoTrackRenderer> renderer_;
  int num_get_user_media_calls_ = 0;
};

#endif  // PC_TEST_PEERCONNECTIONTESTWRAPPER_H_
