// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/viz/service/display_embedder/compositor_overlay_candidate_validator_win.h"

#include "cc/output/overlay_processor.h"

namespace viz {

CompositorOverlayCandidateValidatorWin::
    CompositorOverlayCandidateValidatorWin() {}

CompositorOverlayCandidateValidatorWin::
    ~CompositorOverlayCandidateValidatorWin() {}

void CompositorOverlayCandidateValidatorWin::GetStrategies(
    cc::OverlayProcessor::StrategyList* strategies) {}

void CompositorOverlayCandidateValidatorWin::CheckOverlaySupport(
    cc::OverlayCandidateList* candidates) {
  NOTIMPLEMENTED();
}

bool CompositorOverlayCandidateValidatorWin::AllowCALayerOverlays() {
  return false;
}

bool CompositorOverlayCandidateValidatorWin::AllowDCLayerOverlays() {
  return true;
}

void CompositorOverlayCandidateValidatorWin::SetSoftwareMirrorMode(
    bool enabled) {
  // Software mirroring isn't supported on Windows.
  NOTIMPLEMENTED();
}

}  // namespace viz
