/*
 *  Copyright 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <memory>

#include "api/audio_codecs/L16/audio_decoder_L16.h"
#include "api/audio_codecs/L16/audio_encoder_L16.h"
#include "api/audio_codecs/audio_codec_pair_id.h"
#include "api/audio_codecs/audio_decoder_factory_template.h"
#include "api/audio_codecs/audio_encoder_factory_template.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "rtc_base/gunit.h"
#include "rtc_base/logging.h"
#include "rtc_base/ptr_util.h"
#include "rtc_base/stringencode.h"
#include "rtc_base/stringutils.h"

#ifdef WEBRTC_ANDROID
#include "pc/test/androidtestinitializer.h"
#endif
#include "pc/test/peerconnectiontestwrapper.h"
// Notice that mockpeerconnectionobservers.h must be included after the above!
#include "pc/test/mockpeerconnectionobservers.h"
#include "test/mock_audio_decoder.h"
#include "test/mock_audio_decoder_factory.h"

using testing::AtLeast;
using testing::Invoke;
using testing::StrictMock;
using testing::Values;
using testing::_;

using webrtz::DataChannelInterface;
using webrtz::FakeConstraints;
using webrtz::MediaConstraintsInterface;
using webrtz::MediaStreamInterface;
using webrtz::PeerConnectionInterface;
using webrtz::SdpSemantics;

namespace {

const int kMaxWait = 10000;

}  // namespace

class PeerConnectionEndToEndBaseTest : public sigslot::has_slots<>,
                                       public testing::Test {
 public:
  typedef std::vector<rtc::scoped_refptr<DataChannelInterface> >
      DataChannelList;

  explicit PeerConnectionEndToEndBaseTest(SdpSemantics sdp_semantics) {
    network_thread_ = rtc::Thread::CreateWithSocketServer();
    worker_thread_ = rtc::Thread::Create();
    RTC_CHECK(network_thread_->Start());
    RTC_CHECK(worker_thread_->Start());
    caller_ = new rtc::RefCountedObject<PeerConnectionTestWrapper>(
        "caller", network_thread_.get(), worker_thread_.get());
    callee_ = new rtc::RefCountedObject<PeerConnectionTestWrapper>(
        "callee", network_thread_.get(), worker_thread_.get());
    webrtz::PeerConnectionInterface::IceServer ice_server;
    ice_server.uri = "stun:stun.l.google.com:19302";
    config_.servers.push_back(ice_server);
    config_.sdp_semantics = sdp_semantics;

#ifdef WEBRTC_ANDROID
    webrtz::InitializeAndroidObjects();
#endif
  }

  void CreatePcs(
      const MediaConstraintsInterface* pc_constraints,
      rtc::scoped_refptr<webrtz::AudioEncoderFactory> audio_encoder_factory1,
      rtc::scoped_refptr<webrtz::AudioDecoderFactory> audio_decoder_factory1,
      rtc::scoped_refptr<webrtz::AudioEncoderFactory> audio_encoder_factory2,
      rtc::scoped_refptr<webrtz::AudioDecoderFactory> audio_decoder_factory2) {
    EXPECT_TRUE(caller_->CreatePc(pc_constraints, config_,
                                  audio_encoder_factory1,
                                  audio_decoder_factory1));
    EXPECT_TRUE(callee_->CreatePc(pc_constraints, config_,
                                  audio_encoder_factory2,
                                  audio_decoder_factory2));
    PeerConnectionTestWrapper::Connect(caller_.get(), callee_.get());

    caller_->SignalOnDataChannel.connect(
        this, &PeerConnectionEndToEndBaseTest::OnCallerAddedDataChanel);
    callee_->SignalOnDataChannel.connect(
        this, &PeerConnectionEndToEndBaseTest::OnCalleeAddedDataChannel);
  }

  void CreatePcs(
      const MediaConstraintsInterface* pc_constraints,
      rtc::scoped_refptr<webrtz::AudioEncoderFactory> audio_encoder_factory,
      rtc::scoped_refptr<webrtz::AudioDecoderFactory> audio_decoder_factory) {
    CreatePcs(pc_constraints, audio_encoder_factory, audio_decoder_factory,
              audio_encoder_factory, audio_decoder_factory);
  }

  void GetAndAddUserMedia() {
    FakeConstraints audio_constraints;
    FakeConstraints video_constraints;
    GetAndAddUserMedia(true, audio_constraints, true, video_constraints);
  }

  void GetAndAddUserMedia(bool audio,
                          const FakeConstraints& audio_constraints,
                          bool video,
                          const FakeConstraints& video_constraints) {
    caller_->GetAndAddUserMedia(audio, audio_constraints,
                                video, video_constraints);
    callee_->GetAndAddUserMedia(audio, audio_constraints,
                                video, video_constraints);
  }

  void Negotiate() {
    caller_->CreateOffer(NULL);
  }

  void WaitForCallEstablished() {
    caller_->WaitForCallEstablished();
    callee_->WaitForCallEstablished();
  }

  void WaitForConnection() {
    caller_->WaitForConnection();
    callee_->WaitForConnection();
  }

  void OnCallerAddedDataChanel(DataChannelInterface* dc) {
    caller_signaled_data_channels_.push_back(dc);
  }

  void OnCalleeAddedDataChannel(DataChannelInterface* dc) {
    callee_signaled_data_channels_.push_back(dc);
  }

  // Tests that |dc1| and |dc2| can send to and receive from each other.
  void TestDataChannelSendAndReceive(
      DataChannelInterface* dc1, DataChannelInterface* dc2) {
    std::unique_ptr<webrtz::MockDataChannelObserver> dc1_observer(
        new webrtz::MockDataChannelObserver(dc1));

    std::unique_ptr<webrtz::MockDataChannelObserver> dc2_observer(
        new webrtz::MockDataChannelObserver(dc2));

    static const std::string kDummyData = "abcdefg";
    webrtz::DataBuffer buffer(kDummyData);
    EXPECT_TRUE(dc1->Send(buffer));
    EXPECT_EQ_WAIT(kDummyData, dc2_observer->last_message(), kMaxWait);

    EXPECT_TRUE(dc2->Send(buffer));
    EXPECT_EQ_WAIT(kDummyData, dc1_observer->last_message(), kMaxWait);

    EXPECT_EQ(1U, dc1_observer->received_message_count());
    EXPECT_EQ(1U, dc2_observer->received_message_count());
  }

  void WaitForDataChannelsToOpen(DataChannelInterface* local_dc,
                                 const DataChannelList& remote_dc_list,
                                 size_t remote_dc_index) {
    EXPECT_EQ_WAIT(DataChannelInterface::kOpen, local_dc->state(), kMaxWait);

    EXPECT_TRUE_WAIT(remote_dc_list.size() > remote_dc_index, kMaxWait);
    EXPECT_EQ_WAIT(DataChannelInterface::kOpen,
                   remote_dc_list[remote_dc_index]->state(),
                   kMaxWait);
    EXPECT_EQ(local_dc->id(), remote_dc_list[remote_dc_index]->id());
  }

  void CloseDataChannels(DataChannelInterface* local_dc,
                         const DataChannelList& remote_dc_list,
                         size_t remote_dc_index) {
    local_dc->Close();
    EXPECT_EQ_WAIT(DataChannelInterface::kClosed, local_dc->state(), kMaxWait);
    EXPECT_EQ_WAIT(DataChannelInterface::kClosed,
                   remote_dc_list[remote_dc_index]->state(),
                   kMaxWait);
  }

 protected:
  std::unique_ptr<rtc::Thread> network_thread_;
  std::unique_ptr<rtc::Thread> worker_thread_;
  rtc::scoped_refptr<PeerConnectionTestWrapper> caller_;
  rtc::scoped_refptr<PeerConnectionTestWrapper> callee_;
  DataChannelList caller_signaled_data_channels_;
  DataChannelList callee_signaled_data_channels_;
  webrtz::PeerConnectionInterface::RTCConfiguration config_;
};

class PeerConnectionEndToEndTest
    : public PeerConnectionEndToEndBaseTest,
      public ::testing::WithParamInterface<SdpSemantics> {
 protected:
  PeerConnectionEndToEndTest() : PeerConnectionEndToEndBaseTest(GetParam()) {}
};

namespace {

std::unique_ptr<webrtz::AudioDecoder> CreateForwardingMockDecoder(
    std::unique_ptr<webrtz::AudioDecoder> real_decoder) {
  class ForwardingMockDecoder : public StrictMock<webrtz::MockAudioDecoder> {
   public:
    explicit ForwardingMockDecoder(std::unique_ptr<AudioDecoder> decoder)
        : decoder_(std::move(decoder)) {}

   private:
    std::unique_ptr<AudioDecoder> decoder_;
  };

  const auto dec = real_decoder.get();  // For lambda capturing.
  auto mock_decoder =
      rtc::MakeUnique<ForwardingMockDecoder>(std::move(real_decoder));
  EXPECT_CALL(*mock_decoder, Channels())
      .Times(AtLeast(1))
      .WillRepeatedly(Invoke([dec] { return dec->Channels(); }));
  EXPECT_CALL(*mock_decoder, DecodeInternal(_, _, _, _, _))
      .Times(AtLeast(1))
      .WillRepeatedly(
          Invoke([dec](const uint8_t* encoded, size_t encoded_len,
                       int sample_rate_hz, int16_t* decoded,
                       webrtz::AudioDecoder::SpeechType* speech_type) {
            return dec->Decode(encoded, encoded_len, sample_rate_hz,
                               std::numeric_limits<size_t>::max(), decoded,
                               speech_type);
          }));
  EXPECT_CALL(*mock_decoder, Die());
  EXPECT_CALL(*mock_decoder, HasDecodePlc()).WillRepeatedly(Invoke([dec] {
    return dec->HasDecodePlc();
  }));
  EXPECT_CALL(*mock_decoder, IncomingPacket(_, _, _, _, _))
      .Times(AtLeast(1))
      .WillRepeatedly(Invoke([dec](const uint8_t* payload, size_t payload_len,
                                   uint16_t rtp_sequence_number,
                                   uint32_t rtp_timestamp,
                                   uint32_t arrival_timestamp) {
        return dec->IncomingPacket(payload, payload_len, rtp_sequence_number,
                                   rtp_timestamp, arrival_timestamp);
      }));
  EXPECT_CALL(*mock_decoder, PacketDuration(_, _))
      .Times(AtLeast(1))
      .WillRepeatedly(Invoke([dec](const uint8_t* encoded, size_t encoded_len) {
        return dec->PacketDuration(encoded, encoded_len);
      }));
  EXPECT_CALL(*mock_decoder, SampleRateHz())
      .Times(AtLeast(1))
      .WillRepeatedly(Invoke([dec] { return dec->SampleRateHz(); }));

  return std::move(mock_decoder);
}

rtc::scoped_refptr<webrtz::AudioDecoderFactory>
CreateForwardingMockDecoderFactory(
    webrtz::AudioDecoderFactory* real_decoder_factory) {
  rtc::scoped_refptr<webrtz::MockAudioDecoderFactory> mock_decoder_factory =
      new rtc::RefCountedObject<StrictMock<webrtz::MockAudioDecoderFactory>>;
  EXPECT_CALL(*mock_decoder_factory, GetSupportedDecoders())
      .Times(AtLeast(1))
      .WillRepeatedly(Invoke([real_decoder_factory] {
        return real_decoder_factory->GetSupportedDecoders();
      }));
  EXPECT_CALL(*mock_decoder_factory, IsSupportedDecoder(_))
      .Times(AtLeast(1))
      .WillRepeatedly(
          Invoke([real_decoder_factory](const webrtz::SdpAudioFormat& format) {
            return real_decoder_factory->IsSupportedDecoder(format);
          }));
  EXPECT_CALL(*mock_decoder_factory, MakeAudioDecoderMock(_, _, _))
      .Times(AtLeast(2))
      .WillRepeatedly(
          Invoke([real_decoder_factory](
                     const webrtz::SdpAudioFormat& format,
                     rtc::Optional<webrtz::AudioCodecPairId> codec_pair_id,
                     std::unique_ptr<webrtz::AudioDecoder>* return_value) {
            auto real_decoder =
                real_decoder_factory->MakeAudioDecoder(format, codec_pair_id);
            *return_value =
                real_decoder
                    ? CreateForwardingMockDecoder(std::move(real_decoder))
                    : nullptr;
          }));
  return mock_decoder_factory;
}

struct AudioEncoderUnicornSparklesRainbow {
  using Config = webrtz::AudioEncoderL16::Config;
  static rtc::Optional<Config> SdpToConfig(webrtz::SdpAudioFormat format) {
    if (STR_CASE_CMP(format.name.c_str(), "UnicornSparklesRainbow") == 0) {
      const webrtz::SdpAudioFormat::Parameters expected_params = {
          {"num_horns", "1"}};
      EXPECT_EQ(expected_params, format.parameters);
      format.parameters.clear();
      format.name = "L16";
      return webrtz::AudioEncoderL16::SdpToConfig(format);
    } else {
      return rtc::nullopt;
    }
  }
  static void AppendSupportedEncoders(
      std::vector<webrtz::AudioCodecSpec>* specs) {
    std::vector<webrtz::AudioCodecSpec> new_specs;
    webrtz::AudioEncoderL16::AppendSupportedEncoders(&new_specs);
    for (auto& spec : new_specs) {
      spec.format.name = "UnicornSparklesRainbow";
      EXPECT_TRUE(spec.format.parameters.empty());
      spec.format.parameters.emplace("num_horns", "1");
      specs->push_back(spec);
    }
  }
  static webrtz::AudioCodecInfo QueryAudioEncoder(const Config& config) {
    return webrtz::AudioEncoderL16::QueryAudioEncoder(config);
  }
  static std::unique_ptr<webrtz::AudioEncoder> MakeAudioEncoder(
      const Config& config,
      int payload_type,
      rtc::Optional<webrtz::AudioCodecPairId> codec_pair_id = rtc::nullopt) {
    return webrtz::AudioEncoderL16::MakeAudioEncoder(config, payload_type,
                                                     codec_pair_id);
  }
};

struct AudioDecoderUnicornSparklesRainbow {
  using Config = webrtz::AudioDecoderL16::Config;
  static rtc::Optional<Config> SdpToConfig(webrtz::SdpAudioFormat format) {
    if (STR_CASE_CMP(format.name.c_str(), "UnicornSparklesRainbow") == 0) {
      const webrtz::SdpAudioFormat::Parameters expected_params = {
          {"num_horns", "1"}};
      EXPECT_EQ(expected_params, format.parameters);
      format.parameters.clear();
      format.name = "L16";
      return webrtz::AudioDecoderL16::SdpToConfig(format);
    } else {
      return rtc::nullopt;
    }
  }
  static void AppendSupportedDecoders(
      std::vector<webrtz::AudioCodecSpec>* specs) {
    std::vector<webrtz::AudioCodecSpec> new_specs;
    webrtz::AudioDecoderL16::AppendSupportedDecoders(&new_specs);
    for (auto& spec : new_specs) {
      spec.format.name = "UnicornSparklesRainbow";
      EXPECT_TRUE(spec.format.parameters.empty());
      spec.format.parameters.emplace("num_horns", "1");
      specs->push_back(spec);
    }
  }
  static std::unique_ptr<webrtz::AudioDecoder> MakeAudioDecoder(
      const Config& config,
      rtc::Optional<webrtz::AudioCodecPairId> codec_pair_id = rtc::nullopt) {
    return webrtz::AudioDecoderL16::MakeAudioDecoder(config, codec_pair_id);
  }
};

}  // namespace

// Disabled for TSan v2, see
// https://bugs.chromium.org/p/webrtc/issues/detail?id=4719 for details.
// Disabled for Mac, see
// https://bugs.chromium.org/p/webrtc/issues/detail?id=5231 for details.
#if defined(THREAD_SANITIZER) || defined(WEBRTC_MAC)
TEST_P(PeerConnectionEndToEndTest, DISABLED_Call) {
#else
TEST_P(PeerConnectionEndToEndTest, Call) {
#endif  //  defined(THREAD_SANITIZER) || defined(WEBRTC_MAC)
  rtc::scoped_refptr<webrtz::AudioDecoderFactory> real_decoder_factory =
      webrtz::CreateBuiltinAudioDecoderFactory();
  CreatePcs(nullptr, webrtz::CreateBuiltinAudioEncoderFactory(),
            CreateForwardingMockDecoderFactory(real_decoder_factory.get()));
  GetAndAddUserMedia();
  Negotiate();
  WaitForCallEstablished();
}

#if !defined(ADDRESS_SANITIZER)
TEST_P(PeerConnectionEndToEndTest, CallWithLegacySdp) {
  FakeConstraints pc_constraints;
  pc_constraints.AddMandatory(MediaConstraintsInterface::kEnableDtlsSrtp,
                              false);
  CreatePcs(&pc_constraints, webrtz::CreateBuiltinAudioEncoderFactory(),
            webrtz::CreateBuiltinAudioDecoderFactory());
  GetAndAddUserMedia();
  Negotiate();
  WaitForCallEstablished();
}
#endif  // !defined(ADDRESS_SANITIZER)

TEST_P(PeerConnectionEndToEndTest, CallWithCustomCodec) {
  class IdLoggingAudioEncoderFactory : public webrtz::AudioEncoderFactory {
   public:
    IdLoggingAudioEncoderFactory(
        rtc::scoped_refptr<AudioEncoderFactory> real_factory,
        std::vector<webrtz::AudioCodecPairId>* const codec_ids)
        : fact_(real_factory), codec_ids_(codec_ids) {}
    std::vector<webrtz::AudioCodecSpec> GetSupportedEncoders() override {
      return fact_->GetSupportedEncoders();
    }
    rtc::Optional<webrtz::AudioCodecInfo> QueryAudioEncoder(
        const webrtz::SdpAudioFormat& format) override {
      return fact_->QueryAudioEncoder(format);
    }
    std::unique_ptr<webrtz::AudioEncoder> MakeAudioEncoder(
        int payload_type,
        const webrtz::SdpAudioFormat& format,
        rtc::Optional<webrtz::AudioCodecPairId> codec_pair_id) override {
      EXPECT_TRUE(codec_pair_id.has_value());
      codec_ids_->push_back(*codec_pair_id);
      return fact_->MakeAudioEncoder(payload_type, format, codec_pair_id);
    }

   private:
    const rtc::scoped_refptr<webrtz::AudioEncoderFactory> fact_;
    std::vector<webrtz::AudioCodecPairId>* const codec_ids_;
  };

  class IdLoggingAudioDecoderFactory : public webrtz::AudioDecoderFactory {
   public:
    IdLoggingAudioDecoderFactory(
        rtc::scoped_refptr<AudioDecoderFactory> real_factory,
        std::vector<webrtz::AudioCodecPairId>* const codec_ids)
        : fact_(real_factory), codec_ids_(codec_ids) {}
    std::vector<webrtz::AudioCodecSpec> GetSupportedDecoders() override {
      return fact_->GetSupportedDecoders();
    }
    bool IsSupportedDecoder(const webrtz::SdpAudioFormat& format) override {
      return fact_->IsSupportedDecoder(format);
    }
    std::unique_ptr<webrtz::AudioDecoder> MakeAudioDecoder(
        const webrtz::SdpAudioFormat& format,
        rtc::Optional<webrtz::AudioCodecPairId> codec_pair_id) override {
      EXPECT_TRUE(codec_pair_id.has_value());
      codec_ids_->push_back(*codec_pair_id);
      return fact_->MakeAudioDecoder(format, codec_pair_id);
    }

   private:
    const rtc::scoped_refptr<webrtz::AudioDecoderFactory> fact_;
    std::vector<webrtz::AudioCodecPairId>* const codec_ids_;
  };

  std::vector<webrtz::AudioCodecPairId> encoder_id1, encoder_id2, decoder_id1,
      decoder_id2;
  CreatePcs(nullptr,
            rtc::scoped_refptr<webrtz::AudioEncoderFactory>(
                new rtc::RefCountedObject<IdLoggingAudioEncoderFactory>(
                    webrtz::CreateAudioEncoderFactory<
                        AudioEncoderUnicornSparklesRainbow>(),
                    &encoder_id1)),
            rtc::scoped_refptr<webrtz::AudioDecoderFactory>(
                new rtc::RefCountedObject<IdLoggingAudioDecoderFactory>(
                    webrtz::CreateAudioDecoderFactory<
                        AudioDecoderUnicornSparklesRainbow>(),
                    &decoder_id1)),
            rtc::scoped_refptr<webrtz::AudioEncoderFactory>(
                new rtc::RefCountedObject<IdLoggingAudioEncoderFactory>(
                    webrtz::CreateAudioEncoderFactory<
                        AudioEncoderUnicornSparklesRainbow>(),
                    &encoder_id2)),
            rtc::scoped_refptr<webrtz::AudioDecoderFactory>(
                new rtc::RefCountedObject<IdLoggingAudioDecoderFactory>(
                    webrtz::CreateAudioDecoderFactory<
                        AudioDecoderUnicornSparklesRainbow>(),
                    &decoder_id2)));
  GetAndAddUserMedia();
  Negotiate();
  WaitForCallEstablished();

  // Each codec factory has been used to create one codec. The first pair got
  // the same ID because they were passed to the same PeerConnectionFactory,
  // and the second pair got the same ID---but these two IDs are not equal,
  // because each PeerConnectionFactory has its own ID.
  EXPECT_EQ(1, encoder_id1.size());
  EXPECT_EQ(1, encoder_id2.size());
  EXPECT_EQ(encoder_id1, decoder_id1);
  EXPECT_EQ(encoder_id2, decoder_id2);
  EXPECT_NE(encoder_id1, encoder_id2);
}

#ifdef HAVE_SCTP
// Verifies that a DataChannel created before the negotiation can transition to
// "OPEN" and transfer data.
TEST_P(PeerConnectionEndToEndTest, CreateDataChannelBeforeNegotiate) {
  CreatePcs(nullptr, webrtz::CreateBuiltinAudioEncoderFactory(),
            webrtz::MockAudioDecoderFactory::CreateEmptyFactory());

  webrtz::DataChannelInit init;
  rtc::scoped_refptr<DataChannelInterface> caller_dc(
      caller_->CreateDataChannel("data", init));
  rtc::scoped_refptr<DataChannelInterface> callee_dc(
      callee_->CreateDataChannel("data", init));

  Negotiate();
  WaitForConnection();

  WaitForDataChannelsToOpen(caller_dc, callee_signaled_data_channels_, 0);
  WaitForDataChannelsToOpen(callee_dc, caller_signaled_data_channels_, 0);

  TestDataChannelSendAndReceive(caller_dc, callee_signaled_data_channels_[0]);
  TestDataChannelSendAndReceive(callee_dc, caller_signaled_data_channels_[0]);

  CloseDataChannels(caller_dc, callee_signaled_data_channels_, 0);
  CloseDataChannels(callee_dc, caller_signaled_data_channels_, 0);
}

// Verifies that a DataChannel created after the negotiation can transition to
// "OPEN" and transfer data.
TEST_P(PeerConnectionEndToEndTest, CreateDataChannelAfterNegotiate) {
  CreatePcs(nullptr, webrtz::CreateBuiltinAudioEncoderFactory(),
            webrtz::MockAudioDecoderFactory::CreateEmptyFactory());

  webrtz::DataChannelInit init;

  // This DataChannel is for creating the data content in the negotiation.
  rtc::scoped_refptr<DataChannelInterface> dummy(
      caller_->CreateDataChannel("data", init));
  Negotiate();
  WaitForConnection();

  // Wait for the data channel created pre-negotiation to be opened.
  WaitForDataChannelsToOpen(dummy, callee_signaled_data_channels_, 0);

  // Create new DataChannels after the negotiation and verify their states.
  rtc::scoped_refptr<DataChannelInterface> caller_dc(
      caller_->CreateDataChannel("hello", init));
  rtc::scoped_refptr<DataChannelInterface> callee_dc(
      callee_->CreateDataChannel("hello", init));

  WaitForDataChannelsToOpen(caller_dc, callee_signaled_data_channels_, 1);
  WaitForDataChannelsToOpen(callee_dc, caller_signaled_data_channels_, 0);

  TestDataChannelSendAndReceive(caller_dc, callee_signaled_data_channels_[1]);
  TestDataChannelSendAndReceive(callee_dc, caller_signaled_data_channels_[0]);

  CloseDataChannels(caller_dc, callee_signaled_data_channels_, 1);
  CloseDataChannels(callee_dc, caller_signaled_data_channels_, 0);
}

// Verifies that DataChannel IDs are even/odd based on the DTLS roles.
TEST_P(PeerConnectionEndToEndTest, DataChannelIdAssignment) {
  CreatePcs(nullptr, webrtz::CreateBuiltinAudioEncoderFactory(),
            webrtz::MockAudioDecoderFactory::CreateEmptyFactory());

  webrtz::DataChannelInit init;
  rtc::scoped_refptr<DataChannelInterface> caller_dc_1(
      caller_->CreateDataChannel("data", init));
  rtc::scoped_refptr<DataChannelInterface> callee_dc_1(
      callee_->CreateDataChannel("data", init));

  Negotiate();
  WaitForConnection();

  EXPECT_EQ(1U, caller_dc_1->id() % 2);
  EXPECT_EQ(0U, callee_dc_1->id() % 2);

  rtc::scoped_refptr<DataChannelInterface> caller_dc_2(
      caller_->CreateDataChannel("data", init));
  rtc::scoped_refptr<DataChannelInterface> callee_dc_2(
      callee_->CreateDataChannel("data", init));

  EXPECT_EQ(1U, caller_dc_2->id() % 2);
  EXPECT_EQ(0U, callee_dc_2->id() % 2);
}

// Verifies that the message is received by the right remote DataChannel when
// there are multiple DataChannels.
TEST_P(PeerConnectionEndToEndTest,
       MessageTransferBetweenTwoPairsOfDataChannels) {
  CreatePcs(nullptr, webrtz::CreateBuiltinAudioEncoderFactory(),
            webrtz::MockAudioDecoderFactory::CreateEmptyFactory());

  webrtz::DataChannelInit init;

  rtc::scoped_refptr<DataChannelInterface> caller_dc_1(
      caller_->CreateDataChannel("data", init));
  rtc::scoped_refptr<DataChannelInterface> caller_dc_2(
      caller_->CreateDataChannel("data", init));

  Negotiate();
  WaitForConnection();
  WaitForDataChannelsToOpen(caller_dc_1, callee_signaled_data_channels_, 0);
  WaitForDataChannelsToOpen(caller_dc_2, callee_signaled_data_channels_, 1);

  std::unique_ptr<webrtz::MockDataChannelObserver> dc_1_observer(
      new webrtz::MockDataChannelObserver(callee_signaled_data_channels_[0]));

  std::unique_ptr<webrtz::MockDataChannelObserver> dc_2_observer(
      new webrtz::MockDataChannelObserver(callee_signaled_data_channels_[1]));

  const std::string message_1 = "hello 1";
  const std::string message_2 = "hello 2";

  caller_dc_1->Send(webrtz::DataBuffer(message_1));
  EXPECT_EQ_WAIT(message_1, dc_1_observer->last_message(), kMaxWait);

  caller_dc_2->Send(webrtz::DataBuffer(message_2));
  EXPECT_EQ_WAIT(message_2, dc_2_observer->last_message(), kMaxWait);

  EXPECT_EQ(1U, dc_1_observer->received_message_count());
  EXPECT_EQ(1U, dc_2_observer->received_message_count());
}

// Verifies that a DataChannel added from an OPEN message functions after
// a channel has been previously closed (webrtc issue 3778).
// This previously failed because the new channel re-uses the ID of the closed
// channel, and the closed channel was incorrectly still assigned to the id.
// TODO(deadbeef): This is disabled because there's currently a race condition
// caused by the fact that a data channel signals that it's closed before it
// really is. Re-enable this test once that's fixed.
// See: https://bugs.chromium.org/p/webrtc/issues/detail?id=4453
TEST_P(PeerConnectionEndToEndTest,
       DISABLED_DataChannelFromOpenWorksAfterClose) {
  CreatePcs(nullptr, webrtz::CreateBuiltinAudioEncoderFactory(),
            webrtz::MockAudioDecoderFactory::CreateEmptyFactory());

  webrtz::DataChannelInit init;
  rtc::scoped_refptr<DataChannelInterface> caller_dc(
      caller_->CreateDataChannel("data", init));

  Negotiate();
  WaitForConnection();

  WaitForDataChannelsToOpen(caller_dc, callee_signaled_data_channels_, 0);
  CloseDataChannels(caller_dc, callee_signaled_data_channels_, 0);

  // Create a new channel and ensure it works after closing the previous one.
  caller_dc = caller_->CreateDataChannel("data2", init);

  WaitForDataChannelsToOpen(caller_dc, callee_signaled_data_channels_, 1);
  TestDataChannelSendAndReceive(caller_dc, callee_signaled_data_channels_[1]);

  CloseDataChannels(caller_dc, callee_signaled_data_channels_, 1);
}

// This tests that if a data channel is closed remotely while not referenced
// by the application (meaning only the PeerConnection contributes to its
// reference count), no memory access violation will occur.
// See: https://code.google.com/p/chromium/issues/detail?id=565048
TEST_P(PeerConnectionEndToEndTest, CloseDataChannelRemotelyWhileNotReferenced) {
  CreatePcs(nullptr, webrtz::CreateBuiltinAudioEncoderFactory(),
            webrtz::MockAudioDecoderFactory::CreateEmptyFactory());

  webrtz::DataChannelInit init;
  rtc::scoped_refptr<DataChannelInterface> caller_dc(
      caller_->CreateDataChannel("data", init));

  Negotiate();
  WaitForConnection();

  WaitForDataChannelsToOpen(caller_dc, callee_signaled_data_channels_, 0);
  // This removes the reference to the remote data channel that we hold.
  callee_signaled_data_channels_.clear();
  caller_dc->Close();
  EXPECT_EQ_WAIT(DataChannelInterface::kClosed, caller_dc->state(), kMaxWait);

  // Wait for a bit longer so the remote data channel will receive the
  // close message and be destroyed.
  rtc::Thread::Current()->ProcessMessages(100);
}
#endif  // HAVE_SCTP

INSTANTIATE_TEST_CASE_P(PeerConnectionEndToEndTest,
                        PeerConnectionEndToEndTest,
                        Values(SdpSemantics::kPlanB,
                               SdpSemantics::kUnifiedPlan));
