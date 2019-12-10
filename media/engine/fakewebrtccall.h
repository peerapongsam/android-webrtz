/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// This file contains fake implementations, for use in unit tests, of the
// following classes:
//
//   webrtz::Call
//   webrtz::AudioSendStream
//   webrtz::AudioReceiveStream
//   webrtz::VideoSendStream
//   webrtz::VideoReceiveStream

#ifndef MEDIA_ENGINE_FAKEWEBRTCCALL_H_
#define MEDIA_ENGINE_FAKEWEBRTCCALL_H_

#include <memory>
#include <string>
#include <vector>

#include "api/video/video_frame.h"
#include "call/audio_receive_stream.h"
#include "call/audio_send_stream.h"
#include "call/call.h"
#include "call/flexfec_receive_stream.h"
#include "call/test/mock_rtp_transport_controller_send.h"
#include "call/video_receive_stream.h"
#include "call/video_send_stream.h"
#include "modules/rtp_rtcp/source/rtp_packet_received.h"
#include "rtc_base/buffer.h"

namespace cricket {
class FakeAudioSendStream final : public webrtz::AudioSendStream {
 public:
  struct TelephoneEvent {
    int payload_type = -1;
    int payload_frequency = -1;
    int event_code = 0;
    int duration_ms = 0;
  };

  explicit FakeAudioSendStream(
      int id, const webrtz::AudioSendStream::Config& config);

  int id() const { return id_; }
  const webrtz::AudioSendStream::Config& GetConfig() const override;
  void SetStats(const webrtz::AudioSendStream::Stats& stats);
  TelephoneEvent GetLatestTelephoneEvent() const;
  bool IsSending() const { return sending_; }
  bool muted() const { return muted_; }

 private:
  // webrtz::AudioSendStream implementation.
  void Reconfigure(const webrtz::AudioSendStream::Config& config) override;
  void Start() override { sending_ = true; }
  void Stop() override { sending_ = false; }
  void SendAudioData(std::unique_ptr<webrtz::AudioFrame> audio_frame) override {
  }
  bool SendTelephoneEvent(int payload_type, int payload_frequency, int event,
                          int duration_ms) override;
  void SetMuted(bool muted) override;
  webrtz::AudioSendStream::Stats GetStats() const override;
  webrtz::AudioSendStream::Stats GetStats(
      bool has_remote_tracks) const override;

  int id_ = -1;
  TelephoneEvent latest_telephone_event_;
  webrtz::AudioSendStream::Config config_;
  webrtz::AudioSendStream::Stats stats_;
  bool sending_ = false;
  bool muted_ = false;
};

class FakeAudioReceiveStream final : public webrtz::AudioReceiveStream {
 public:
  explicit FakeAudioReceiveStream(
      int id, const webrtz::AudioReceiveStream::Config& config);

  int id() const { return id_; }
  const webrtz::AudioReceiveStream::Config& GetConfig() const;
  void SetStats(const webrtz::AudioReceiveStream::Stats& stats);
  int received_packets() const { return received_packets_; }
  bool VerifyLastPacket(const uint8_t* data, size_t length) const;
  const webrtz::AudioSinkInterface* sink() const { return sink_; }
  float gain() const { return gain_; }
  bool DeliverRtp(const uint8_t* packet,
                  size_t length,
                  const webrtz::PacketTime& packet_time);
  bool started() const { return started_; }

 private:
  // webrtz::AudioReceiveStream implementation.
  void Reconfigure(const webrtz::AudioReceiveStream::Config& config) override;
  void Start() override { started_ = true; }
  void Stop() override { started_ = false; }

  webrtz::AudioReceiveStream::Stats GetStats() const override;
  void SetSink(webrtz::AudioSinkInterface* sink) override;
  void SetGain(float gain) override;
  std::vector<webrtz::RtpSource> GetSources() const override {
    return std::vector<webrtz::RtpSource>();
  }

  int id_ = -1;
  webrtz::AudioReceiveStream::Config config_;
  webrtz::AudioReceiveStream::Stats stats_;
  int received_packets_ = 0;
  webrtz::AudioSinkInterface* sink_ = nullptr;
  float gain_ = 1.0f;
  rtc::Buffer last_packet_;
  bool started_ = false;
};

class FakeVideoSendStream final
    : public webrtz::VideoSendStream,
      public rtc::VideoSinkInterface<webrtz::VideoFrame> {
 public:
  FakeVideoSendStream(webrtz::VideoSendStream::Config config,
                      webrtz::VideoEncoderConfig encoder_config);
  ~FakeVideoSendStream() override;
  const webrtz::VideoSendStream::Config& GetConfig() const;
  const webrtz::VideoEncoderConfig& GetEncoderConfig() const;
  const std::vector<webrtz::VideoStream>& GetVideoStreams() const;

  bool IsSending() const;
  bool GetVp8Settings(webrtz::VideoCodecVP8* settings) const;
  bool GetVp9Settings(webrtz::VideoCodecVP9* settings) const;

  int GetNumberOfSwappedFrames() const;
  int GetLastWidth() const;
  int GetLastHeight() const;
  int64_t GetLastTimestamp() const;
  void SetStats(const webrtz::VideoSendStream::Stats& stats);
  int num_encoder_reconfigurations() const {
    return num_encoder_reconfigurations_;
  }

  void EnableEncodedFrameRecording(const std::vector<rtc::PlatformFile>& files,
                                   size_t byte_limit) override;

  bool resolution_scaling_enabled() const {
    return resolution_scaling_enabled_;
  }
  bool framerate_scaling_enabled() const { return framerate_scaling_enabled_; }
  void InjectVideoSinkWants(const rtc::VideoSinkWants& wants);

  rtc::VideoSourceInterface<webrtz::VideoFrame>* source() const {
    return source_;
  }

 private:
  // rtc::VideoSinkInterface<VideoFrame> implementation.
  void OnFrame(const webrtz::VideoFrame& frame) override;

  // webrtz::VideoSendStream implementation.
  void UpdateActiveSimulcastLayers(
      const std::vector<bool> active_layers) override;
  void Start() override;
  void Stop() override;
  void SetSource(rtc::VideoSourceInterface<webrtz::VideoFrame>* source,
                 const webrtz::VideoSendStream::DegradationPreference&
                     degradation_preference) override;
  webrtz::VideoSendStream::Stats GetStats() override;
  void ReconfigureVideoEncoder(webrtz::VideoEncoderConfig config) override;

  bool sending_;
  webrtz::VideoSendStream::Config config_;
  webrtz::VideoEncoderConfig encoder_config_;
  std::vector<webrtz::VideoStream> video_streams_;
  rtc::VideoSinkWants sink_wants_;

  bool codec_settings_set_;
  union VpxSettings {
    webrtz::VideoCodecVP8 vp8;
    webrtz::VideoCodecVP9 vp9;
  } vpx_settings_;
  bool resolution_scaling_enabled_;
  bool framerate_scaling_enabled_;
  rtc::VideoSourceInterface<webrtz::VideoFrame>* source_;
  int num_swapped_frames_;
  rtc::Optional<webrtz::VideoFrame> last_frame_;
  webrtz::VideoSendStream::Stats stats_;
  int num_encoder_reconfigurations_ = 0;
};

class FakeVideoReceiveStream final : public webrtz::VideoReceiveStream {
 public:
  explicit FakeVideoReceiveStream(webrtz::VideoReceiveStream::Config config);

  const webrtz::VideoReceiveStream::Config& GetConfig() const;

  bool IsReceiving() const;

  void InjectFrame(const webrtz::VideoFrame& frame);

  void SetStats(const webrtz::VideoReceiveStream::Stats& stats);

  void EnableEncodedFrameRecording(rtc::PlatformFile file,
                                   size_t byte_limit) override;

  void AddSecondarySink(webrtz::RtpPacketSinkInterface* sink) override;
  void RemoveSecondarySink(const webrtz::RtpPacketSinkInterface* sink) override;

  int GetNumAddedSecondarySinks() const;
  int GetNumRemovedSecondarySinks() const;

 private:
  // webrtz::VideoReceiveStream implementation.
  void Start() override;
  void Stop() override;

  webrtz::VideoReceiveStream::Stats GetStats() const override;

  webrtz::VideoReceiveStream::Config config_;
  bool receiving_;
  webrtz::VideoReceiveStream::Stats stats_;

  int num_added_secondary_sinks_;
  int num_removed_secondary_sinks_;
};

class FakeFlexfecReceiveStream final : public webrtz::FlexfecReceiveStream {
 public:
  explicit FakeFlexfecReceiveStream(
      const webrtz::FlexfecReceiveStream::Config& config);

  const webrtz::FlexfecReceiveStream::Config& GetConfig() const override;

 private:
  webrtz::FlexfecReceiveStream::Stats GetStats() const override;

  void OnRtpPacket(const webrtz::RtpPacketReceived& packet) override;

  webrtz::FlexfecReceiveStream::Config config_;
};

class FakeCall final : public webrtz::Call, public webrtz::PacketReceiver {
 public:
  FakeCall();
  ~FakeCall() override;

  webrtz::MockRtpTransportControllerSend* GetMockTransportControllerSend() {
    return &transport_controller_send_;
  }

  const std::vector<FakeVideoSendStream*>& GetVideoSendStreams();
  const std::vector<FakeVideoReceiveStream*>& GetVideoReceiveStreams();

  const std::vector<FakeAudioSendStream*>& GetAudioSendStreams();
  const FakeAudioSendStream* GetAudioSendStream(uint32_t ssrc);
  const std::vector<FakeAudioReceiveStream*>& GetAudioReceiveStreams();
  const FakeAudioReceiveStream* GetAudioReceiveStream(uint32_t ssrc);

  const std::vector<FakeFlexfecReceiveStream*>& GetFlexfecReceiveStreams();

  rtc::SentPacket last_sent_packet() const { return last_sent_packet_; }

  // This is useful if we care about the last media packet (with id populated)
  // but not the last ICE packet (with -1 ID).
  int last_sent_nonnegative_packet_id() const {
    return last_sent_nonnegative_packet_id_;
  }

  webrtz::NetworkState GetNetworkState(webrtz::MediaType media) const;
  int GetNumCreatedSendStreams() const;
  int GetNumCreatedReceiveStreams() const;
  void SetStats(const webrtz::Call::Stats& stats);

 private:
  webrtz::AudioSendStream* CreateAudioSendStream(
      const webrtz::AudioSendStream::Config& config) override;
  void DestroyAudioSendStream(webrtz::AudioSendStream* send_stream) override;

  webrtz::AudioReceiveStream* CreateAudioReceiveStream(
      const webrtz::AudioReceiveStream::Config& config) override;
  void DestroyAudioReceiveStream(
      webrtz::AudioReceiveStream* receive_stream) override;

  webrtz::VideoSendStream* CreateVideoSendStream(
      webrtz::VideoSendStream::Config config,
      webrtz::VideoEncoderConfig encoder_config) override;
  void DestroyVideoSendStream(webrtz::VideoSendStream* send_stream) override;

  webrtz::VideoReceiveStream* CreateVideoReceiveStream(
      webrtz::VideoReceiveStream::Config config) override;
  void DestroyVideoReceiveStream(
      webrtz::VideoReceiveStream* receive_stream) override;

  webrtz::FlexfecReceiveStream* CreateFlexfecReceiveStream(
      const webrtz::FlexfecReceiveStream::Config& config) override;
  void DestroyFlexfecReceiveStream(
      webrtz::FlexfecReceiveStream* receive_stream) override;

  webrtz::PacketReceiver* Receiver() override;

  DeliveryStatus DeliverPacket(webrtz::MediaType media_type,
                               rtc::CopyOnWriteBuffer packet,
                               const webrtz::PacketTime& packet_time) override;

  webrtz::RtpTransportControllerSendInterface* GetTransportControllerSend()
      override {
    return &transport_controller_send_;
  }

  webrtz::Call::Stats GetStats() const override;

  void SetBitrateAllocationStrategy(
      std::unique_ptr<rtc::BitrateAllocationStrategy>
          bitrate_allocation_strategy) override;

  void SignalChannelNetworkState(webrtz::MediaType media,
                                 webrtz::NetworkState state) override;
  void OnTransportOverheadChanged(webrtz::MediaType media,
                                  int transport_overhead_per_packet) override;
  void OnSentPacket(const rtc::SentPacket& sent_packet) override;

  testing::NiceMock<webrtz::MockRtpTransportControllerSend>
      transport_controller_send_;

  webrtz::NetworkState audio_network_state_;
  webrtz::NetworkState video_network_state_;
  rtc::SentPacket last_sent_packet_;
  int last_sent_nonnegative_packet_id_ = -1;
  int next_stream_id_ = 665;
  webrtz::Call::Stats stats_;
  std::vector<FakeVideoSendStream*> video_send_streams_;
  std::vector<FakeAudioSendStream*> audio_send_streams_;
  std::vector<FakeVideoReceiveStream*> video_receive_streams_;
  std::vector<FakeAudioReceiveStream*> audio_receive_streams_;
  std::vector<FakeFlexfecReceiveStream*> flexfec_receive_streams_;

  int num_created_send_streams_;
  int num_created_receive_streams_;

  int audio_transport_overhead_;
  int video_transport_overhead_;
};

}  // namespace cricket
#endif  // MEDIA_ENGINE_FAKEWEBRTCCALL_H_
