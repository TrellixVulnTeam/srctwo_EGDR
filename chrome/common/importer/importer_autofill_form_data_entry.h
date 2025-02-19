// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_IMPORTER_IMPORTER_AUTOFILL_FORM_DATA_ENTRY_H_
#define CHROME_COMMON_IMPORTER_IMPORTER_AUTOFILL_FORM_DATA_ENTRY_H_

#include "base/strings/string16.h"
#include "base/time/time.h"

// Used as the target for importing form history from other browsers' profiles
// in the utility process.
struct ImporterAutofillFormDataEntry {
  ImporterAutofillFormDataEntry();
  ~ImporterAutofillFormDataEntry();

  // Name of input element.
  base::string16 name;

  // Value of input element.
  base::string16 value;

  // Number of times this name-value pair has been used.
  int times_used;

  // The date of the first time when this name-value pair was used.
  base::Time first_used;

  // The date of the last time when this name-value pair was used.
  base::Time last_used;
};

#endif  // CHROME_COMMON_IMPORTER_IMPORTER_AUTOFILL_FORM_DATA_ENTRY_H_
