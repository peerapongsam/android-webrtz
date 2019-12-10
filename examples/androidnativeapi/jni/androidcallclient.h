/*
 *  Copyright 2018 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef EXAMPLES_ANDROIDNATIVEAPI_JNI_ANDROIDCALLCLIENT_H_
#define EXAMPLES_ANDROIDNATIVEAPI_JNI_ANDROIDCALLCLIENT_H_

#include <jni.h>

#include <memory>
#include <string>

#include "api/peerconnectioninterface.h"
#include "rtc_base/criticalsection.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "rtc_base/thread_checker.h"
#include "sdk/android/native_api/jni/scoped_java_ref.h"

namespace webrtz_examples {

class AndroidCallClient {
 public:
  AndroidCallClient();

  void Call(JNIEnv* env,
            const webrtz::JavaRef<jobject>& cls,
            const webrtz::JavaRef<jobject>& local_sink,
            const webrtz::JavaRef<jobject>& remote_sink);
  void Hangup(JNIEnv* env, const webrtz::JavaRef<jobject>& cls);
  // A helper method for Java code to delete this object. Calls delete this.
  void Delete(JNIEnv* env, const webrtz::JavaRef<jobject>& cls);

 private:
  class PCObserver;

  void CreatePeerConnectionFactory() RTC_RUN_ON(thread_checker_);
  void CreatePeerConnection() RTC_RUN_ON(thread_checker_);
  void Connect() RTC_RUN_ON(thread_checker_);

  rtc::ThreadChecker thread_checker_;

  bool call_started_ RTC_GUARDED_BY(thread_checker_);

  const std::unique_ptr<PCObserver> pc_observer_;

  rtc::scoped_refptr<webrtz::PeerConnectionFactoryInterface> pcf_
      RTC_GUARDED_BY(thread_checker_);
  std::unique_ptr<rtc::Thread> network_thread_ RTC_GUARDED_BY(thread_checker_);
  std::unique_ptr<rtc::Thread> worker_thread_ RTC_GUARDED_BY(thread_checker_);
  std::unique_ptr<rtc::Thread> signaling_thread_
      RTC_GUARDED_BY(thread_checker_);

  std::unique_ptr<rtc::VideoSinkInterface<webrtz::VideoFrame>> local_sink_
      RTC_GUARDED_BY(thread_checker_);
  std::unique_ptr<rtc::VideoSinkInterface<webrtz::VideoFrame>> remote_sink_
      RTC_GUARDED_BY(thread_checker_);
  rtc::scoped_refptr<webrtz::VideoTrackSourceInterface> video_source_
      RTC_GUARDED_BY(thread_checker_);

  rtc::CriticalSection pc_mutex_;
  rtc::scoped_refptr<webrtz::PeerConnectionInterface> pc_
      RTC_GUARDED_BY(pc_mutex_);
};

}  // namespace webrtz_examples

#endif  // EXAMPLES_ANDROIDNATIVEAPI_JNI_ANDROIDCALLCLIENT_H_
