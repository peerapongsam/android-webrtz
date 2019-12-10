/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MEDIA_BASE_VIDEOSOURCEBASE_H_
#define MEDIA_BASE_VIDEOSOURCEBASE_H_

#include <vector>

#include "api/video/video_frame.h"
#include "api/videosourceinterface.h"
#include "rtc_base/thread_checker.h"

namespace rtc {

// VideoSourceBase is not thread safe.
class VideoSourceBase : public VideoSourceInterface<webrtz::VideoFrame> {
 public:
  VideoSourceBase();
  ~VideoSourceBase() override;
  void AddOrUpdateSink(VideoSinkInterface<webrtz::VideoFrame>* sink,
                       const VideoSinkWants& wants) override;
  void RemoveSink(VideoSinkInterface<webrtz::VideoFrame>* sink) override;

 protected:
  struct SinkPair {
    SinkPair(VideoSinkInterface<webrtz::VideoFrame>* sink, VideoSinkWants wants)
        : sink(sink), wants(wants) {}
    VideoSinkInterface<webrtz::VideoFrame>* sink;
    VideoSinkWants wants;
  };
  SinkPair* FindSinkPair(const VideoSinkInterface<webrtz::VideoFrame>* sink);

  const std::vector<SinkPair>& sink_pairs() const { return sinks_; }
  ThreadChecker thread_checker_;

 private:
  std::vector<SinkPair> sinks_;
};

}  // namespace rtc

#endif  // MEDIA_BASE_VIDEOSOURCEBASE_H_
