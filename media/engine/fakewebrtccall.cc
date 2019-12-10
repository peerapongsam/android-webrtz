/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "media/engine/fakewebrtccall.h"

#include <algorithm>
#include <utility>

#include "api/call/audio_sink.h"
#include "media/base/rtputils.h"
#include "rtc_base/checks.h"
#include "rtc_base/gunit.h"
#include "rtc_base/platform_file.h"

namespace cricket {
FakeAudioSendStream::FakeAudioSendStream(
    int id, const webrtz::AudioSendStream::Config& config)
    : id_(id), config_(config) {
}

void FakeAudioSendStream::Reconfigure(
    const webrtz::AudioSendStream::Config& config) {
  config_ = config;
}

const webrtz::AudioSendStream::Config&
    FakeAudioSendStream::GetConfig() const {
  return config_;
}

void FakeAudioSendStream::SetStats(
    const webrtz::AudioSendStream::Stats& stats) {
  stats_ = stats;
}

FakeAudioSendStream::TelephoneEvent
    FakeAudioSendStream::GetLatestTelephoneEvent() const {
  return latest_telephone_event_;
}

bool FakeAudioSendStream::SendTelephoneEvent(int payload_type,
                                             int payload_frequency, int event,
                                             int duration_ms) {
  latest_telephone_event_.payload_type = payload_type;
  latest_telephone_event_.payload_frequency = payload_frequency;
  latest_telephone_event_.event_code = event;
  latest_telephone_event_.duration_ms = duration_ms;
  return true;
}

void FakeAudioSendStream::SetMuted(bool muted) {
  muted_ = muted;
}

webrtz::AudioSendStream::Stats FakeAudioSendStream::GetStats() const {
  return stats_;
}

webrtz::AudioSendStream::Stats FakeAudioSendStream::GetStats(
    bool /*has_remote_tracks*/) const {
  return stats_;
}

FakeAudioReceiveStream::FakeAudioReceiveStream(
    int id, const webrtz::AudioReceiveStream::Config& config)
    : id_(id), config_(config) {
}

const webrtz::AudioReceiveStream::Config&
    FakeAudioReceiveStream::GetConfig() const {
  return config_;
}

void FakeAudioReceiveStream::SetStats(
    const webrtz::AudioReceiveStream::Stats& stats) {
  stats_ = stats;
}

bool FakeAudioReceiveStream::VerifyLastPacket(const uint8_t* data,
                                              size_t length) const {
  return last_packet_ == rtc::Buffer(data, length);
}

bool FakeAudioReceiveStream::DeliverRtp(const uint8_t* packet,
                                        size_t length,
                                        const webrtz::PacketTime& packet_time) {
  ++received_packets_;
  last_packet_.SetData(packet, length);
  return true;
}

void FakeAudioReceiveStream::Reconfigure(
    const webrtz::AudioReceiveStream::Config& config) {
  config_ = config;
}

webrtz::AudioReceiveStream::Stats FakeAudioReceiveStream::GetStats() const {
  return stats_;
}

void FakeAudioReceiveStream::SetSink(webrtz::AudioSinkInterface* sink) {
  sink_ = sink;
}

void FakeAudioReceiveStream::SetGain(float gain) {
  gain_ = gain;
}

FakeVideoSendStream::FakeVideoSendStream(
    webrtz::VideoSendStream::Config config,
    webrtz::VideoEncoderConfig encoder_config)
    : sending_(false),
      config_(std::move(config)),
      codec_settings_set_(false),
      resolution_scaling_enabled_(false),
      framerate_scaling_enabled_(false),
      source_(nullptr),
      num_swapped_frames_(0) {
  RTC_DCHECK(config.encoder_settings.encoder != NULL);
  ReconfigureVideoEncoder(std::move(encoder_config));
}

FakeVideoSendStream::~FakeVideoSendStream() {
  if (source_)
    source_->RemoveSink(this);
}

const webrtz::VideoSendStream::Config& FakeVideoSendStream::GetConfig() const {
  return config_;
}

const webrtz::VideoEncoderConfig& FakeVideoSendStream::GetEncoderConfig()
    const {
  return encoder_config_;
}

const std::vector<webrtz::VideoStream>& FakeVideoSendStream::GetVideoStreams()
    const {
  return video_streams_;
}

bool FakeVideoSendStream::IsSending() const {
  return sending_;
}

bool FakeVideoSendStream::GetVp8Settings(
    webrtz::VideoCodecVP8* settings) const {
  if (!codec_settings_set_) {
    return false;
  }

  *settings = vpx_settings_.vp8;
  return true;
}

bool FakeVideoSendStream::GetVp9Settings(
    webrtz::VideoCodecVP9* settings) const {
  if (!codec_settings_set_) {
    return false;
  }

  *settings = vpx_settings_.vp9;
  return true;
}

int FakeVideoSendStream::GetNumberOfSwappedFrames() const {
  return num_swapped_frames_;
}

int FakeVideoSendStream::GetLastWidth() const {
  return last_frame_->width();
}

int FakeVideoSendStream::GetLastHeight() const {
  return last_frame_->height();
}

int64_t FakeVideoSendStream::GetLastTimestamp() const {
  RTC_DCHECK(last_frame_->ntp_time_ms() == 0);
  return last_frame_->render_time_ms();
}

void FakeVideoSendStream::OnFrame(const webrtz::VideoFrame& frame) {
  ++num_swapped_frames_;
  if (!last_frame_ ||
      frame.width() != last_frame_->width() ||
      frame.height() != last_frame_->height() ||
      frame.rotation() != last_frame_->rotation()) {
    video_streams_ = encoder_config_.video_stream_factory->CreateEncoderStreams(
        frame.width(), frame.height(), encoder_config_);
  }
  last_frame_ = frame;
}

void FakeVideoSendStream::SetStats(
    const webrtz::VideoSendStream::Stats& stats) {
  stats_ = stats;
}

webrtz::VideoSendStream::Stats FakeVideoSendStream::GetStats() {
  return stats_;
}

void FakeVideoSendStream::EnableEncodedFrameRecording(
    const std::vector<rtc::PlatformFile>& files,
    size_t byte_limit) {
  for (rtc::PlatformFile file : files)
    rtc::ClosePlatformFile(file);
}

void FakeVideoSendStream::ReconfigureVideoEncoder(
    webrtz::VideoEncoderConfig config) {
  int width, height;
  if (last_frame_) {
    width = last_frame_->width();
    height = last_frame_->height();
  } else {
    width = height = 0;
  }
  video_streams_ = config.video_stream_factory->CreateEncoderStreams(
      width, height, config);
  if (config.encoder_specific_settings != NULL) {
    const unsigned char num_temporal_layers = static_cast<unsigned char>(
        video_streams_.back().num_temporal_layers.value_or(1));
    if (config_.rtp.payload_name == "VP8") {
      config.encoder_specific_settings->FillVideoCodecVp8(&vpx_settings_.vp8);
      if (!video_streams_.empty()) {
        vpx_settings_.vp8.numberOfTemporalLayers = num_temporal_layers;
      }
    } else if (config_.rtp.payload_name == "VP9") {
      config.encoder_specific_settings->FillVideoCodecVp9(&vpx_settings_.vp9);
      if (!video_streams_.empty()) {
        vpx_settings_.vp9.numberOfTemporalLayers = num_temporal_layers;
      }
    } else {
      ADD_FAILURE() << "Unsupported encoder payload: "
                    << config_.rtp.payload_name;
    }
  }
  codec_settings_set_ = config.encoder_specific_settings != NULL;
  encoder_config_ = std::move(config);
  ++num_encoder_reconfigurations_;
}

void FakeVideoSendStream::UpdateActiveSimulcastLayers(
    const std::vector<bool> active_layers) {
  sending_ = false;
  for (const bool active_layer : active_layers) {
    if (active_layer) {
      sending_ = true;
      break;
    }
  }
}

void FakeVideoSendStream::Start() {
  sending_ = true;
}

void FakeVideoSendStream::Stop() {
  sending_ = false;
}

void FakeVideoSendStream::SetSource(
    rtc::VideoSourceInterface<webrtz::VideoFrame>* source,
    const webrtz::VideoSendStream::DegradationPreference&
        degradation_preference) {
  RTC_DCHECK(source != source_);
  if (source_)
    source_->RemoveSink(this);
  source_ = source;
  switch (degradation_preference) {
    case DegradationPreference::kMaintainFramerate:
      resolution_scaling_enabled_ = true;
      framerate_scaling_enabled_ = false;
      break;
    case DegradationPreference::kMaintainResolution:
      resolution_scaling_enabled_ = false;
      framerate_scaling_enabled_ = true;
      break;
    case DegradationPreference::kBalanced:
      resolution_scaling_enabled_ = true;
      framerate_scaling_enabled_ = true;
      break;
    case DegradationPreference::kDegradationDisabled:
      resolution_scaling_enabled_ = false;
      framerate_scaling_enabled_ = false;
      break;
  }
  if (source)
    source->AddOrUpdateSink(this, resolution_scaling_enabled_
                                      ? sink_wants_
                                      : rtc::VideoSinkWants());
}

void FakeVideoSendStream::InjectVideoSinkWants(
    const rtc::VideoSinkWants& wants) {
  sink_wants_ = wants;
  source_->AddOrUpdateSink(this, wants);
}

FakeVideoReceiveStream::FakeVideoReceiveStream(
    webrtz::VideoReceiveStream::Config config)
    : config_(std::move(config)),
      receiving_(false),
      num_added_secondary_sinks_(0),
      num_removed_secondary_sinks_(0) {}

const webrtz::VideoReceiveStream::Config& FakeVideoReceiveStream::GetConfig()
    const {
  return config_;
}

bool FakeVideoReceiveStream::IsReceiving() const {
  return receiving_;
}

void FakeVideoReceiveStream::InjectFrame(const webrtz::VideoFrame& frame) {
  config_.renderer->OnFrame(frame);
}

webrtz::VideoReceiveStream::Stats FakeVideoReceiveStream::GetStats() const {
  return stats_;
}

void FakeVideoReceiveStream::Start() {
  receiving_ = true;
}

void FakeVideoReceiveStream::Stop() {
  receiving_ = false;
}

void FakeVideoReceiveStream::SetStats(
    const webrtz::VideoReceiveStream::Stats& stats) {
  stats_ = stats;
}

void FakeVideoReceiveStream::EnableEncodedFrameRecording(rtc::PlatformFile file,
                                                         size_t byte_limit) {
  rtc::ClosePlatformFile(file);
}

void FakeVideoReceiveStream::AddSecondarySink(
    webrtz::RtpPacketSinkInterface* sink) {
  ++num_added_secondary_sinks_;
}

void FakeVideoReceiveStream::RemoveSecondarySink(
    const webrtz::RtpPacketSinkInterface* sink) {
  ++num_removed_secondary_sinks_;
}

int FakeVideoReceiveStream::GetNumAddedSecondarySinks() const {
  return num_added_secondary_sinks_;
}

int FakeVideoReceiveStream::GetNumRemovedSecondarySinks() const {
  return num_removed_secondary_sinks_;
}

FakeFlexfecReceiveStream::FakeFlexfecReceiveStream(
    const webrtz::FlexfecReceiveStream::Config& config)
    : config_(config) {}

const webrtz::FlexfecReceiveStream::Config&
FakeFlexfecReceiveStream::GetConfig() const {
  return config_;
}

// TODO(brandtr): Implement when the stats have been designed.
webrtz::FlexfecReceiveStream::Stats FakeFlexfecReceiveStream::GetStats() const {
  return webrtz::FlexfecReceiveStream::Stats();
}

void FakeFlexfecReceiveStream::OnRtpPacket(const webrtz::RtpPacketReceived&) {
  RTC_NOTREACHED() << "Not implemented.";
}

FakeCall::FakeCall()
    : audio_network_state_(webrtz::kNetworkUp),
      video_network_state_(webrtz::kNetworkUp),
      num_created_send_streams_(0),
      num_created_receive_streams_(0),
      audio_transport_overhead_(0),
      video_transport_overhead_(0) {}

FakeCall::~FakeCall() {
  EXPECT_EQ(0u, video_send_streams_.size());
  EXPECT_EQ(0u, audio_send_streams_.size());
  EXPECT_EQ(0u, video_receive_streams_.size());
  EXPECT_EQ(0u, audio_receive_streams_.size());
}

const std::vector<FakeVideoSendStream*>& FakeCall::GetVideoSendStreams() {
  return video_send_streams_;
}

const std::vector<FakeVideoReceiveStream*>& FakeCall::GetVideoReceiveStreams() {
  return video_receive_streams_;
}

const std::vector<FakeAudioSendStream*>& FakeCall::GetAudioSendStreams() {
  return audio_send_streams_;
}

const FakeAudioSendStream* FakeCall::GetAudioSendStream(uint32_t ssrc) {
  for (const auto* p : GetAudioSendStreams()) {
    if (p->GetConfig().rtp.ssrc == ssrc) {
      return p;
    }
  }
  return nullptr;
}

const std::vector<FakeAudioReceiveStream*>& FakeCall::GetAudioReceiveStreams() {
  return audio_receive_streams_;
}

const FakeAudioReceiveStream* FakeCall::GetAudioReceiveStream(uint32_t ssrc) {
  for (const auto* p : GetAudioReceiveStreams()) {
    if (p->GetConfig().rtp.remote_ssrc == ssrc) {
      return p;
    }
  }
  return nullptr;
}

const std::vector<FakeFlexfecReceiveStream*>&
FakeCall::GetFlexfecReceiveStreams() {
  return flexfec_receive_streams_;
}

webrtz::NetworkState FakeCall::GetNetworkState(webrtz::MediaType media) const {
  switch (media) {
    case webrtz::MediaType::AUDIO:
      return audio_network_state_;
    case webrtz::MediaType::VIDEO:
      return video_network_state_;
    case webrtz::MediaType::DATA:
    case webrtz::MediaType::ANY:
      ADD_FAILURE() << "GetNetworkState called with unknown parameter.";
      return webrtz::kNetworkDown;
  }
  // Even though all the values for the enum class are listed above,the compiler
  // will emit a warning as the method may be called with a value outside of the
  // valid enum range, unless this case is also handled.
  ADD_FAILURE() << "GetNetworkState called with unknown parameter.";
  return webrtz::kNetworkDown;
}

webrtz::AudioSendStream* FakeCall::CreateAudioSendStream(
    const webrtz::AudioSendStream::Config& config) {
  FakeAudioSendStream* fake_stream = new FakeAudioSendStream(next_stream_id_++,
                                                             config);
  audio_send_streams_.push_back(fake_stream);
  ++num_created_send_streams_;
  return fake_stream;
}

void FakeCall::DestroyAudioSendStream(webrtz::AudioSendStream* send_stream) {
  auto it = std::find(audio_send_streams_.begin(),
                      audio_send_streams_.end(),
                      static_cast<FakeAudioSendStream*>(send_stream));
  if (it == audio_send_streams_.end()) {
    ADD_FAILURE() << "DestroyAudioSendStream called with unknown parameter.";
  } else {
    delete *it;
    audio_send_streams_.erase(it);
  }
}

webrtz::AudioReceiveStream* FakeCall::CreateAudioReceiveStream(
    const webrtz::AudioReceiveStream::Config& config) {
  audio_receive_streams_.push_back(new FakeAudioReceiveStream(next_stream_id_++,
                                                              config));
  ++num_created_receive_streams_;
  return audio_receive_streams_.back();
}

void FakeCall::DestroyAudioReceiveStream(
    webrtz::AudioReceiveStream* receive_stream) {
  auto it = std::find(audio_receive_streams_.begin(),
                      audio_receive_streams_.end(),
                      static_cast<FakeAudioReceiveStream*>(receive_stream));
  if (it == audio_receive_streams_.end()) {
    ADD_FAILURE() << "DestroyAudioReceiveStream called with unknown parameter.";
  } else {
    delete *it;
    audio_receive_streams_.erase(it);
  }
}

webrtz::VideoSendStream* FakeCall::CreateVideoSendStream(
    webrtz::VideoSendStream::Config config,
    webrtz::VideoEncoderConfig encoder_config) {
  FakeVideoSendStream* fake_stream =
      new FakeVideoSendStream(std::move(config), std::move(encoder_config));
  video_send_streams_.push_back(fake_stream);
  ++num_created_send_streams_;
  return fake_stream;
}

void FakeCall::DestroyVideoSendStream(webrtz::VideoSendStream* send_stream) {
  auto it = std::find(video_send_streams_.begin(),
                      video_send_streams_.end(),
                      static_cast<FakeVideoSendStream*>(send_stream));
  if (it == video_send_streams_.end()) {
    ADD_FAILURE() << "DestroyVideoSendStream called with unknown parameter.";
  } else {
    delete *it;
    video_send_streams_.erase(it);
  }
}

webrtz::VideoReceiveStream* FakeCall::CreateVideoReceiveStream(
    webrtz::VideoReceiveStream::Config config) {
  video_receive_streams_.push_back(
      new FakeVideoReceiveStream(std::move(config)));
  ++num_created_receive_streams_;
  return video_receive_streams_.back();
}

void FakeCall::DestroyVideoReceiveStream(
    webrtz::VideoReceiveStream* receive_stream) {
  auto it = std::find(video_receive_streams_.begin(),
                      video_receive_streams_.end(),
                      static_cast<FakeVideoReceiveStream*>(receive_stream));
  if (it == video_receive_streams_.end()) {
    ADD_FAILURE() << "DestroyVideoReceiveStream called with unknown parameter.";
  } else {
    delete *it;
    video_receive_streams_.erase(it);
  }
}

webrtz::FlexfecReceiveStream* FakeCall::CreateFlexfecReceiveStream(
    const webrtz::FlexfecReceiveStream::Config& config) {
  FakeFlexfecReceiveStream* fake_stream = new FakeFlexfecReceiveStream(config);
  flexfec_receive_streams_.push_back(fake_stream);
  ++num_created_receive_streams_;
  return fake_stream;
}

void FakeCall::DestroyFlexfecReceiveStream(
    webrtz::FlexfecReceiveStream* receive_stream) {
  auto it = std::find(flexfec_receive_streams_.begin(),
                      flexfec_receive_streams_.end(),
                      static_cast<FakeFlexfecReceiveStream*>(receive_stream));
  if (it == flexfec_receive_streams_.end()) {
    ADD_FAILURE()
        << "DestroyFlexfecReceiveStream called with unknown parameter.";
  } else {
    delete *it;
    flexfec_receive_streams_.erase(it);
  }
}

webrtz::PacketReceiver* FakeCall::Receiver() {
  return this;
}

FakeCall::DeliveryStatus FakeCall::DeliverPacket(
    webrtz::MediaType media_type,
    rtc::CopyOnWriteBuffer packet,
    const webrtz::PacketTime& packet_time) {
  EXPECT_GE(packet.size(), 12u);
  RTC_DCHECK(media_type == webrtz::MediaType::AUDIO ||
             media_type == webrtz::MediaType::VIDEO);

  uint32_t ssrc;
  if (!GetRtpSsrc(packet.cdata(), packet.size(), &ssrc))
    return DELIVERY_PACKET_ERROR;

  if (media_type == webrtz::MediaType::VIDEO) {
    for (auto receiver : video_receive_streams_) {
      if (receiver->GetConfig().rtp.remote_ssrc == ssrc)
        return DELIVERY_OK;
    }
  }
  if (media_type == webrtz::MediaType::AUDIO) {
    for (auto receiver : audio_receive_streams_) {
      if (receiver->GetConfig().rtp.remote_ssrc == ssrc) {
        receiver->DeliverRtp(packet.cdata(), packet.size(), packet_time);
        return DELIVERY_OK;
      }
    }
  }
  return DELIVERY_UNKNOWN_SSRC;
}

void FakeCall::SetStats(const webrtz::Call::Stats& stats) {
  stats_ = stats;
}

int FakeCall::GetNumCreatedSendStreams() const {
  return num_created_send_streams_;
}

int FakeCall::GetNumCreatedReceiveStreams() const {
  return num_created_receive_streams_;
}

webrtz::Call::Stats FakeCall::GetStats() const {
  return stats_;
}

void FakeCall::SetBitrateAllocationStrategy(
    std::unique_ptr<rtc::BitrateAllocationStrategy>
        bitrate_allocation_strategy) {
  // TODO(alexnarest): not implemented
}

void FakeCall::SignalChannelNetworkState(webrtz::MediaType media,
                                         webrtz::NetworkState state) {
  switch (media) {
    case webrtz::MediaType::AUDIO:
      audio_network_state_ = state;
      break;
    case webrtz::MediaType::VIDEO:
      video_network_state_ = state;
      break;
    case webrtz::MediaType::DATA:
    case webrtz::MediaType::ANY:
      ADD_FAILURE()
          << "SignalChannelNetworkState called with unknown parameter.";
  }
}

void FakeCall::OnTransportOverheadChanged(webrtz::MediaType media,
                                          int transport_overhead_per_packet) {
  switch (media) {
    case webrtz::MediaType::AUDIO:
      audio_transport_overhead_ = transport_overhead_per_packet;
      break;
    case webrtz::MediaType::VIDEO:
      video_transport_overhead_ = transport_overhead_per_packet;
      break;
    case webrtz::MediaType::DATA:
    case webrtz::MediaType::ANY:
      ADD_FAILURE()
          << "SignalChannelNetworkState called with unknown parameter.";
  }
}

void FakeCall::OnSentPacket(const rtc::SentPacket& sent_packet) {
  last_sent_packet_ = sent_packet;
  if (sent_packet.packet_id >= 0) {
    last_sent_nonnegative_packet_id_ = sent_packet.packet_id;
  }
}

}  // namespace cricket
