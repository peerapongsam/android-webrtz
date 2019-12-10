/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_SYNCHRONIZATION_RW_LOCK_WIN_H_
#define RTC_BASE_SYNCHRONIZATION_RW_LOCK_WIN_H_

#include "rtc_base/synchronization/rw_lock_wrapper.h"

#include <Windows.h>

namespace webrtz {

class RWLockWin : public RWLockWrapper {
 public:
  static RWLockWin* Create();

  void AcquireLockExclusive() override;
  void ReleaseLockExclusive() override;

  void AcquireLockShared() override;
  void ReleaseLockShared() override;

 private:
  RWLockWin();
  static bool LoadModule();

  SRWLOCK lock_;
};

}  // namespace webrtz

#endif  // RTC_BASE_SYNCHRONIZATION_RW_LOCK_WIN_H_
