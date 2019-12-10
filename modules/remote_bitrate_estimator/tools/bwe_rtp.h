/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_REMOTE_BITRATE_ESTIMATOR_TOOLS_BWE_RTP_H_
#define MODULES_REMOTE_BITRATE_ESTIMATOR_TOOLS_BWE_RTP_H_

#include <string>

namespace webrtz {
class Clock;
class RemoteBitrateEstimator;
class RemoteBitrateObserver;
class RtpHeaderParser;
namespace test {
class RtpFileReader;
}
}

bool ParseArgsAndSetupEstimator(
    int argc,
    char** argv,
    webrtz::Clock* clock,
    webrtz::RemoteBitrateObserver* observer,
    webrtz::test::RtpFileReader** rtp_reader,
    webrtz::RtpHeaderParser** parser,
    webrtz::RemoteBitrateEstimator** estimator,
    std::string* estimator_used);

#endif  // MODULES_REMOTE_BITRATE_ESTIMATOR_TOOLS_BWE_RTP_H_
