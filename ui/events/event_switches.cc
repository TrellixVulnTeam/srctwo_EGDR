// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"
#include "ui/events/event_switches.h"

namespace switches {

// Enable scroll prediction for scroll update events.
const char kEnableScrollPrediction[] = "enable-scroll-prediction";

// Enable compensation for unstable pinch zoom. Some touch screens display
// significant amount of wobble when moving a finger in a straight line. This
// makes two finger scroll trigger an oscillating pinch zoom. See
// crbug.com/394380 for details.
const char kCompensateForUnstablePinchZoom[] =
    "compensate-for-unstable-pinch-zoom";

#if defined(OS_LINUX)
// Tells chrome to interpret events from these devices as touch events. Only
// available with XInput 2 (i.e. X server 1.8 or above). The id's of the
// devices can be retrieved from 'xinput list'.
const char kTouchDevices[] = "touch-devices";
#endif

#if defined(USE_X11) || defined(USE_OZONE)
// Tells Chrome to do additional touch noise filtering. Should only be used if
// the driver level filtering is insufficient.
const char kExtraTouchNoiseFiltering[] = "touch-noise-filtering";

// The calibration factors given as "<left>,<right>,<top>,<bottom>".
const char kTouchCalibration[] = "touch-calibration";

// Tells Chrome to do edge touch filtering. Useful for convertible tablet.
const char kEdgeTouchFiltering[] = "edge-touch-filtering";
#endif

}  // namespace switches
