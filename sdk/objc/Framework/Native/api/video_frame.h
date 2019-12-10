/*
 *  Copyright 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SDK_OBJC_FRAMEWORK_NATIVE_API_VIDEO_FRAME_H_
#define SDK_OBJC_FRAMEWORK_NATIVE_API_VIDEO_FRAME_H_

#import "WebRTC/RTCVideoFrame.h"

#include "api/video/video_frame.h"

namespace webrtz {

RTCVideoFrame* NativeToObjCVideoFrame(const VideoFrame& frame);

}  // namespace webrtz

#endif  // SDK_OBJC_FRAMEWORK_NATIVE_API_VIDEO_FRAME_H_
