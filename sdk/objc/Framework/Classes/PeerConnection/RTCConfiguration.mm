/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCConfiguration+Private.h"

#include <memory>

#import "RTCConfiguration+Native.h"
#import "RTCIceServer+Private.h"
#import "RTCIntervalRange+Private.h"
#import "WebRTC/RTCLogging.h"

#include "rtc_base/rtccertificategenerator.h"
#include "rtc_base/sslidentity.h"

@implementation RTCConfiguration

@synthesize iceServers = _iceServers;
@synthesize iceTransportPolicy = _iceTransportPolicy;
@synthesize bundlePolicy = _bundlePolicy;
@synthesize rtcpMuxPolicy = _rtcpMuxPolicy;
@synthesize tcpCandidatePolicy = _tcpCandidatePolicy;
@synthesize candidateNetworkPolicy = _candidateNetworkPolicy;
@synthesize continualGatheringPolicy = _continualGatheringPolicy;
@synthesize maxIPv6Networks = _maxIPv6Networks;
@synthesize disableLinkLocalNetworks = _disableLinkLocalNetworks;
@synthesize audioJitterBufferMaxPackets = _audioJitterBufferMaxPackets;
@synthesize audioJitterBufferFastAccelerate = _audioJitterBufferFastAccelerate;
@synthesize iceConnectionReceivingTimeout = _iceConnectionReceivingTimeout;
@synthesize iceBackupCandidatePairPingInterval =
    _iceBackupCandidatePairPingInterval;
@synthesize keyType = _keyType;
@synthesize iceCandidatePoolSize = _iceCandidatePoolSize;
@synthesize shouldPruneTurnPorts = _shouldPruneTurnPorts;
@synthesize shouldPresumeWritableWhenFullyRelayed =
    _shouldPresumeWritableWhenFullyRelayed;
@synthesize iceCheckMinInterval = _iceCheckMinInterval;
@synthesize iceRegatherIntervalRange = _iceRegatherIntervalRange;
@synthesize sdpSemantics = _sdpSemantics;
@synthesize turnCustomizer = _turnCustomizer;

- (instancetype)init {
  // Copy defaults.
  webrtz::PeerConnectionInterface::RTCConfiguration config(
    webrtz::PeerConnectionInterface::RTCConfigurationType::kAggressive);
  return [self initWithNativeConfiguration:config];
}

- (instancetype)initWithNativeConfiguration:
    (const webrtz::PeerConnectionInterface::RTCConfiguration &)config {
  if (self = [super init]) {
    NSMutableArray *iceServers = [NSMutableArray array];
    for (const webrtz::PeerConnectionInterface::IceServer& server : config.servers) {
      RTCIceServer *iceServer = [[RTCIceServer alloc] initWithNativeServer:server];
      [iceServers addObject:iceServer];
    }
    _iceServers = iceServers;
    _iceTransportPolicy =
        [[self class] transportPolicyForTransportsType:config.type];
    _bundlePolicy =
        [[self class] bundlePolicyForNativePolicy:config.bundle_policy];
    _rtcpMuxPolicy =
        [[self class] rtcpMuxPolicyForNativePolicy:config.rtcp_mux_policy];
    _tcpCandidatePolicy = [[self class] tcpCandidatePolicyForNativePolicy:
        config.tcp_candidate_policy];
    _candidateNetworkPolicy = [[self class]
        candidateNetworkPolicyForNativePolicy:config.candidate_network_policy];
    webrtz::PeerConnectionInterface::ContinualGatheringPolicy nativePolicy =
    config.continual_gathering_policy;
    _continualGatheringPolicy =
        [[self class] continualGatheringPolicyForNativePolicy:nativePolicy];
    _maxIPv6Networks = config.max_ipv6_networks;
    _disableLinkLocalNetworks = config.disable_link_local_networks;
    _audioJitterBufferMaxPackets = config.audio_jitter_buffer_max_packets;
    _audioJitterBufferFastAccelerate = config.audio_jitter_buffer_fast_accelerate;
    _iceConnectionReceivingTimeout = config.ice_connection_receiving_timeout;
    _iceBackupCandidatePairPingInterval =
        config.ice_backup_candidate_pair_ping_interval;
    _keyType = RTCEncryptionKeyTypeECDSA;
    _iceCandidatePoolSize = config.ice_candidate_pool_size;
    _shouldPruneTurnPorts = config.prune_turn_ports;
    _shouldPresumeWritableWhenFullyRelayed =
        config.presume_writable_when_fully_relayed;
    if (config.ice_check_min_interval) {
      _iceCheckMinInterval =
          [NSNumber numberWithInt:*config.ice_check_min_interval];
    }
    if (config.ice_regather_interval_range) {
      const rtc::IntervalRange &nativeIntervalRange = config.ice_regather_interval_range.value();
      _iceRegatherIntervalRange =
          [[RTCIntervalRange alloc] initWithNativeIntervalRange:nativeIntervalRange];
    }
    _sdpSemantics = [[self class] sdpSemanticsForNativeSdpSemantics:config.sdp_semantics];
    _turnCustomizer = config.turn_customizer;
  }
  return self;
}

- (NSString *)description {
  static NSString *formatString =
      @"RTCConfiguration: "
      @"{\n%@\n%@\n%@\n%@\n%@\n%@\n%@\n%@\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%@\n%@\n%d\n%d\n}\n";

  return [NSString
      stringWithFormat:formatString,
                       _iceServers,
                       [[self class] stringForTransportPolicy:_iceTransportPolicy],
                       [[self class] stringForBundlePolicy:_bundlePolicy],
                       [[self class] stringForRtcpMuxPolicy:_rtcpMuxPolicy],
                       [[self class] stringForTcpCandidatePolicy:_tcpCandidatePolicy],
                       [[self class] stringForCandidateNetworkPolicy:_candidateNetworkPolicy],
                       [[self class] stringForContinualGatheringPolicy:_continualGatheringPolicy],
                       [[self class] stringForSdpSemantics:_sdpSemantics],
                       _audioJitterBufferMaxPackets,
                       _audioJitterBufferFastAccelerate,
                       _iceConnectionReceivingTimeout,
                       _iceBackupCandidatePairPingInterval,
                       _iceCandidatePoolSize,
                       _shouldPruneTurnPorts,
                       _shouldPresumeWritableWhenFullyRelayed,
                       _iceCheckMinInterval,
                       _iceRegatherIntervalRange,
                       _disableLinkLocalNetworks,
                       _maxIPv6Networks];
}

#pragma mark - Private

- (webrtz::PeerConnectionInterface::RTCConfiguration *)
    createNativeConfiguration {
  std::unique_ptr<webrtz::PeerConnectionInterface::RTCConfiguration>
      nativeConfig(new webrtz::PeerConnectionInterface::RTCConfiguration(
          webrtz::PeerConnectionInterface::RTCConfigurationType::kAggressive));

  for (RTCIceServer *iceServer in _iceServers) {
    nativeConfig->servers.push_back(iceServer.nativeServer);
  }
  nativeConfig->type =
      [[self class] nativeTransportsTypeForTransportPolicy:_iceTransportPolicy];
  nativeConfig->bundle_policy =
      [[self class] nativeBundlePolicyForPolicy:_bundlePolicy];
  nativeConfig->rtcp_mux_policy =
      [[self class] nativeRtcpMuxPolicyForPolicy:_rtcpMuxPolicy];
  nativeConfig->tcp_candidate_policy =
      [[self class] nativeTcpCandidatePolicyForPolicy:_tcpCandidatePolicy];
  nativeConfig->candidate_network_policy = [[self class]
      nativeCandidateNetworkPolicyForPolicy:_candidateNetworkPolicy];
  nativeConfig->continual_gathering_policy = [[self class]
      nativeContinualGatheringPolicyForPolicy:_continualGatheringPolicy];
  nativeConfig->max_ipv6_networks = _maxIPv6Networks;
  nativeConfig->disable_link_local_networks = _disableLinkLocalNetworks;
  nativeConfig->audio_jitter_buffer_max_packets = _audioJitterBufferMaxPackets;
  nativeConfig->audio_jitter_buffer_fast_accelerate =
      _audioJitterBufferFastAccelerate  ? true : false;
  nativeConfig->ice_connection_receiving_timeout =
      _iceConnectionReceivingTimeout;
  nativeConfig->ice_backup_candidate_pair_ping_interval =
      _iceBackupCandidatePairPingInterval;
  rtc::KeyType keyType =
      [[self class] nativeEncryptionKeyTypeForKeyType:_keyType];
  // Generate non-default certificate.
  if (keyType != rtc::KT_DEFAULT) {
    rtc::scoped_refptr<rtc::RTCCertificate> certificate =
        rtc::RTCCertificateGenerator::GenerateCertificate(
            rtc::KeyParams(keyType), rtc::Optional<uint64_t>());
    if (!certificate) {
      RTCLogError(@"Failed to generate certificate.");
      return nullptr;
    }
    nativeConfig->certificates.push_back(certificate);
  }
  nativeConfig->ice_candidate_pool_size = _iceCandidatePoolSize;
  nativeConfig->prune_turn_ports = _shouldPruneTurnPorts ? true : false;
  nativeConfig->presume_writable_when_fully_relayed =
      _shouldPresumeWritableWhenFullyRelayed ? true : false;
  if (_iceCheckMinInterval != nil) {
    nativeConfig->ice_check_min_interval =
        rtc::Optional<int>(_iceCheckMinInterval.intValue);
  }
  if (_iceRegatherIntervalRange != nil) {
    std::unique_ptr<rtc::IntervalRange> nativeIntervalRange(
        _iceRegatherIntervalRange.nativeIntervalRange);
    nativeConfig->ice_regather_interval_range =
        rtc::Optional<rtc::IntervalRange>(*nativeIntervalRange);
  }
  nativeConfig->sdp_semantics = [[self class] nativeSdpSemanticsForSdpSemantics:_sdpSemantics];
  if (_turnCustomizer) {
    nativeConfig->turn_customizer = _turnCustomizer;
  }
  return nativeConfig.release();
}

+ (webrtz::PeerConnectionInterface::IceTransportsType)
    nativeTransportsTypeForTransportPolicy:(RTCIceTransportPolicy)policy {
  switch (policy) {
    case RTCIceTransportPolicyNone:
      return webrtz::PeerConnectionInterface::kNone;
    case RTCIceTransportPolicyRelay:
      return webrtz::PeerConnectionInterface::kRelay;
    case RTCIceTransportPolicyNoHost:
      return webrtz::PeerConnectionInterface::kNoHost;
    case RTCIceTransportPolicyAll:
      return webrtz::PeerConnectionInterface::kAll;
  }
}

+ (RTCIceTransportPolicy)transportPolicyForTransportsType:
    (webrtz::PeerConnectionInterface::IceTransportsType)nativeType {
  switch (nativeType) {
    case webrtz::PeerConnectionInterface::kNone:
      return RTCIceTransportPolicyNone;
    case webrtz::PeerConnectionInterface::kRelay:
      return RTCIceTransportPolicyRelay;
    case webrtz::PeerConnectionInterface::kNoHost:
      return RTCIceTransportPolicyNoHost;
    case webrtz::PeerConnectionInterface::kAll:
      return RTCIceTransportPolicyAll;
  }
}

+ (NSString *)stringForTransportPolicy:(RTCIceTransportPolicy)policy {
  switch (policy) {
    case RTCIceTransportPolicyNone:
      return @"NONE";
    case RTCIceTransportPolicyRelay:
      return @"RELAY";
    case RTCIceTransportPolicyNoHost:
      return @"NO_HOST";
    case RTCIceTransportPolicyAll:
      return @"ALL";
  }
}

+ (webrtz::PeerConnectionInterface::BundlePolicy)nativeBundlePolicyForPolicy:
    (RTCBundlePolicy)policy {
  switch (policy) {
    case RTCBundlePolicyBalanced:
      return webrtz::PeerConnectionInterface::kBundlePolicyBalanced;
    case RTCBundlePolicyMaxCompat:
      return webrtz::PeerConnectionInterface::kBundlePolicyMaxCompat;
    case RTCBundlePolicyMaxBundle:
      return webrtz::PeerConnectionInterface::kBundlePolicyMaxBundle;
  }
}

+ (RTCBundlePolicy)bundlePolicyForNativePolicy:
    (webrtz::PeerConnectionInterface::BundlePolicy)nativePolicy {
  switch (nativePolicy) {
    case webrtz::PeerConnectionInterface::kBundlePolicyBalanced:
      return RTCBundlePolicyBalanced;
    case webrtz::PeerConnectionInterface::kBundlePolicyMaxCompat:
      return RTCBundlePolicyMaxCompat;
    case webrtz::PeerConnectionInterface::kBundlePolicyMaxBundle:
      return RTCBundlePolicyMaxBundle;
  }
}

+ (NSString *)stringForBundlePolicy:(RTCBundlePolicy)policy {
  switch (policy) {
    case RTCBundlePolicyBalanced:
      return @"BALANCED";
    case RTCBundlePolicyMaxCompat:
      return @"MAX_COMPAT";
    case RTCBundlePolicyMaxBundle:
      return @"MAX_BUNDLE";
  }
}

+ (webrtz::PeerConnectionInterface::RtcpMuxPolicy)nativeRtcpMuxPolicyForPolicy:
    (RTCRtcpMuxPolicy)policy {
  switch (policy) {
    case RTCRtcpMuxPolicyNegotiate:
      return webrtz::PeerConnectionInterface::kRtcpMuxPolicyNegotiate;
    case RTCRtcpMuxPolicyRequire:
      return webrtz::PeerConnectionInterface::kRtcpMuxPolicyRequire;
  }
}

+ (RTCRtcpMuxPolicy)rtcpMuxPolicyForNativePolicy:
    (webrtz::PeerConnectionInterface::RtcpMuxPolicy)nativePolicy {
  switch (nativePolicy) {
    case webrtz::PeerConnectionInterface::kRtcpMuxPolicyNegotiate:
      return RTCRtcpMuxPolicyNegotiate;
    case webrtz::PeerConnectionInterface::kRtcpMuxPolicyRequire:
      return RTCRtcpMuxPolicyRequire;
  }
}

+ (NSString *)stringForRtcpMuxPolicy:(RTCRtcpMuxPolicy)policy {
  switch (policy) {
    case RTCRtcpMuxPolicyNegotiate:
      return @"NEGOTIATE";
    case RTCRtcpMuxPolicyRequire:
      return @"REQUIRE";
  }
}

+ (webrtz::PeerConnectionInterface::TcpCandidatePolicy)
    nativeTcpCandidatePolicyForPolicy:(RTCTcpCandidatePolicy)policy {
  switch (policy) {
    case RTCTcpCandidatePolicyEnabled:
      return webrtz::PeerConnectionInterface::kTcpCandidatePolicyEnabled;
    case RTCTcpCandidatePolicyDisabled:
      return webrtz::PeerConnectionInterface::kTcpCandidatePolicyDisabled;
  }
}

+ (webrtz::PeerConnectionInterface::CandidateNetworkPolicy)
    nativeCandidateNetworkPolicyForPolicy:(RTCCandidateNetworkPolicy)policy {
  switch (policy) {
    case RTCCandidateNetworkPolicyAll:
      return webrtz::PeerConnectionInterface::kCandidateNetworkPolicyAll;
    case RTCCandidateNetworkPolicyLowCost:
      return webrtz::PeerConnectionInterface::kCandidateNetworkPolicyLowCost;
  }
}

+ (RTCTcpCandidatePolicy)tcpCandidatePolicyForNativePolicy:
    (webrtz::PeerConnectionInterface::TcpCandidatePolicy)nativePolicy {
  switch (nativePolicy) {
    case webrtz::PeerConnectionInterface::kTcpCandidatePolicyEnabled:
      return RTCTcpCandidatePolicyEnabled;
    case webrtz::PeerConnectionInterface::kTcpCandidatePolicyDisabled:
      return RTCTcpCandidatePolicyDisabled;
  }
}

+ (NSString *)stringForTcpCandidatePolicy:(RTCTcpCandidatePolicy)policy {
  switch (policy) {
    case RTCTcpCandidatePolicyEnabled:
      return @"TCP_ENABLED";
    case RTCTcpCandidatePolicyDisabled:
      return @"TCP_DISABLED";
  }
}

+ (RTCCandidateNetworkPolicy)candidateNetworkPolicyForNativePolicy:
    (webrtz::PeerConnectionInterface::CandidateNetworkPolicy)nativePolicy {
  switch (nativePolicy) {
    case webrtz::PeerConnectionInterface::kCandidateNetworkPolicyAll:
      return RTCCandidateNetworkPolicyAll;
    case webrtz::PeerConnectionInterface::kCandidateNetworkPolicyLowCost:
      return RTCCandidateNetworkPolicyLowCost;
  }
}

+ (NSString *)stringForCandidateNetworkPolicy:
    (RTCCandidateNetworkPolicy)policy {
  switch (policy) {
    case RTCCandidateNetworkPolicyAll:
      return @"CANDIDATE_ALL_NETWORKS";
    case RTCCandidateNetworkPolicyLowCost:
      return @"CANDIDATE_LOW_COST_NETWORKS";
  }
}

+ (webrtz::PeerConnectionInterface::ContinualGatheringPolicy)
    nativeContinualGatheringPolicyForPolicy:
        (RTCContinualGatheringPolicy)policy {
  switch (policy) {
    case RTCContinualGatheringPolicyGatherOnce:
      return webrtz::PeerConnectionInterface::GATHER_ONCE;
    case RTCContinualGatheringPolicyGatherContinually:
      return webrtz::PeerConnectionInterface::GATHER_CONTINUALLY;
  }
}

+ (RTCContinualGatheringPolicy)continualGatheringPolicyForNativePolicy:
    (webrtz::PeerConnectionInterface::ContinualGatheringPolicy)nativePolicy {
  switch (nativePolicy) {
    case webrtz::PeerConnectionInterface::GATHER_ONCE:
      return RTCContinualGatheringPolicyGatherOnce;
    case webrtz::PeerConnectionInterface::GATHER_CONTINUALLY:
      return RTCContinualGatheringPolicyGatherContinually;
  }
}

+ (NSString *)stringForContinualGatheringPolicy:
    (RTCContinualGatheringPolicy)policy {
  switch (policy) {
    case RTCContinualGatheringPolicyGatherOnce:
      return @"GATHER_ONCE";
    case RTCContinualGatheringPolicyGatherContinually:
      return @"GATHER_CONTINUALLY";
  }
}

+ (rtc::KeyType)nativeEncryptionKeyTypeForKeyType:
    (RTCEncryptionKeyType)keyType {
  switch (keyType) {
    case RTCEncryptionKeyTypeRSA:
      return rtc::KT_RSA;
    case RTCEncryptionKeyTypeECDSA:
      return rtc::KT_ECDSA;
  }
}

+ (webrtz::SdpSemantics)nativeSdpSemanticsForSdpSemantics:(RTCSdpSemantics)sdpSemantics {
  switch (sdpSemantics) {
    case RTCSdpSemanticsDefault:
      return webrtz::SdpSemantics::kDefault;
    case RTCSdpSemanticsPlanB:
      return webrtz::SdpSemantics::kPlanB;
    case RTCSdpSemanticsUnifiedPlan:
      return webrtz::SdpSemantics::kUnifiedPlan;
  }
}

+ (RTCSdpSemantics)sdpSemanticsForNativeSdpSemantics:(webrtz::SdpSemantics)sdpSemantics {
  switch (sdpSemantics) {
    case webrtz::SdpSemantics::kDefault:
      return RTCSdpSemanticsDefault;
    case webrtz::SdpSemantics::kPlanB:
      return RTCSdpSemanticsPlanB;
    case webrtz::SdpSemantics::kUnifiedPlan:
      return RTCSdpSemanticsUnifiedPlan;
  }
}

+ (NSString *)stringForSdpSemantics:(RTCSdpSemantics)sdpSemantics {
  switch (sdpSemantics) {
    case RTCSdpSemanticsDefault:
      return @"DEFAULT";
    case RTCSdpSemanticsPlanB:
      return @"PLAN_B";
    case RTCSdpSemanticsUnifiedPlan:
      return @"UNIFIED_PLAN";
  }
}

@end
