/*
 *  Copyright 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "logging/rtc_event_log/rtc_event_log.h"
#include "modules/video_coding/codecs/vp8/include/vp8.h"
#include "test/call_test.h"
#include "test/encoder_settings.h"
#include "test/field_trial.h"
#include "test/gtest.h"
#include "video/end_to_end_tests/multi_stream_tester.h"

namespace webrtz {
class MultiStreamEndToEndTest
    : public test::CallTest,
      public testing::WithParamInterface<std::string> {
 public:
  MultiStreamEndToEndTest() : field_trial_(GetParam()) {}

  virtual ~MultiStreamEndToEndTest() {
    EXPECT_EQ(nullptr, video_send_stream_);
    EXPECT_TRUE(video_receive_streams_.empty());
  }

 private:
  test::ScopedFieldTrials field_trial_;
};

INSTANTIATE_TEST_CASE_P(RoundRobin,
                        MultiStreamEndToEndTest,
                        ::testing::Values("WebRTC-RoundRobinPacing/Disabled/",
                                          "WebRTC-RoundRobinPacing/Enabled/"));

// Each renderer verifies that it receives the expected resolution, and as soon
// as every renderer has received a frame, the test finishes.
TEST_P(MultiStreamEndToEndTest, SendsAndReceivesMultipleStreams) {
  class VideoOutputObserver : public rtc::VideoSinkInterface<VideoFrame> {
   public:
    VideoOutputObserver(const MultiStreamTester::CodecSettings& settings,
                        uint32_t ssrc,
                        test::FrameGeneratorCapturer** frame_generator)
        : settings_(settings),
          ssrc_(ssrc),
          frame_generator_(frame_generator),
          done_(false, false) {}

    void OnFrame(const VideoFrame& video_frame) override {
      EXPECT_EQ(settings_.width, video_frame.width());
      EXPECT_EQ(settings_.height, video_frame.height());
      (*frame_generator_)->Stop();
      done_.Set();
    }

    uint32_t Ssrc() { return ssrc_; }

    bool Wait() { return done_.Wait(kDefaultTimeoutMs); }

   private:
    const MultiStreamTester::CodecSettings& settings_;
    const uint32_t ssrc_;
    test::FrameGeneratorCapturer** const frame_generator_;
    rtc::Event done_;
  };

  class Tester : public MultiStreamTester {
   public:
    explicit Tester(test::SingleThreadedTaskQueueForTesting* task_queue)
        : MultiStreamTester(task_queue) {}
    virtual ~Tester() {}

   protected:
    void Wait() override {
      for (const auto& observer : observers_) {
        EXPECT_TRUE(observer->Wait())
            << "Time out waiting for from on ssrc " << observer->Ssrc();
      }
    }

    void UpdateSendConfig(
        size_t stream_index,
        VideoSendStream::Config* send_config,
        VideoEncoderConfig* encoder_config,
        test::FrameGeneratorCapturer** frame_generator) override {
      observers_[stream_index].reset(new VideoOutputObserver(
          codec_settings[stream_index], send_config->rtp.ssrcs.front(),
          frame_generator));
    }

    void UpdateReceiveConfig(
        size_t stream_index,
        VideoReceiveStream::Config* receive_config) override {
      receive_config->renderer = observers_[stream_index].get();
    }

   private:
    std::unique_ptr<VideoOutputObserver> observers_[kNumStreams];
  } tester(&task_queue_);

  tester.RunTest();
}
}  // namespace webrtz
