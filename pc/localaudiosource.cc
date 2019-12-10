/*
 *  Copyright 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "pc/localaudiosource.h"

#include <vector>

#include "api/mediaconstraintsinterface.h"
#include "media/base/mediaengine.h"

using webrtz::MediaConstraintsInterface;
using webrtz::MediaSourceInterface;

namespace webrtz {

rtc::scoped_refptr<LocalAudioSource> LocalAudioSource::Create(
    const MediaConstraintsInterface* constraints) {
  rtc::scoped_refptr<LocalAudioSource> source(
      new rtc::RefCountedObject<LocalAudioSource>());
  source->Initialize(constraints);
  return source;
}

rtc::scoped_refptr<LocalAudioSource> LocalAudioSource::Create(
    const cricket::AudioOptions* audio_options) {
  rtc::scoped_refptr<LocalAudioSource> source(
      new rtc::RefCountedObject<LocalAudioSource>());
  source->Initialize(audio_options);
  return source;
}

void LocalAudioSource::Initialize(
    const MediaConstraintsInterface* constraints) {
  CopyConstraintsIntoAudioOptions(constraints, &options_);
}

void LocalAudioSource::Initialize(
    const cricket::AudioOptions* audio_options) {
  if (!audio_options)
    return;

  options_ = *audio_options;
}

}  // namespace webrtz
