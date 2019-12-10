/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_PROCESSING_AEC3_MOCK_MOCK_RENDER_DELAY_CONTROLLER_H_
#define MODULES_AUDIO_PROCESSING_AEC3_MOCK_MOCK_RENDER_DELAY_CONTROLLER_H_

#include "api/array_view.h"
#include "api/optional.h"
#include "modules/audio_processing/aec3/downsampled_render_buffer.h"
#include "modules/audio_processing/aec3/render_delay_controller.h"
#include "test/gmock.h"

namespace webrtz {
namespace test {

class MockRenderDelayController : public RenderDelayController {
 public:
  virtual ~MockRenderDelayController() = default;

  MOCK_METHOD0(Reset, void());
  MOCK_METHOD0(LogRenderCall, void());
  MOCK_METHOD4(
      GetDelay,
      rtc::Optional<DelayEstimate>(const DownsampledRenderBuffer& render_buffer,
                                   size_t render_delay_buffer_delay,
                                   const rtc::Optional<int>& echo_remover_delay,
                                   rtc::ArrayView<const float> capture));
};

}  // namespace test
}  // namespace webrtz

#endif  // MODULES_AUDIO_PROCESSING_AEC3_MOCK_MOCK_RENDER_DELAY_CONTROLLER_H_
