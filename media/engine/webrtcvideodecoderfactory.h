/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MEDIA_ENGINE_WEBRTCVIDEODECODERFACTORY_H_
#define MEDIA_ENGINE_WEBRTCVIDEODECODERFACTORY_H_

#include <string>

#include "common_types.h"  // NOLINT(build/include)
#include "media/base/codec.h"
#include "rtc_base/refcount.h"

namespace webrtz {
class VideoDecoder;
}

namespace cricket {

struct VideoDecoderParams {
  std::string receive_stream_id;
};

// Deprecated. Use webrtz::VideoDecoderFactory instead.
// https://bugs.chromium.org/p/webrtc/issues/detail?id=7925
class WebRtcVideoDecoderFactory {
 public:
  // Caller takes the ownership of the returned object and it should be released
  // by calling DestroyVideoDecoder().
  virtual webrtz::VideoDecoder* CreateVideoDecoderWithParams(
      const VideoCodec& codec,
      VideoDecoderParams params);

  // DEPRECATED.
  // These methods should not be used by new code and will eventually be
  // removed. See http://crbug.com/webrtc/8140.
  virtual webrtz::VideoDecoder* CreateVideoDecoder(webrtz::VideoCodecType type);

  virtual webrtz::VideoDecoder* CreateVideoDecoderWithParams(
      webrtz::VideoCodecType type,
      VideoDecoderParams params);

  virtual ~WebRtcVideoDecoderFactory() {}

  virtual void DestroyVideoDecoder(webrtz::VideoDecoder* decoder) = 0;
};

}  // namespace cricket

#endif  // MEDIA_ENGINE_WEBRTCVIDEODECODERFACTORY_H_
