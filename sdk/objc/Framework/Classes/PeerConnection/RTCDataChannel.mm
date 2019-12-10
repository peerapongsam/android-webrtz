/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCDataChannel+Private.h"

#import "NSString+StdString.h"

#include <memory>

namespace webrtz {

class DataChannelDelegateAdapter : public DataChannelObserver {
 public:
  DataChannelDelegateAdapter(RTCDataChannel *channel) { channel_ = channel; }

  void OnStateChange() override {
    [channel_.delegate dataChannelDidChangeState:channel_];
  }

  void OnMessage(const DataBuffer& buffer) override {
    RTCDataBuffer *data_buffer =
        [[RTCDataBuffer alloc] initWithNativeBuffer:buffer];
    [channel_.delegate dataChannel:channel_
       didReceiveMessageWithBuffer:data_buffer];
  }

  void OnBufferedAmountChange(uint64_t previousAmount) override {
    id<RTCDataChannelDelegate> delegate = channel_.delegate;
    SEL sel = @selector(dataChannel:didChangeBufferedAmount:);
    if ([delegate respondsToSelector:sel]) {
      [delegate dataChannel:channel_ didChangeBufferedAmount:previousAmount];
    }
  }

 private:
  __weak RTCDataChannel *channel_;
};
}


@implementation RTCDataBuffer {
  std::unique_ptr<webrtz::DataBuffer> _dataBuffer;
}

- (instancetype)initWithData:(NSData *)data isBinary:(BOOL)isBinary {
  NSParameterAssert(data);
  if (self = [super init]) {
    rtc::CopyOnWriteBuffer buffer(
        reinterpret_cast<const uint8_t*>(data.bytes), data.length);
    _dataBuffer.reset(new webrtz::DataBuffer(buffer, isBinary));
  }
  return self;
}

- (NSData *)data {
  return [NSData dataWithBytes:_dataBuffer->data.data()
                        length:_dataBuffer->data.size()];
}

- (BOOL)isBinary {
  return _dataBuffer->binary;
}

#pragma mark - Private

- (instancetype)initWithNativeBuffer:(const webrtz::DataBuffer&)nativeBuffer {
  if (self = [super init]) {
    _dataBuffer.reset(new webrtz::DataBuffer(nativeBuffer));
  }
  return self;
}

- (const webrtz::DataBuffer *)nativeDataBuffer {
  return _dataBuffer.get();
}

@end


@implementation RTCDataChannel {
  rtc::scoped_refptr<webrtz::DataChannelInterface> _nativeDataChannel;
  std::unique_ptr<webrtz::DataChannelDelegateAdapter> _observer;
  BOOL _isObserverRegistered;
}

@synthesize delegate = _delegate;

- (void)dealloc {
  // Handles unregistering the observer properly. We need to do this because
  // there may still be other references to the underlying data channel.
  _nativeDataChannel->UnregisterObserver();
}

- (NSString *)label {
  return [NSString stringForStdString:_nativeDataChannel->label()];
}

- (BOOL)isReliable {
  return _nativeDataChannel->reliable();
}

- (BOOL)isOrdered {
  return _nativeDataChannel->ordered();
}

- (NSUInteger)maxRetransmitTime {
  return self.maxPacketLifeTime;
}

- (uint16_t)maxPacketLifeTime {
  return _nativeDataChannel->maxRetransmitTime();
}

- (uint16_t)maxRetransmits {
  return _nativeDataChannel->maxRetransmits();
}

- (NSString *)protocol {
  return [NSString stringForStdString:_nativeDataChannel->protocol()];
}

- (BOOL)isNegotiated {
  return _nativeDataChannel->negotiated();
}

- (NSInteger)streamId {
  return self.channelId;
}

- (int)channelId {
  return _nativeDataChannel->id();
}

- (RTCDataChannelState)readyState {
  return [[self class] dataChannelStateForNativeState:
      _nativeDataChannel->state()];
}

- (uint64_t)bufferedAmount {
  return _nativeDataChannel->buffered_amount();
}

- (void)close {
  _nativeDataChannel->Close();
}

- (BOOL)sendData:(RTCDataBuffer *)data {
  return _nativeDataChannel->Send(*data.nativeDataBuffer);
}

- (NSString *)description {
  return [NSString stringWithFormat:@"RTCDataChannel:\n%ld\n%@\n%@",
                                    (long)self.channelId,
                                    self.label,
                                    [[self class]
                                        stringForState:self.readyState]];
}

#pragma mark - Private

- (instancetype)initWithNativeDataChannel:
    (rtc::scoped_refptr<webrtz::DataChannelInterface>)nativeDataChannel {
  NSParameterAssert(nativeDataChannel);
  if (self = [super init]) {
    _nativeDataChannel = nativeDataChannel;
    _observer.reset(new webrtz::DataChannelDelegateAdapter(self));
    _nativeDataChannel->RegisterObserver(_observer.get());
  }
  return self;
}

+ (webrtz::DataChannelInterface::DataState)
    nativeDataChannelStateForState:(RTCDataChannelState)state {
  switch (state) {
    case RTCDataChannelStateConnecting:
      return webrtz::DataChannelInterface::DataState::kConnecting;
    case RTCDataChannelStateOpen:
      return webrtz::DataChannelInterface::DataState::kOpen;
    case RTCDataChannelStateClosing:
      return webrtz::DataChannelInterface::DataState::kClosing;
    case RTCDataChannelStateClosed:
      return webrtz::DataChannelInterface::DataState::kClosed;
  }
}

+ (RTCDataChannelState)dataChannelStateForNativeState:
    (webrtz::DataChannelInterface::DataState)nativeState {
  switch (nativeState) {
    case webrtz::DataChannelInterface::DataState::kConnecting:
      return RTCDataChannelStateConnecting;
    case webrtz::DataChannelInterface::DataState::kOpen:
      return RTCDataChannelStateOpen;
    case webrtz::DataChannelInterface::DataState::kClosing:
      return RTCDataChannelStateClosing;
    case webrtz::DataChannelInterface::DataState::kClosed:
      return RTCDataChannelStateClosed;
  }
}

+ (NSString *)stringForState:(RTCDataChannelState)state {
  switch (state) {
    case RTCDataChannelStateConnecting:
      return @"Connecting";
    case RTCDataChannelStateOpen:
      return @"Open";
    case RTCDataChannelStateClosing:
      return @"Closing";
    case RTCDataChannelStateClosed:
      return @"Closed";
  }
}

@end
