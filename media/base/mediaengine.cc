/*
 *  Copyright (c) 2004 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "media/base/mediaengine.h"

namespace cricket {

webrtz::RtpParameters CreateRtpParametersWithOneEncoding() {
  webrtz::RtpParameters parameters;
  webrtz::RtpEncodingParameters encoding;
  parameters.encodings.push_back(encoding);
  return parameters;
}

webrtz::RtpParameters CreateRtpParametersWithEncodings(StreamParams sp) {
  std::vector<uint32_t> primary_ssrcs;
  sp.GetPrimarySsrcs(&primary_ssrcs);
  size_t encoding_count = primary_ssrcs.size();

  std::vector<webrtz::RtpEncodingParameters> encodings(encoding_count);
  for (size_t i = 0; i < encodings.size(); ++i) {
    encodings[i].ssrc = primary_ssrcs[i];
  }
  webrtz::RtpParameters parameters;
  parameters.encodings = encodings;
  return parameters;
}

};  // namespace cricket
