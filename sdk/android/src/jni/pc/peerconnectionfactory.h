/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SDK_ANDROID_SRC_JNI_PC_PEERCONNECTIONFACTORY_H_
#define SDK_ANDROID_SRC_JNI_PC_PEERCONNECTIONFACTORY_H_

#include <jni.h>
#include "api/peerconnectioninterface.h"
#include "rtc_base/thread.h"

namespace webrtz {
namespace jni {

void PeerConnectionFactoryNetworkThreadReady();
void PeerConnectionFactoryWorkerThreadReady();
void PeerConnectionFactorySignalingThreadReady();

// Creates java PeerConnectionFactory with specified |pcf|.
jobject NativeToJavaPeerConnectionFactory(
    JNIEnv* jni,
    rtc::scoped_refptr<webrtz::PeerConnectionFactoryInterface> pcf,
    std::unique_ptr<rtc::Thread> network_thread,
    std::unique_ptr<rtc::Thread> worker_thread,
    std::unique_ptr<rtc::Thread> signaling_thread,
    rtc::NetworkMonitorFactory* network_monitor_factory = nullptr);

}  // namespace jni
}  // namespace webrtz

#endif  // SDK_ANDROID_SRC_JNI_PC_PEERCONNECTIONFACTORY_H_
