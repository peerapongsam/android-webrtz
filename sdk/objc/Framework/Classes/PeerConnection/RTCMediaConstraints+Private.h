/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "WebRTC/RTCMediaConstraints.h"

#include <memory>

#include "api/mediaconstraintsinterface.h"

namespace webrtz {

class MediaConstraints : public MediaConstraintsInterface {
 public:
  virtual ~MediaConstraints();
  MediaConstraints();
  MediaConstraints(
      const MediaConstraintsInterface::Constraints& mandatory,
      const MediaConstraintsInterface::Constraints& optional);
  virtual const Constraints& GetMandatory() const;
  virtual const Constraints& GetOptional() const;

 private:
  MediaConstraintsInterface::Constraints mandatory_;
  MediaConstraintsInterface::Constraints optional_;
};

}  // namespace webrtz


NS_ASSUME_NONNULL_BEGIN

@interface RTCMediaConstraints ()

/**
 * A MediaConstraints representation of this RTCMediaConstraints object. This is
 * needed to pass to the underlying C++ APIs.
 */
- (std::unique_ptr<webrtz::MediaConstraints>)nativeConstraints;

/** Return a native Constraints object representing these constraints */
+ (webrtz::MediaConstraintsInterface::Constraints)
    nativeConstraintsForConstraints:
        (NSDictionary<NSString *, NSString *> *)constraints;

@end

NS_ASSUME_NONNULL_END
