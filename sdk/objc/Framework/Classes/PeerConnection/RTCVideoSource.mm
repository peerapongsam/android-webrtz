/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCVideoSource+Private.h"

#include "api/videosourceproxy.h"
#include "rtc_base/checks.h"
#include "sdk/objc/Framework/Native/src/objc_video_track_source.h"

static webrtz::ObjCVideoTrackSource *getObjCVideoSource(
    const rtc::scoped_refptr<webrtz::VideoTrackSourceInterface> nativeSource) {
  webrtz::VideoTrackSourceProxy *proxy_source =
      static_cast<webrtz::VideoTrackSourceProxy *>(nativeSource.get());
  return static_cast<webrtz::ObjCVideoTrackSource *>(proxy_source->internal());
}

// TODO(magjed): Refactor this class and target ObjCVideoTrackSource only once
// RTCAVFoundationVideoSource is gone. See http://crbug/webrtc/7177 for more
// info.
@implementation RTCVideoSource {
  rtc::scoped_refptr<webrtz::VideoTrackSourceInterface> _nativeVideoSource;
}

- (instancetype)initWithNativeVideoSource:
    (rtc::scoped_refptr<webrtz::VideoTrackSourceInterface>)nativeVideoSource {
  RTC_DCHECK(nativeVideoSource);
  if (self = [super initWithNativeMediaSource:nativeVideoSource
                                         type:RTCMediaSourceTypeVideo]) {
    _nativeVideoSource = nativeVideoSource;
  }
  return self;
}

- (instancetype)initWithNativeMediaSource:
    (rtc::scoped_refptr<webrtz::MediaSourceInterface>)nativeMediaSource
                                     type:(RTCMediaSourceType)type {
  RTC_NOTREACHED();
  return nil;
}

- (instancetype)initWithSignalingThread:(rtc::Thread *)signalingThread
                           workerThread:(rtc::Thread *)workerThread {
  rtc::scoped_refptr<webrtz::ObjCVideoTrackSource> objCVideoTrackSource(
      new rtc::RefCountedObject<webrtz::ObjCVideoTrackSource>());

  return [self initWithNativeVideoSource:webrtz::VideoTrackSourceProxy::Create(
                                             signalingThread, workerThread, objCVideoTrackSource)];
}

- (NSString *)description {
  NSString *stateString = [[self class] stringForState:self.state];
  return [NSString stringWithFormat:@"RTCVideoSource( %p ): %@", self, stateString];
}

- (void)capturer:(RTCVideoCapturer *)capturer didCaptureVideoFrame:(RTCVideoFrame *)frame {
  getObjCVideoSource(_nativeVideoSource)->OnCapturedFrame(frame);
}

- (void)adaptOutputFormatToWidth:(int)width height:(int)height fps:(int)fps {
  getObjCVideoSource(_nativeVideoSource)->OnOutputFormatRequest(width, height, fps);
}

#pragma mark - Private

- (rtc::scoped_refptr<webrtz::VideoTrackSourceInterface>)nativeVideoSource {
  return _nativeVideoSource;
}

@end
