/*
 *  Copyright 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SDK_ANDROID_NATIVE_API_CODECS_WRAPPER_H_
#define SDK_ANDROID_NATIVE_API_CODECS_WRAPPER_H_

#include <jni.h>
#include <memory>

#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"

namespace webrtz {

// Creates an instance of webrtz::VideoDecoderFactory from Java
// VideoDecoderFactory.
std::unique_ptr<VideoDecoderFactory> JavaToNativeVideoDecoderFactory(
    JNIEnv* jni,
    jobject decoder_factory);

// Creates an instance of webrtz::VideoEncoderFactory from Java
// VideoEncoderFactory.
std::unique_ptr<VideoEncoderFactory> JavaToNativeVideoEncoderFactory(
    JNIEnv* jni,
    jobject encoder_factory);

}  // namespace webrtz

#endif  // SDK_ANDROID_NATIVE_API_CODECS_WRAPPER_H_
