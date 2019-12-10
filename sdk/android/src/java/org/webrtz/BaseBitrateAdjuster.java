/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package org.webrtz;

/** BitrateAdjuster that tracks bitrate and framerate but does not adjust them. */
class BaseBitrateAdjuster implements BitrateAdjuster {
  protected int targetBitrateBps = 0;
  protected int targetFps = 0;

  @Override
  public void setTargets(int targetBitrateBps, int targetFps) {
    this.targetBitrateBps = targetBitrateBps;
    this.targetFps = targetFps;
  }

  @Override
  public void reportEncodedFrame(int size) {
    // No op.
  }

  @Override
  public int getAdjustedBitrateBps() {
    return targetBitrateBps;
  }

  @Override
  public int getCodecConfigFramerate() {
    return targetFps;
  }
}
