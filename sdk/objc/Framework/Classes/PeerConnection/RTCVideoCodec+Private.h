/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "WebRTC/RTCVideoCodec.h"

#import "WebRTC/RTCVideoCodecH264.h"

#include "api/video_codecs/sdp_video_format.h"
#include "common_video/include/video_frame.h"
#include "media/base/codec.h"
#include "modules/video_coding/include/video_codec_interface.h"

NS_ASSUME_NONNULL_BEGIN

/* Interfaces for converting to/from internal C++ formats. */
@interface RTCEncodedImage ()

- (instancetype)initWithNativeEncodedImage:(webrtz::EncodedImage)encodedImage;
- (webrtz::EncodedImage)nativeEncodedImage;

@end

@interface RTCVideoEncoderSettings ()

- (instancetype)initWithNativeVideoCodec:(const webrtz::VideoCodec *__nullable)videoCodec;
- (webrtz::VideoCodec)nativeVideoCodec;

@end

@interface RTCCodecSpecificInfoH264 ()

- (webrtz::CodecSpecificInfo)nativeCodecSpecificInfo;

@end

@interface RTCRtpFragmentationHeader ()

- (instancetype)initWithNativeFragmentationHeader:
        (const webrtz::RTPFragmentationHeader *__nullable)fragmentationHeader;
- (std::unique_ptr<webrtz::RTPFragmentationHeader>)createNativeFragmentationHeader;

@end

@interface RTCVideoCodecInfo ()

- (instancetype)initWithNativeSdpVideoFormat:(webrtz::SdpVideoFormat)format;
- (webrtz::SdpVideoFormat)nativeSdpVideoFormat;

/* TODO(andersc): These are deprecated, remove when no longer in use. */
- (instancetype)initWithNativeVideoCodec:(cricket::VideoCodec)videoCodec;
- (cricket::VideoCodec)nativeVideoCodec;

@end

NS_ASSUME_NONNULL_END
