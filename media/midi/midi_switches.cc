// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"
#include "media/midi/midi_switches.h"

namespace midi {
namespace features {

#if defined(OS_ANDROID)
const base::Feature kMidiManagerAndroid{"MidiManagerAndroid",
                                        base::FEATURE_DISABLED_BY_DEFAULT};
#endif

#if defined(OS_WIN)
const base::Feature kMidiManagerWinrt{"MidiManagerWinrt",
                                      base::FEATURE_DISABLED_BY_DEFAULT};
#endif

#if defined(OS_CHROMEOS)
const base::Feature kMidiManagerCros{"MidiManagerCros",
                                     base::FEATURE_DISABLED_BY_DEFAULT};
#endif

const base::Feature kMidiManagerDynamicInstantiation{
    "MidiManagerDynamicInstantiation", base::FEATURE_DISABLED_BY_DEFAULT};

}  // namespace features
}  // namespace midi
