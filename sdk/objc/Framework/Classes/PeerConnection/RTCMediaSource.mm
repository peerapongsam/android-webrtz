/*
 *  Copyright 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCMediaSource+Private.h"

#include "rtc_base/checks.h"

@implementation RTCMediaSource {
  RTCMediaSourceType _type;
}

@synthesize nativeMediaSource = _nativeMediaSource;

- (instancetype)initWithNativeMediaSource:
   (rtc::scoped_refptr<webrtz::MediaSourceInterface>)nativeMediaSource
                                     type:(RTCMediaSourceType)type {
  RTC_DCHECK(nativeMediaSource);
  if (self = [super init]) {
    _nativeMediaSource = nativeMediaSource;
    _type = type;
  }
  return self;
}

- (RTCSourceState)state {
  return [[self class] sourceStateForNativeState:_nativeMediaSource->state()];
}

#pragma mark - Private

+ (webrtz::MediaSourceInterface::SourceState)nativeSourceStateForState:
    (RTCSourceState)state {
  switch (state) {
    case RTCSourceStateInitializing:
      return webrtz::MediaSourceInterface::kInitializing;
    case RTCSourceStateLive:
      return webrtz::MediaSourceInterface::kLive;
    case RTCSourceStateEnded:
      return webrtz::MediaSourceInterface::kEnded;
    case RTCSourceStateMuted:
      return webrtz::MediaSourceInterface::kMuted;
  }
}

+ (RTCSourceState)sourceStateForNativeState:
    (webrtz::MediaSourceInterface::SourceState)nativeState {
  switch (nativeState) {
    case webrtz::MediaSourceInterface::kInitializing:
      return RTCSourceStateInitializing;
    case webrtz::MediaSourceInterface::kLive:
      return RTCSourceStateLive;
    case webrtz::MediaSourceInterface::kEnded:
      return RTCSourceStateEnded;
    case webrtz::MediaSourceInterface::kMuted:
      return RTCSourceStateMuted;
  }
}

+ (NSString *)stringForState:(RTCSourceState)state {
  switch (state) {
    case RTCSourceStateInitializing:
      return @"Initializing";
    case RTCSourceStateLive:
      return @"Live";
    case RTCSourceStateEnded:
      return @"Ended";
    case RTCSourceStateMuted:
      return @"Muted";
  }
}

@end
