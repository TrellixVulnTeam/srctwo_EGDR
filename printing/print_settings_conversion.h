// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PRINTING_PRINT_SETTINGS_CONVERSION_H_
#define PRINTING_PRINT_SETTINGS_CONVERSION_H_

#include "base/logging.h"
#include "printing/page_range.h"
#include "printing/printing_export.h"

namespace base {
class DictionaryValue;
}

namespace printing {

class PrintSettings;

bool PrintSettingsFromJobSettings(const base::DictionaryValue& job_settings,
                                  PrintSettings* print_settings);

// Use for debug only, because output is not completely consistent with format
// of |PrintSettingsFromJobSettings| input.
void PrintSettingsToJobSettingsDebug(const PrintSettings& settings,
                                     base::DictionaryValue* job_settings);

}  // namespace printing

#endif  // PRINTING_PRINT_SETTINGS_CONVERSION_H_
