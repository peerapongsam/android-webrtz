/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "WebRTC/RTCPeerConnectionFactory.h"

#include "rtc_base/scoped_ref_ptr.h"

namespace webrtz {

class AudioDeviceModule;
class AudioEncoderFactory;
class AudioDecoderFactory;
class VideoEncoderFactory;
class VideoDecoderFactory;
class AudioProcessing;

}  // namespace webrtz

#if defined(USE_BUILTIN_SW_CODECS)
namespace cricket {

class WebRtcVideoEncoderFactory;
class WebRtcVideoDecoderFactory;

}  // namespace cricket
#endif

NS_ASSUME_NONNULL_BEGIN

/**
 * This class extension exposes methods that work directly with injectable C++ components.
 */
@interface RTCPeerConnectionFactory ()

- (instancetype)initNative NS_DESIGNATED_INITIALIZER;

/* Initializer used when WebRTC is compiled with no media support */
- (instancetype)initWithNoMedia;

/* Initialize object with injectable native audio/video encoder/decoder factories */
- (instancetype)initWithNativeAudioEncoderFactory:
                    (rtc::scoped_refptr<webrtz::AudioEncoderFactory>)audioEncoderFactory
                        nativeAudioDecoderFactory:
                            (rtc::scoped_refptr<webrtz::AudioDecoderFactory>)audioDecoderFactory
                        nativeVideoEncoderFactory:
                            (std::unique_ptr<webrtz::VideoEncoderFactory>)videoEncoderFactory
                        nativeVideoDecoderFactory:
                            (std::unique_ptr<webrtz::VideoDecoderFactory>)videoDecoderFactory
                                audioDeviceModule:
                                    (nullable webrtz::AudioDeviceModule *)audioDeviceModule
                            audioProcessingModule:
                                (rtc::scoped_refptr<webrtz::AudioProcessing>)audioProcessingModule;

#if defined(USE_BUILTIN_SW_CODECS)
/* Initialize object with legacy injectable native audio/video encoder/decoder factories
   TODO(andersc): Remove this when backwards compatiblity is no longer needed.
 */
- (instancetype)
    initWithNativeAudioEncoderFactory:
        (rtc::scoped_refptr<webrtz::AudioEncoderFactory>)audioEncoderFactory
            nativeAudioDecoderFactory:
                (rtc::scoped_refptr<webrtz::AudioDecoderFactory>)audioDecoderFactory
      legacyNativeVideoEncoderFactory:(cricket::WebRtcVideoEncoderFactory*)videoEncoderFactory
      legacyNativeVideoDecoderFactory:(cricket::WebRtcVideoDecoderFactory*)videoDecoderFactory
                    audioDeviceModule:(nullable webrtz::AudioDeviceModule *)audioDeviceModule;
#endif

@end

NS_ASSUME_NONNULL_END
