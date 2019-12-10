/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "media/engine/webrtcvideodecoderfactory.h"

namespace cricket {

webrtz::VideoDecoder* WebRtcVideoDecoderFactory::CreateVideoDecoderWithParams(
    const VideoCodec& codec,
    VideoDecoderParams params) {
  // Default implementation that delegates to old version in order to preserve
  // backwards-compatability.
  webrtz::VideoCodecType type = webrtz::PayloadStringToCodecType(codec.name);
  return CreateVideoDecoderWithParams(type, params);
}

webrtz::VideoDecoder* WebRtcVideoDecoderFactory::CreateVideoDecoder(
    webrtz::VideoCodecType type) {
  RTC_NOTREACHED();
  return nullptr;
}

webrtz::VideoDecoder* WebRtcVideoDecoderFactory::CreateVideoDecoderWithParams(
    webrtz::VideoCodecType type,
    VideoDecoderParams params) {
  return CreateVideoDecoder(type);
}

}  // namespace cricket
