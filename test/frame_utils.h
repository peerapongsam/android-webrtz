/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef TEST_FRAME_UTILS_H_
#define TEST_FRAME_UTILS_H_

#include <stdint.h>

#include "rtc_base/scoped_ref_ptr.h"

namespace webrtz {
class I420Buffer;
class VideoFrame;
class VideoFrameBuffer;
namespace test {

bool EqualPlane(const uint8_t* data1,
                const uint8_t* data2,
                int stride1,
                int stride2,
                int width,
                int height);

static inline bool EqualPlane(const uint8_t* data1,
                              const uint8_t* data2,
                              int stride,
                              int width,
                              int height) {
  return EqualPlane(data1, data2, stride, stride, width, height);
}

bool FramesEqual(const webrtz::VideoFrame& f1, const webrtz::VideoFrame& f2);

bool FrameBufsEqual(const rtc::scoped_refptr<webrtz::VideoFrameBuffer>& f1,
                    const rtc::scoped_refptr<webrtz::VideoFrameBuffer>& f2);

rtc::scoped_refptr<I420Buffer> ReadI420Buffer(int width, int height, FILE *);

}  // namespace test
}  // namespace webrtz

#endif  // TEST_FRAME_UTILS_H_
