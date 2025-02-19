// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/test/icu_test_util.h"

#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/i18n/icu_util.h"
#include "base/i18n/rtl.h"
#include "third_party/icu/source/common/unicode/uloc.h"

namespace base {
namespace test {

ScopedRestoreICUDefaultLocale::ScopedRestoreICUDefaultLocale()
    : default_locale_(uloc_getDefault()) {}

ScopedRestoreICUDefaultLocale::~ScopedRestoreICUDefaultLocale() {
  i18n::SetICUDefaultLocale(default_locale_.data());
}

void InitializeICUForTesting() {
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kTestDoNotInitializeIcu)) {
    base::i18n::AllowMultipleInitializeCallsForTesting();
    i18n::InitializeICU();
  }
}

}  // namespace test
}  // namespace base
