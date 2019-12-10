/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCPeerConnection+Private.h"

#import "NSString+StdString.h"
#import "RTCConfiguration+Private.h"
#import "RTCDataChannel+Private.h"
#import "RTCIceCandidate+Private.h"
#import "RTCLegacyStatsReport+Private.h"
#import "RTCMediaConstraints+Private.h"
#import "RTCMediaStream+Private.h"
#import "RTCMediaStreamTrack+Private.h"
#import "RTCPeerConnection+Native.h"
#import "RTCPeerConnectionFactory+Private.h"
#import "RTCRtpReceiver+Private.h"
#import "RTCRtpSender+Private.h"
#import "RTCRtpTransceiver+Private.h"
#import "RTCSessionDescription+Private.h"
#import "WebRTC/RTCLogging.h"

#include <memory>

#include "api/jsepicecandidate.h"
#include "rtc_base/checks.h"

NSString * const kRTCPeerConnectionErrorDomain =
    @"org.webrtz.RTCPeerConnection";
int const kRTCPeerConnnectionSessionDescriptionError = -1;

namespace webrtz {

class CreateSessionDescriptionObserverAdapter
    : public CreateSessionDescriptionObserver {
 public:
  CreateSessionDescriptionObserverAdapter(
      void (^completionHandler)(RTCSessionDescription *sessionDescription,
                                NSError *error)) {
    completion_handler_ = completionHandler;
  }

  ~CreateSessionDescriptionObserverAdapter() {
    completion_handler_ = nil;
  }

  void OnSuccess(SessionDescriptionInterface *desc) override {
    RTC_DCHECK(completion_handler_);
    std::unique_ptr<webrtz::SessionDescriptionInterface> description =
        std::unique_ptr<webrtz::SessionDescriptionInterface>(desc);
    RTCSessionDescription* session =
        [[RTCSessionDescription alloc] initWithNativeDescription:
            description.get()];
    completion_handler_(session, nil);
    completion_handler_ = nil;
  }

  void OnFailure(const std::string& error) override {
    RTC_DCHECK(completion_handler_);
    NSString* str = [NSString stringForStdString:error];
    NSError* err =
        [NSError errorWithDomain:kRTCPeerConnectionErrorDomain
                            code:kRTCPeerConnnectionSessionDescriptionError
                        userInfo:@{ NSLocalizedDescriptionKey : str }];
    completion_handler_(nil, err);
    completion_handler_ = nil;
  }

 private:
  void (^completion_handler_)
      (RTCSessionDescription *sessionDescription, NSError *error);
};

class SetSessionDescriptionObserverAdapter :
    public SetSessionDescriptionObserver {
 public:
  SetSessionDescriptionObserverAdapter(void (^completionHandler)
      (NSError *error)) {
    completion_handler_ = completionHandler;
  }

  ~SetSessionDescriptionObserverAdapter() {
    completion_handler_ = nil;
  }

  void OnSuccess() override {
    RTC_DCHECK(completion_handler_);
    completion_handler_(nil);
    completion_handler_ = nil;
  }

  void OnFailure(const std::string& error) override {
    RTC_DCHECK(completion_handler_);
    NSString* str = [NSString stringForStdString:error];
    NSError* err =
        [NSError errorWithDomain:kRTCPeerConnectionErrorDomain
                            code:kRTCPeerConnnectionSessionDescriptionError
                        userInfo:@{ NSLocalizedDescriptionKey : str }];
    completion_handler_(err);
    completion_handler_ = nil;
  }

 private:
  void (^completion_handler_)(NSError *error);
};

PeerConnectionDelegateAdapter::PeerConnectionDelegateAdapter(
    RTCPeerConnection *peerConnection) {
  peer_connection_ = peerConnection;
}

PeerConnectionDelegateAdapter::~PeerConnectionDelegateAdapter() {
  peer_connection_ = nil;
}

void PeerConnectionDelegateAdapter::OnSignalingChange(
    PeerConnectionInterface::SignalingState new_state) {
  RTCSignalingState state =
      [[RTCPeerConnection class] signalingStateForNativeState:new_state];
  RTCPeerConnection *peer_connection = peer_connection_;
  [peer_connection.delegate peerConnection:peer_connection
                   didChangeSignalingState:state];
}

void PeerConnectionDelegateAdapter::OnAddStream(
    rtc::scoped_refptr<MediaStreamInterface> stream) {
  RTCMediaStream *mediaStream =
      [[RTCMediaStream alloc] initWithNativeMediaStream:stream];
  RTCPeerConnection *peer_connection = peer_connection_;
  [peer_connection.delegate peerConnection:peer_connection
                              didAddStream:mediaStream];
}

void PeerConnectionDelegateAdapter::OnRemoveStream(
    rtc::scoped_refptr<MediaStreamInterface> stream) {
  RTCMediaStream *mediaStream =
      [[RTCMediaStream alloc] initWithNativeMediaStream:stream];
  RTCPeerConnection *peer_connection = peer_connection_;
  [peer_connection.delegate peerConnection:peer_connection
                           didRemoveStream:mediaStream];
}

void PeerConnectionDelegateAdapter::OnTrack(
    rtc::scoped_refptr<RtpTransceiverInterface> nativeTransceiver) {
  RTCRtpTransceiver *transceiver =
      [[RTCRtpTransceiver alloc] initWithNativeRtpTransceiver:nativeTransceiver];
  RTCPeerConnection *peer_connection = peer_connection_;
  if ([peer_connection.delegate
          respondsToSelector:@selector(peerConnection:didStartReceivingOnTransceiver:)]) {
    [peer_connection.delegate peerConnection:peer_connection
              didStartReceivingOnTransceiver:transceiver];
  }
}

void PeerConnectionDelegateAdapter::OnDataChannel(
    rtc::scoped_refptr<DataChannelInterface> data_channel) {
  RTCDataChannel *dataChannel =
      [[RTCDataChannel alloc] initWithNativeDataChannel:data_channel];
  RTCPeerConnection *peer_connection = peer_connection_;
  [peer_connection.delegate peerConnection:peer_connection
                        didOpenDataChannel:dataChannel];
}

void PeerConnectionDelegateAdapter::OnRenegotiationNeeded() {
  RTCPeerConnection *peer_connection = peer_connection_;
  [peer_connection.delegate peerConnectionShouldNegotiate:peer_connection];
}

void PeerConnectionDelegateAdapter::OnIceConnectionChange(
    PeerConnectionInterface::IceConnectionState new_state) {
  RTCIceConnectionState state =
      [[RTCPeerConnection class] iceConnectionStateForNativeState:new_state];
  RTCPeerConnection *peer_connection = peer_connection_;
  [peer_connection.delegate peerConnection:peer_connection
               didChangeIceConnectionState:state];
}

void PeerConnectionDelegateAdapter::OnIceGatheringChange(
    PeerConnectionInterface::IceGatheringState new_state) {
  RTCIceGatheringState state =
      [[RTCPeerConnection class] iceGatheringStateForNativeState:new_state];
  RTCPeerConnection *peer_connection = peer_connection_;
  [peer_connection.delegate peerConnection:peer_connection
                didChangeIceGatheringState:state];
}

void PeerConnectionDelegateAdapter::OnIceCandidate(
    const IceCandidateInterface *candidate) {
  RTCIceCandidate *iceCandidate =
      [[RTCIceCandidate alloc] initWithNativeCandidate:candidate];
  RTCPeerConnection *peer_connection = peer_connection_;
  [peer_connection.delegate peerConnection:peer_connection
                   didGenerateIceCandidate:iceCandidate];
}

void PeerConnectionDelegateAdapter::OnIceCandidatesRemoved(
    const std::vector<cricket::Candidate>& candidates) {
  NSMutableArray* ice_candidates =
      [NSMutableArray arrayWithCapacity:candidates.size()];
  for (const auto& candidate : candidates) {
    std::unique_ptr<JsepIceCandidate> candidate_wrapper(
        new JsepIceCandidate(candidate.transport_name(), -1, candidate));
    RTCIceCandidate* ice_candidate = [[RTCIceCandidate alloc]
        initWithNativeCandidate:candidate_wrapper.get()];
    [ice_candidates addObject:ice_candidate];
  }
  RTCPeerConnection* peer_connection = peer_connection_;
  [peer_connection.delegate peerConnection:peer_connection
                    didRemoveIceCandidates:ice_candidates];
}

void PeerConnectionDelegateAdapter::OnAddTrack(
    rtc::scoped_refptr<RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<MediaStreamInterface>>& streams) {
  RTCPeerConnection *peer_connection = peer_connection_;
  if ([peer_connection.delegate
          respondsToSelector:@selector(peerConnection:didAddReceiver:streams:)]) {
    NSMutableArray *mediaStreams = [NSMutableArray arrayWithCapacity:streams.size()];
    for (const auto& nativeStream : streams) {
      RTCMediaStream *mediaStream = [[RTCMediaStream alloc] initWithNativeMediaStream:nativeStream];
      [mediaStreams addObject:mediaStream];
    }
    RTCRtpReceiver *rtpReceiver = [[RTCRtpReceiver alloc] initWithNativeRtpReceiver:receiver];

    [peer_connection.delegate peerConnection:peer_connection
                              didAddReceiver:rtpReceiver
                                     streams:mediaStreams];
  }
}

}  // namespace webrtz


@implementation RTCPeerConnection {
  NSMutableArray<RTCMediaStream *> *_localStreams;
  std::unique_ptr<webrtz::PeerConnectionDelegateAdapter> _observer;
  rtc::scoped_refptr<webrtz::PeerConnectionInterface> _peerConnection;
  std::unique_ptr<webrtz::MediaConstraints> _nativeConstraints;
  BOOL _hasStartedRtcEventLog;
}

@synthesize delegate = _delegate;

- (instancetype)initWithFactory:(RTCPeerConnectionFactory *)factory
                  configuration:(RTCConfiguration *)configuration
                    constraints:(RTCMediaConstraints *)constraints
                       delegate:(id<RTCPeerConnectionDelegate>)delegate {
  NSParameterAssert(factory);
  std::unique_ptr<webrtz::PeerConnectionInterface::RTCConfiguration> config(
      [configuration createNativeConfiguration]);
  if (!config) {
    return nil;
  }
  if (self = [super init]) {
    _observer.reset(new webrtz::PeerConnectionDelegateAdapter(self));
    _nativeConstraints = constraints.nativeConstraints;
    CopyConstraintsIntoRtcConfiguration(_nativeConstraints.get(),
                                        config.get());
    _peerConnection =
        factory.nativeFactory->CreatePeerConnection(*config,
                                                    nullptr,
                                                    nullptr,
                                                    _observer.get());
    if (!_peerConnection) {
      return nil;
    }
    _localStreams = [[NSMutableArray alloc] init];
    _delegate = delegate;
  }
  return self;
}

- (NSArray<RTCMediaStream *> *)localStreams {
  return [_localStreams copy];
}

- (RTCSessionDescription *)localDescription {
  const webrtz::SessionDescriptionInterface *description =
      _peerConnection->local_description();
  return description ?
      [[RTCSessionDescription alloc] initWithNativeDescription:description]
          : nil;
}

- (RTCSessionDescription *)remoteDescription {
  const webrtz::SessionDescriptionInterface *description =
      _peerConnection->remote_description();
  return description ?
      [[RTCSessionDescription alloc] initWithNativeDescription:description]
          : nil;
}

- (RTCSignalingState)signalingState {
  return [[self class]
      signalingStateForNativeState:_peerConnection->signaling_state()];
}

- (RTCIceConnectionState)iceConnectionState {
  return [[self class] iceConnectionStateForNativeState:
      _peerConnection->ice_connection_state()];
}

- (RTCIceGatheringState)iceGatheringState {
  return [[self class] iceGatheringStateForNativeState:
      _peerConnection->ice_gathering_state()];
}

- (BOOL)setConfiguration:(RTCConfiguration *)configuration {
  std::unique_ptr<webrtz::PeerConnectionInterface::RTCConfiguration> config(
      [configuration createNativeConfiguration]);
  if (!config) {
    return NO;
  }
  CopyConstraintsIntoRtcConfiguration(_nativeConstraints.get(),
                                      config.get());
  return _peerConnection->SetConfiguration(*config);
}

- (RTCConfiguration *)configuration {
  webrtz::PeerConnectionInterface::RTCConfiguration config =
    _peerConnection->GetConfiguration();
  return [[RTCConfiguration alloc] initWithNativeConfiguration:config];
}

- (void)close {
  _peerConnection->Close();
}

- (void)addIceCandidate:(RTCIceCandidate *)candidate {
  std::unique_ptr<const webrtz::IceCandidateInterface> iceCandidate(
      candidate.nativeCandidate);
  _peerConnection->AddIceCandidate(iceCandidate.get());
}

- (void)removeIceCandidates:(NSArray<RTCIceCandidate *> *)iceCandidates {
  std::vector<cricket::Candidate> candidates;
  for (RTCIceCandidate *iceCandidate in iceCandidates) {
    std::unique_ptr<const webrtz::IceCandidateInterface> candidate(
        iceCandidate.nativeCandidate);
    if (candidate) {
      candidates.push_back(candidate->candidate());
      // Need to fill the transport name from the sdp_mid.
      candidates.back().set_transport_name(candidate->sdp_mid());
    }
  }
  if (!candidates.empty()) {
    _peerConnection->RemoveIceCandidates(candidates);
  }
}

- (void)addStream:(RTCMediaStream *)stream {
  if (!_peerConnection->AddStream(stream.nativeMediaStream)) {
    RTCLogError(@"Failed to add stream: %@", stream);
    return;
  }
  [_localStreams addObject:stream];
}

- (void)removeStream:(RTCMediaStream *)stream {
  _peerConnection->RemoveStream(stream.nativeMediaStream);
  [_localStreams removeObject:stream];
}

- (RTCRtpSender *)addTrack:(RTCMediaStreamTrack *)track streamIds:(NSArray<NSString *> *)streamIds {
  std::vector<std::string> nativeStreamIds;
  for (NSString *streamId in streamIds) {
    nativeStreamIds.push_back([streamId UTF8String]);
  }
  webrtz::RTCErrorOr<rtc::scoped_refptr<webrtz::RtpSenderInterface>> nativeSenderOrError =
      _peerConnection->AddTrack(track.nativeTrack, nativeStreamIds);
  if (!nativeSenderOrError.ok()) {
    RTCLogError(@"Failed to add track %@: %s", track, nativeSenderOrError.error().message());
    return nil;
  }
  return [[RTCRtpSender alloc] initWithNativeRtpSender:nativeSenderOrError.MoveValue()];
}

- (BOOL)removeTrack:(RTCRtpSender *)sender {
  bool result = _peerConnection->RemoveTrack(sender.nativeRtpSender);
  if (!result) {
    RTCLogError(@"Failed to remote track %@", sender);
  }
  return result;
}

- (RTCRtpTransceiver *)addTransceiverWithTrack:(RTCMediaStreamTrack *)track {
  return [self addTransceiverWithTrack:track init:[[RTCRtpTransceiverInit alloc] init]];
}

- (RTCRtpTransceiver *)addTransceiverWithTrack:(RTCMediaStreamTrack *)track
                                          init:(RTCRtpTransceiverInit *)init {
  webrtz::RTCErrorOr<rtc::scoped_refptr<webrtz::RtpTransceiverInterface>> nativeTransceiverOrError =
      _peerConnection->AddTransceiver(track.nativeTrack, init.nativeInit);
  if (!nativeTransceiverOrError.ok()) {
    RTCLogError(
        @"Failed to add transceiver %@: %s", track, nativeTransceiverOrError.error().message());
    return nil;
  }
  return
      [[RTCRtpTransceiver alloc] initWithNativeRtpTransceiver:nativeTransceiverOrError.MoveValue()];
}

- (RTCRtpTransceiver *)addTransceiverOfType:(RTCRtpMediaType)mediaType {
  return [self addTransceiverOfType:mediaType init:[[RTCRtpTransceiverInit alloc] init]];
}

- (RTCRtpTransceiver *)addTransceiverOfType:(RTCRtpMediaType)mediaType
                                       init:(RTCRtpTransceiverInit *)init {
  webrtz::RTCErrorOr<rtc::scoped_refptr<webrtz::RtpTransceiverInterface>> nativeTransceiverOrError =
      _peerConnection->AddTransceiver([RTCRtpReceiver nativeMediaTypeForMediaType:mediaType],
                                      init.nativeInit);
  if (!nativeTransceiverOrError.ok()) {
    RTCLogError(@"Failed to add transceiver %@: %s",
                [RTCRtpReceiver stringForMediaType:mediaType],
                nativeTransceiverOrError.error().message());
    return nil;
  }
  return
      [[RTCRtpTransceiver alloc] initWithNativeRtpTransceiver:nativeTransceiverOrError.MoveValue()];
}

- (void)offerForConstraints:(RTCMediaConstraints *)constraints
          completionHandler:
    (void (^)(RTCSessionDescription *sessionDescription,
              NSError *error))completionHandler {
  rtc::scoped_refptr<webrtz::CreateSessionDescriptionObserverAdapter>
      observer(new rtc::RefCountedObject
          <webrtz::CreateSessionDescriptionObserverAdapter>(completionHandler));
  _peerConnection->CreateOffer(observer, constraints.nativeConstraints.get());
}

- (void)answerForConstraints:(RTCMediaConstraints *)constraints
           completionHandler:
    (void (^)(RTCSessionDescription *sessionDescription,
              NSError *error))completionHandler {
  rtc::scoped_refptr<webrtz::CreateSessionDescriptionObserverAdapter>
      observer(new rtc::RefCountedObject
          <webrtz::CreateSessionDescriptionObserverAdapter>(completionHandler));
  _peerConnection->CreateAnswer(observer, constraints.nativeConstraints.get());
}

- (void)setLocalDescription:(RTCSessionDescription *)sdp
          completionHandler:(void (^)(NSError *error))completionHandler {
  rtc::scoped_refptr<webrtz::SetSessionDescriptionObserverAdapter> observer(
      new rtc::RefCountedObject<webrtz::SetSessionDescriptionObserverAdapter>(
          completionHandler));
  _peerConnection->SetLocalDescription(observer, sdp.nativeDescription);
}

- (void)setRemoteDescription:(RTCSessionDescription *)sdp
           completionHandler:(void (^)(NSError *error))completionHandler {
  rtc::scoped_refptr<webrtz::SetSessionDescriptionObserverAdapter> observer(
      new rtc::RefCountedObject<webrtz::SetSessionDescriptionObserverAdapter>(
          completionHandler));
  _peerConnection->SetRemoteDescription(observer, sdp.nativeDescription);
}

- (BOOL)setBweMinBitrateBps:(nullable NSNumber *)minBitrateBps
          currentBitrateBps:(nullable NSNumber *)currentBitrateBps
              maxBitrateBps:(nullable NSNumber *)maxBitrateBps {
  webrtz::PeerConnectionInterface::BitrateParameters params;
  if (minBitrateBps != nil) {
    params.min_bitrate_bps = rtc::Optional<int>(minBitrateBps.intValue);
  }
  if (currentBitrateBps != nil) {
    params.current_bitrate_bps = rtc::Optional<int>(currentBitrateBps.intValue);
  }
  if (maxBitrateBps != nil) {
    params.max_bitrate_bps = rtc::Optional<int>(maxBitrateBps.intValue);
  }
  return _peerConnection->SetBitrate(params).ok();
}

- (void)setBitrateAllocationStrategy:
        (std::unique_ptr<rtc::BitrateAllocationStrategy>)bitrateAllocationStrategy {
  _peerConnection->SetBitrateAllocationStrategy(std::move(bitrateAllocationStrategy));
}

- (BOOL)startRtcEventLogWithFilePath:(NSString *)filePath
                      maxSizeInBytes:(int64_t)maxSizeInBytes {
  RTC_DCHECK(filePath.length);
  RTC_DCHECK_GT(maxSizeInBytes, 0);
  RTC_DCHECK(!_hasStartedRtcEventLog);
  if (_hasStartedRtcEventLog) {
    RTCLogError(@"Event logging already started.");
    return NO;
  }
  int fd = open(filePath.UTF8String, O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR);
  if (fd < 0) {
    RTCLogError(@"Error opening file: %@. Error: %d", filePath, errno);
    return NO;
  }
  _hasStartedRtcEventLog =
      _peerConnection->StartRtcEventLog(fd, maxSizeInBytes);
  return _hasStartedRtcEventLog;
}

- (void)stopRtcEventLog {
  _peerConnection->StopRtcEventLog();
  _hasStartedRtcEventLog = NO;
}

- (RTCRtpSender *)senderWithKind:(NSString *)kind
                        streamId:(NSString *)streamId {
  std::string nativeKind = [NSString stdStringForString:kind];
  std::string nativeStreamId = [NSString stdStringForString:streamId];
  rtc::scoped_refptr<webrtz::RtpSenderInterface> nativeSender(
      _peerConnection->CreateSender(nativeKind, nativeStreamId));
  return nativeSender ?
      [[RTCRtpSender alloc] initWithNativeRtpSender:nativeSender]
      : nil;
}

- (NSArray<RTCRtpSender *> *)senders {
  std::vector<rtc::scoped_refptr<webrtz::RtpSenderInterface>> nativeSenders(
      _peerConnection->GetSenders());
  NSMutableArray *senders = [[NSMutableArray alloc] init];
  for (const auto &nativeSender : nativeSenders) {
    RTCRtpSender *sender =
        [[RTCRtpSender alloc] initWithNativeRtpSender:nativeSender];
    [senders addObject:sender];
  }
  return senders;
}

- (NSArray<RTCRtpReceiver *> *)receivers {
  std::vector<rtc::scoped_refptr<webrtz::RtpReceiverInterface>> nativeReceivers(
      _peerConnection->GetReceivers());
  NSMutableArray *receivers = [[NSMutableArray alloc] init];
  for (const auto &nativeReceiver : nativeReceivers) {
    RTCRtpReceiver *receiver =
        [[RTCRtpReceiver alloc] initWithNativeRtpReceiver:nativeReceiver];
    [receivers addObject:receiver];
  }
  return receivers;
}

- (NSArray<RTCRtpTransceiver *> *)transceivers {
  std::vector<rtc::scoped_refptr<webrtz::RtpTransceiverInterface>> nativeTransceivers(
      _peerConnection->GetTransceivers());
  NSMutableArray *transceivers = [[NSMutableArray alloc] init];
  for (auto nativeTransceiver : nativeTransceivers) {
    RTCRtpTransceiver *transceiver =
        [[RTCRtpTransceiver alloc] initWithNativeRtpTransceiver:nativeTransceiver];
    [transceivers addObject:transceiver];
  }
  return transceivers;
}

#pragma mark - Private

+ (webrtz::PeerConnectionInterface::SignalingState)nativeSignalingStateForState:
    (RTCSignalingState)state {
  switch (state) {
    case RTCSignalingStateStable:
      return webrtz::PeerConnectionInterface::kStable;
    case RTCSignalingStateHaveLocalOffer:
      return webrtz::PeerConnectionInterface::kHaveLocalOffer;
    case RTCSignalingStateHaveLocalPrAnswer:
      return webrtz::PeerConnectionInterface::kHaveLocalPrAnswer;
    case RTCSignalingStateHaveRemoteOffer:
      return webrtz::PeerConnectionInterface::kHaveRemoteOffer;
    case RTCSignalingStateHaveRemotePrAnswer:
      return webrtz::PeerConnectionInterface::kHaveRemotePrAnswer;
    case RTCSignalingStateClosed:
      return webrtz::PeerConnectionInterface::kClosed;
  }
}

+ (RTCSignalingState)signalingStateForNativeState:
    (webrtz::PeerConnectionInterface::SignalingState)nativeState {
  switch (nativeState) {
    case webrtz::PeerConnectionInterface::kStable:
      return RTCSignalingStateStable;
    case webrtz::PeerConnectionInterface::kHaveLocalOffer:
      return RTCSignalingStateHaveLocalOffer;
    case webrtz::PeerConnectionInterface::kHaveLocalPrAnswer:
      return RTCSignalingStateHaveLocalPrAnswer;
    case webrtz::PeerConnectionInterface::kHaveRemoteOffer:
      return RTCSignalingStateHaveRemoteOffer;
    case webrtz::PeerConnectionInterface::kHaveRemotePrAnswer:
      return RTCSignalingStateHaveRemotePrAnswer;
    case webrtz::PeerConnectionInterface::kClosed:
      return RTCSignalingStateClosed;
  }
}

+ (NSString *)stringForSignalingState:(RTCSignalingState)state {
  switch (state) {
    case RTCSignalingStateStable:
      return @"STABLE";
    case RTCSignalingStateHaveLocalOffer:
      return @"HAVE_LOCAL_OFFER";
    case RTCSignalingStateHaveLocalPrAnswer:
      return @"HAVE_LOCAL_PRANSWER";
    case RTCSignalingStateHaveRemoteOffer:
      return @"HAVE_REMOTE_OFFER";
    case RTCSignalingStateHaveRemotePrAnswer:
      return @"HAVE_REMOTE_PRANSWER";
    case RTCSignalingStateClosed:
      return @"CLOSED";
  }
}

+ (webrtz::PeerConnectionInterface::IceConnectionState)
    nativeIceConnectionStateForState:(RTCIceConnectionState)state {
  switch (state) {
    case RTCIceConnectionStateNew:
      return webrtz::PeerConnectionInterface::kIceConnectionNew;
    case RTCIceConnectionStateChecking:
      return webrtz::PeerConnectionInterface::kIceConnectionChecking;
    case RTCIceConnectionStateConnected:
      return webrtz::PeerConnectionInterface::kIceConnectionConnected;
    case RTCIceConnectionStateCompleted:
      return webrtz::PeerConnectionInterface::kIceConnectionCompleted;
    case RTCIceConnectionStateFailed:
      return webrtz::PeerConnectionInterface::kIceConnectionFailed;
    case RTCIceConnectionStateDisconnected:
      return webrtz::PeerConnectionInterface::kIceConnectionDisconnected;
    case RTCIceConnectionStateClosed:
      return webrtz::PeerConnectionInterface::kIceConnectionClosed;
    case RTCIceConnectionStateCount:
      return webrtz::PeerConnectionInterface::kIceConnectionMax;
  }
}

+ (RTCIceConnectionState)iceConnectionStateForNativeState:
    (webrtz::PeerConnectionInterface::IceConnectionState)nativeState {
  switch (nativeState) {
    case webrtz::PeerConnectionInterface::kIceConnectionNew:
      return RTCIceConnectionStateNew;
    case webrtz::PeerConnectionInterface::kIceConnectionChecking:
      return RTCIceConnectionStateChecking;
    case webrtz::PeerConnectionInterface::kIceConnectionConnected:
      return RTCIceConnectionStateConnected;
    case webrtz::PeerConnectionInterface::kIceConnectionCompleted:
      return RTCIceConnectionStateCompleted;
    case webrtz::PeerConnectionInterface::kIceConnectionFailed:
      return RTCIceConnectionStateFailed;
    case webrtz::PeerConnectionInterface::kIceConnectionDisconnected:
      return RTCIceConnectionStateDisconnected;
    case webrtz::PeerConnectionInterface::kIceConnectionClosed:
      return RTCIceConnectionStateClosed;
    case webrtz::PeerConnectionInterface::kIceConnectionMax:
      return RTCIceConnectionStateCount;
  }
}

+ (NSString *)stringForIceConnectionState:(RTCIceConnectionState)state {
  switch (state) {
    case RTCIceConnectionStateNew:
      return @"NEW";
    case RTCIceConnectionStateChecking:
      return @"CHECKING";
    case RTCIceConnectionStateConnected:
      return @"CONNECTED";
    case RTCIceConnectionStateCompleted:
      return @"COMPLETED";
    case RTCIceConnectionStateFailed:
      return @"FAILED";
    case RTCIceConnectionStateDisconnected:
      return @"DISCONNECTED";
    case RTCIceConnectionStateClosed:
      return @"CLOSED";
    case RTCIceConnectionStateCount:
      return @"COUNT";
  }
}

+ (webrtz::PeerConnectionInterface::IceGatheringState)
    nativeIceGatheringStateForState:(RTCIceGatheringState)state {
  switch (state) {
    case RTCIceGatheringStateNew:
      return webrtz::PeerConnectionInterface::kIceGatheringNew;
    case RTCIceGatheringStateGathering:
      return webrtz::PeerConnectionInterface::kIceGatheringGathering;
    case RTCIceGatheringStateComplete:
      return webrtz::PeerConnectionInterface::kIceGatheringComplete;
  }
}

+ (RTCIceGatheringState)iceGatheringStateForNativeState:
    (webrtz::PeerConnectionInterface::IceGatheringState)nativeState {
  switch (nativeState) {
    case webrtz::PeerConnectionInterface::kIceGatheringNew:
      return RTCIceGatheringStateNew;
    case webrtz::PeerConnectionInterface::kIceGatheringGathering:
      return RTCIceGatheringStateGathering;
    case webrtz::PeerConnectionInterface::kIceGatheringComplete:
      return RTCIceGatheringStateComplete;
  }
}

+ (NSString *)stringForIceGatheringState:(RTCIceGatheringState)state {
  switch (state) {
    case RTCIceGatheringStateNew:
      return @"NEW";
    case RTCIceGatheringStateGathering:
      return @"GATHERING";
    case RTCIceGatheringStateComplete:
      return @"COMPLETE";
  }
}

+ (webrtz::PeerConnectionInterface::StatsOutputLevel)
    nativeStatsOutputLevelForLevel:(RTCStatsOutputLevel)level {
  switch (level) {
    case RTCStatsOutputLevelStandard:
      return webrtz::PeerConnectionInterface::kStatsOutputLevelStandard;
    case RTCStatsOutputLevelDebug:
      return webrtz::PeerConnectionInterface::kStatsOutputLevelDebug;
  }
}

- (rtc::scoped_refptr<webrtz::PeerConnectionInterface>)nativePeerConnection {
  return _peerConnection;
}

@end
