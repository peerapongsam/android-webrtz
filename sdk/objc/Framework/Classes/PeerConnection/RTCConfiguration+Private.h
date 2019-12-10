/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "WebRTC/RTCConfiguration.h"

#include "api/peerconnectioninterface.h"

NS_ASSUME_NONNULL_BEGIN

@interface RTCConfiguration ()

+ (webrtz::PeerConnectionInterface::IceTransportsType)
    nativeTransportsTypeForTransportPolicy:(RTCIceTransportPolicy)policy;

+ (RTCIceTransportPolicy)transportPolicyForTransportsType:
    (webrtz::PeerConnectionInterface::IceTransportsType)nativeType;

+ (NSString *)stringForTransportPolicy:(RTCIceTransportPolicy)policy;

+ (webrtz::PeerConnectionInterface::BundlePolicy)nativeBundlePolicyForPolicy:
    (RTCBundlePolicy)policy;

+ (RTCBundlePolicy)bundlePolicyForNativePolicy:
    (webrtz::PeerConnectionInterface::BundlePolicy)nativePolicy;

+ (NSString *)stringForBundlePolicy:(RTCBundlePolicy)policy;

+ (webrtz::PeerConnectionInterface::RtcpMuxPolicy)nativeRtcpMuxPolicyForPolicy:
    (RTCRtcpMuxPolicy)policy;

+ (RTCRtcpMuxPolicy)rtcpMuxPolicyForNativePolicy:
    (webrtz::PeerConnectionInterface::RtcpMuxPolicy)nativePolicy;

+ (NSString *)stringForRtcpMuxPolicy:(RTCRtcpMuxPolicy)policy;

+ (webrtz::PeerConnectionInterface::TcpCandidatePolicy)
    nativeTcpCandidatePolicyForPolicy:(RTCTcpCandidatePolicy)policy;

+ (RTCTcpCandidatePolicy)tcpCandidatePolicyForNativePolicy:
    (webrtz::PeerConnectionInterface::TcpCandidatePolicy)nativePolicy;

+ (NSString *)stringForTcpCandidatePolicy:(RTCTcpCandidatePolicy)policy;

+ (webrtz::PeerConnectionInterface::CandidateNetworkPolicy)
    nativeCandidateNetworkPolicyForPolicy:(RTCCandidateNetworkPolicy)policy;

+ (RTCCandidateNetworkPolicy)candidateNetworkPolicyForNativePolicy:
    (webrtz::PeerConnectionInterface::CandidateNetworkPolicy)nativePolicy;

+ (NSString *)stringForCandidateNetworkPolicy:(RTCCandidateNetworkPolicy)policy;

+ (rtc::KeyType)nativeEncryptionKeyTypeForKeyType:(RTCEncryptionKeyType)keyType;

+ (webrtz::SdpSemantics)nativeSdpSemanticsForSdpSemantics:(RTCSdpSemantics)sdpSemantics;

+ (RTCSdpSemantics)sdpSemanticsForNativeSdpSemantics:(webrtz::SdpSemantics)sdpSemantics;

+ (NSString *)stringForSdpSemantics:(RTCSdpSemantics)sdpSemantics;

/**
 * RTCConfiguration struct representation of this RTCConfiguration. This is
 * needed to pass to the underlying C++ APIs.
 */
- (nullable webrtz::PeerConnectionInterface::RTCConfiguration *)createNativeConfiguration;

- (instancetype)initWithNativeConfiguration:
    (const webrtz::PeerConnectionInterface::RTCConfiguration &)config NS_DESIGNATED_INITIALIZER;

@end

NS_ASSUME_NONNULL_END
