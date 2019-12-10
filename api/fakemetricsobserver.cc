/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "api/fakemetricsobserver.h"
#include "rtc_base/checks.h"

namespace webrtz {

FakeMetricsObserver::FakeMetricsObserver() {
  Reset();
}

void FakeMetricsObserver::Reset() {
  RTC_DCHECK(thread_checker_.CalledOnValidThread());
  counters_.clear();
  memset(histogram_samples_, 0, sizeof(histogram_samples_));
}

void FakeMetricsObserver::IncrementEnumCounter(
    PeerConnectionEnumCounterType type,
    int counter,
    int counter_max) {
  RTC_DCHECK(thread_checker_.CalledOnValidThread());
  if (counters_.size() <= static_cast<size_t>(type)) {
    counters_.resize(type + 1);
  }
  auto& counters = counters_[type];
  ++counters[counter];
}

void FakeMetricsObserver::AddHistogramSample(PeerConnectionMetricsName type,
    int value) {
  RTC_DCHECK(thread_checker_.CalledOnValidThread());
  RTC_DCHECK_EQ(histogram_samples_[type], 0);
  histogram_samples_[type] = value;
}

int FakeMetricsObserver::GetEnumCounter(PeerConnectionEnumCounterType type,
                                        int counter) const {
  RTC_DCHECK(thread_checker_.CalledOnValidThread());
  if (counters_.size() <= static_cast<size_t>(type)) {
    return 0;
  }
  const auto& it = counters_[type].find(counter);
  if (it == counters_[type].end()) {
    return 0;
  }
  return it->second;
}

int FakeMetricsObserver::GetHistogramSample(
    PeerConnectionMetricsName type) const {
  RTC_DCHECK(thread_checker_.CalledOnValidThread());
  return histogram_samples_[type];
}

bool FakeMetricsObserver::ExpectOnlySingleEnumCount(
    PeerConnectionEnumCounterType type,
    int counter) const {
  RTC_DCHECK(thread_checker_.CalledOnValidThread());
  if (counters_.size() <= static_cast<size_t>(type)) {
    // If a counter has not been allocated then there has been no call to
    // |IncrementEnumCounter| so all the values are 0.
    return false;
  }
  bool pass = true;
  if (GetEnumCounter(type, counter) != 1) {
    RTC_LOG(LS_ERROR) << "Expected single count for counter: " << counter;
    pass = false;
  }
  for (const auto& entry : counters_[type]) {
    if (entry.first != counter && entry.second > 0) {
      RTC_LOG(LS_ERROR) << "Expected no count for counter: " << entry.first;
      pass = false;
    }
  }
  return pass;
}

}  // namespace webrtz
