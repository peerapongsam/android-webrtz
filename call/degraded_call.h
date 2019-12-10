/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef CALL_DEGRADED_CALL_H_
#define CALL_DEGRADED_CALL_H_

#include <memory>

#include "api/call/transport.h"
#include "api/optional.h"
#include "call/call.h"
#include "call/fake_network_pipe.h"
#include "modules/utility/include/process_thread.h"
#include "system_wrappers/include/clock.h"

namespace webrtz {

class DegradedCall : public Call, private Transport, private PacketReceiver {
 public:
  explicit DegradedCall(std::unique_ptr<Call> call,
                        rtc::Optional<FakeNetworkPipe::Config> send_config,
                        rtc::Optional<FakeNetworkPipe::Config> receive_config);
  ~DegradedCall() override;

  // Implements Call.
  AudioSendStream* CreateAudioSendStream(
      const AudioSendStream::Config& config) override;
  void DestroyAudioSendStream(AudioSendStream* send_stream) override;

  AudioReceiveStream* CreateAudioReceiveStream(
      const AudioReceiveStream::Config& config) override;
  void DestroyAudioReceiveStream(AudioReceiveStream* receive_stream) override;

  VideoSendStream* CreateVideoSendStream(
      VideoSendStream::Config config,
      VideoEncoderConfig encoder_config) override;
  VideoSendStream* CreateVideoSendStream(
      VideoSendStream::Config config,
      VideoEncoderConfig encoder_config,
      std::unique_ptr<FecController> fec_controller) override;
  void DestroyVideoSendStream(VideoSendStream* send_stream) override;

  VideoReceiveStream* CreateVideoReceiveStream(
      VideoReceiveStream::Config configuration) override;
  void DestroyVideoReceiveStream(VideoReceiveStream* receive_stream) override;

  FlexfecReceiveStream* CreateFlexfecReceiveStream(
      const FlexfecReceiveStream::Config& config) override;
  void DestroyFlexfecReceiveStream(
      FlexfecReceiveStream* receive_stream) override;

  PacketReceiver* Receiver() override;

  RtpTransportControllerSendInterface* GetTransportControllerSend() override;

  Stats GetStats() const override;

  void SetBitrateAllocationStrategy(
      std::unique_ptr<rtc::BitrateAllocationStrategy>
          bitrate_allocation_strategy) override;

  void SignalChannelNetworkState(MediaType media, NetworkState state) override;

  void OnTransportOverheadChanged(MediaType media,
                                  int transport_overhead_per_packet) override;

  void OnSentPacket(const rtc::SentPacket& sent_packet) override;

 protected:
  // Implements Transport.
  bool SendRtp(const uint8_t* packet,
               size_t length,
               const PacketOptions& options) override;

  bool SendRtcp(const uint8_t* packet, size_t length) override;

  // Implements PacketReceiver.
  DeliveryStatus DeliverPacket(MediaType media_type,
                               rtc::CopyOnWriteBuffer packet,
                               const PacketTime& packet_time) override;

 private:
  Clock* const clock_;
  const std::unique_ptr<Call> call_;

  const rtc::Optional<FakeNetworkPipe::Config> send_config_;
  const std::unique_ptr<ProcessThread> send_process_thread_;
  std::unique_ptr<FakeNetworkPipe> send_pipe_;
  size_t num_send_streams_;

  const rtc::Optional<FakeNetworkPipe::Config> receive_config_;
  std::unique_ptr<FakeNetworkPipe> receive_pipe_;
};

}  // namespace webrtz

#endif  // CALL_DEGRADED_CALL_H_
