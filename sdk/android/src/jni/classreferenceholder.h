/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SDK_ANDROID_SRC_JNI_CLASSREFERENCEHOLDER_H_
#define SDK_ANDROID_SRC_JNI_CLASSREFERENCEHOLDER_H_

// TODO(magjed): Update external clients to call webrtz::jni::InitClassLoader
// immediately instead.
#include "sdk/android/native_api/jni/class_loader.h"
#include "sdk/android/src/jni/jni_helpers.h"

namespace webrtz {
namespace jni {

// Deprecated. Call webrtz::jni::InitClassLoader() immediately instead..
inline void LoadGlobalClassReferenceHolder() {
  webrtz::InitClassLoader(GetEnv());
}

// Deprecated. Do not call at all.
inline void FreeGlobalClassReferenceHolder() {}

}  // namespace jni
}  // namespace webrtz

// TODO(magjed): Remove once external clients are updated.
namespace webrtz_jni {

using webrtz::jni::LoadGlobalClassReferenceHolder;
using webrtz::jni::FreeGlobalClassReferenceHolder;

}  // namespace webrtz_jni

#endif  // SDK_ANDROID_SRC_JNI_CLASSREFERENCEHOLDER_H_
