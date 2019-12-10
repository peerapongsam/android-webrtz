/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCMediaConstraints+Private.h"

#import "NSString+StdString.h"

#include <memory>

NSString * const kRTCMediaConstraintsMinAspectRatio =
    @(webrtz::MediaConstraintsInterface::kMinAspectRatio);
NSString * const kRTCMediaConstraintsMaxAspectRatio =
    @(webrtz::MediaConstraintsInterface::kMaxAspectRatio);
NSString * const kRTCMediaConstraintsMinWidth =
    @(webrtz::MediaConstraintsInterface::kMinWidth);
NSString * const kRTCMediaConstraintsMaxWidth =
    @(webrtz::MediaConstraintsInterface::kMaxWidth);
NSString * const kRTCMediaConstraintsMinHeight =
    @(webrtz::MediaConstraintsInterface::kMinHeight);
NSString * const kRTCMediaConstraintsMaxHeight =
    @(webrtz::MediaConstraintsInterface::kMaxHeight);
NSString * const kRTCMediaConstraintsMinFrameRate =
    @(webrtz::MediaConstraintsInterface::kMinFrameRate);
NSString * const kRTCMediaConstraintsMaxFrameRate =
    @(webrtz::MediaConstraintsInterface::kMaxFrameRate);
NSString * const kRTCMediaConstraintsAudioNetworkAdaptorConfig =
    @(webrtz::MediaConstraintsInterface::kAudioNetworkAdaptorConfig);

NSString * const kRTCMediaConstraintsIceRestart =
    @(webrtz::MediaConstraintsInterface::kIceRestart);
NSString * const kRTCMediaConstraintsOfferToReceiveAudio =
    @(webrtz::MediaConstraintsInterface::kOfferToReceiveAudio);
NSString * const kRTCMediaConstraintsOfferToReceiveVideo =
    @(webrtz::MediaConstraintsInterface::kOfferToReceiveVideo);
NSString * const kRTCMediaConstraintsVoiceActivityDetection =
    @(webrtz::MediaConstraintsInterface::kVoiceActivityDetection);

NSString * const kRTCMediaConstraintsValueTrue =
    @(webrtz::MediaConstraintsInterface::kValueTrue);
NSString * const kRTCMediaConstraintsValueFalse =
    @(webrtz::MediaConstraintsInterface::kValueFalse);

namespace webrtz {

MediaConstraints::~MediaConstraints() {}

MediaConstraints::MediaConstraints() {}

MediaConstraints::MediaConstraints(
    const MediaConstraintsInterface::Constraints& mandatory,
    const MediaConstraintsInterface::Constraints& optional)
    : mandatory_(mandatory), optional_(optional) {}

const MediaConstraintsInterface::Constraints&
MediaConstraints::GetMandatory() const {
  return mandatory_;
}

const MediaConstraintsInterface::Constraints&
MediaConstraints::GetOptional() const {
  return optional_;
}

}  // namespace webrtz


@implementation RTCMediaConstraints {
  NSDictionary<NSString *, NSString *> *_mandatory;
  NSDictionary<NSString *, NSString *> *_optional;
}

- (instancetype)initWithMandatoryConstraints:
    (NSDictionary<NSString *, NSString *> *)mandatory
                         optionalConstraints:
    (NSDictionary<NSString *, NSString *> *)optional {
  if (self = [super init]) {
    _mandatory = [[NSDictionary alloc] initWithDictionary:mandatory
                                                copyItems:YES];
    _optional = [[NSDictionary alloc] initWithDictionary:optional
                                               copyItems:YES];
  }
  return self;
}

- (NSString *)description {
  return [NSString stringWithFormat:@"RTCMediaConstraints:\n%@\n%@",
                                    _mandatory,
                                    _optional];
}

#pragma mark - Private

- (std::unique_ptr<webrtz::MediaConstraints>)nativeConstraints {
  webrtz::MediaConstraintsInterface::Constraints mandatory =
      [[self class] nativeConstraintsForConstraints:_mandatory];
  webrtz::MediaConstraintsInterface::Constraints optional =
      [[self class] nativeConstraintsForConstraints:_optional];

  webrtz::MediaConstraints *nativeConstraints =
      new webrtz::MediaConstraints(mandatory, optional);
  return std::unique_ptr<webrtz::MediaConstraints>(nativeConstraints);
}

+ (webrtz::MediaConstraintsInterface::Constraints)
    nativeConstraintsForConstraints:
        (NSDictionary<NSString *, NSString *> *)constraints {
  webrtz::MediaConstraintsInterface::Constraints nativeConstraints;
  for (NSString *key in constraints) {
    NSAssert([key isKindOfClass:[NSString class]],
             @"%@ is not an NSString.", key);
    NSString *value = [constraints objectForKey:key];
    NSAssert([value isKindOfClass:[NSString class]],
             @"%@ is not an NSString.", value);
    if ([kRTCMediaConstraintsAudioNetworkAdaptorConfig isEqualToString:key]) {
      // This value is base64 encoded.
      NSData *charData = [[NSData alloc] initWithBase64EncodedString:value options:0];
      std::string configValue =
          std::string(reinterpret_cast<const char *>(charData.bytes), charData.length);
      nativeConstraints.push_back(webrtz::MediaConstraintsInterface::Constraint(
          key.stdString, configValue));
    } else {
      nativeConstraints.push_back(webrtz::MediaConstraintsInterface::Constraint(
          key.stdString, value.stdString));
    }
  }
  return nativeConstraints;
}

@end
