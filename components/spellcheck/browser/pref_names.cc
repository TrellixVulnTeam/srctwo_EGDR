// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/spellcheck/browser/pref_names.h"

namespace spellcheck {
namespace prefs {

// Boolean pref to define the default values for using spellchecker.
const char kEnableSpellcheck[] = "browser.enable_spellchecking";

// String which represents the dictionary name for our spell-checker.
// This is an old preference that is being migrated to kSpellCheckDictionaries.
const char kSpellCheckDictionary[] = "spellcheck.dictionary";

// List of strings representing the dictionary names for our spell-checker.
const char kSpellCheckDictionaries[] = "spellcheck.dictionaries";

// String which represents whether we use the spelling service.
const char kSpellCheckUseSpellingService[] = "spellcheck.use_spelling_service";

}  // namespace prefs
}  // namespace spellcheck
