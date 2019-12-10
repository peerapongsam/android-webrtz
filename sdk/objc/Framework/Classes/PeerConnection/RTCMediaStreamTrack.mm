/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCAudioTrack+Private.h"
#import "RTCMediaStreamTrack+Private.h"
#import "RTCVideoTrack+Private.h"

#import "NSString+StdString.h"

NSString * const kRTCMediaStreamTrackKindAudio =
    @(webrtz::MediaStreamTrackInterface::kAudioKind);
NSString * const kRTCMediaStreamTrackKindVideo =
    @(webrtz::MediaStreamTrackInterface::kVideoKind);

@implementation RTCMediaStreamTrack {
  rtc::scoped_refptr<webrtz::MediaStreamTrackInterface> _nativeTrack;
  RTCMediaStreamTrackType _type;
}

- (NSString *)kind {
  return [NSString stringForStdString:_nativeTrack->kind()];
}

- (NSString *)trackId {
  return [NSString stringForStdString:_nativeTrack->id()];
}

- (BOOL)isEnabled {
  return _nativeTrack->enabled();
}

- (void)setIsEnabled:(BOOL)isEnabled {
  _nativeTrack->set_enabled(isEnabled);
}

- (RTCMediaStreamTrackState)readyState {
  return [[self class] trackStateForNativeState:_nativeTrack->state()];
}

- (NSString *)description {
  NSString *readyState = [[self class] stringForState:self.readyState];
  return [NSString stringWithFormat:@"RTCMediaStreamTrack:\n%@\n%@\n%@\n%@",
                                    self.kind,
                                    self.trackId,
                                    self.isEnabled ? @"enabled" : @"disabled",
                                    readyState];
}

- (BOOL)isEqual:(id)object {
  if (self == object) {
    return YES;
  }
  if (![object isMemberOfClass:[self class]]) {
    return NO;
  }
  return [self isEqualToTrack:(RTCMediaStreamTrack *)object];
}

- (NSUInteger)hash {
  return (NSUInteger)_nativeTrack.get();
}

#pragma mark - Private

- (rtc::scoped_refptr<webrtz::MediaStreamTrackInterface>)nativeTrack {
  return _nativeTrack;
}

- (instancetype)initWithNativeTrack:
    (rtc::scoped_refptr<webrtz::MediaStreamTrackInterface>)nativeTrack
                               type:(RTCMediaStreamTrackType)type {
  NSParameterAssert(nativeTrack);
  if (self = [super init]) {
    _nativeTrack = nativeTrack;
    _type = type;
  }
  return self;
}

- (instancetype)initWithNativeTrack:
    (rtc::scoped_refptr<webrtz::MediaStreamTrackInterface>)nativeTrack {
  NSParameterAssert(nativeTrack);
  if (nativeTrack->kind() ==
      std::string(webrtz::MediaStreamTrackInterface::kAudioKind)) {
    return [self initWithNativeTrack:nativeTrack
                                type:RTCMediaStreamTrackTypeAudio];
  }
  if (nativeTrack->kind() ==
      std::string(webrtz::MediaStreamTrackInterface::kVideoKind)) {
    return [self initWithNativeTrack:nativeTrack
                                type:RTCMediaStreamTrackTypeVideo];
  }
  return nil;
}

- (BOOL)isEqualToTrack:(RTCMediaStreamTrack *)track {
  if (!track) {
    return NO;
  }
  return _nativeTrack == track.nativeTrack;
}

+ (webrtz::MediaStreamTrackInterface::TrackState)nativeTrackStateForState:
    (RTCMediaStreamTrackState)state {
  switch (state) {
    case RTCMediaStreamTrackStateLive:
      return webrtz::MediaStreamTrackInterface::kLive;
    case RTCMediaStreamTrackStateEnded:
      return webrtz::MediaStreamTrackInterface::kEnded;
  }
}

+ (RTCMediaStreamTrackState)trackStateForNativeState:
    (webrtz::MediaStreamTrackInterface::TrackState)nativeState {
  switch (nativeState) {
    case webrtz::MediaStreamTrackInterface::kLive:
      return RTCMediaStreamTrackStateLive;
    case webrtz::MediaStreamTrackInterface::kEnded:
      return RTCMediaStreamTrackStateEnded;
  }
}

+ (NSString *)stringForState:(RTCMediaStreamTrackState)state {
  switch (state) {
    case RTCMediaStreamTrackStateLive:
      return @"Live";
    case RTCMediaStreamTrackStateEnded:
      return @"Ended";
  }
}

+ (RTCMediaStreamTrack *)mediaTrackForNativeTrack:
        (rtc::scoped_refptr<webrtz::MediaStreamTrackInterface>)nativeTrack {
  NSParameterAssert(nativeTrack);
  if (nativeTrack->kind() == webrtz::MediaStreamTrackInterface::kAudioKind) {
    return
        [[RTCAudioTrack alloc] initWithNativeTrack:nativeTrack type:RTCMediaStreamTrackTypeAudio];
  } else if (nativeTrack->kind() == webrtz::MediaStreamTrackInterface::kVideoKind) {
    return
        [[RTCVideoTrack alloc] initWithNativeTrack:nativeTrack type:RTCMediaStreamTrackTypeVideo];
  } else {
    return [[RTCMediaStreamTrack alloc] initWithNativeTrack:nativeTrack];
  }
}

@end
