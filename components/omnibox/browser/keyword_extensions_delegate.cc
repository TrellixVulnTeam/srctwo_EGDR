// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/omnibox/browser/keyword_extensions_delegate.h"

KeywordExtensionsDelegate::KeywordExtensionsDelegate(
    KeywordProvider* provider) {
}

KeywordExtensionsDelegate::~KeywordExtensionsDelegate() {
}

void KeywordExtensionsDelegate::IncrementInputId() {
}

bool KeywordExtensionsDelegate::IsEnabledExtension(
    const std::string& extension_id) {
  return false;
}

bool KeywordExtensionsDelegate::Start(const AutocompleteInput& input,
                                      bool minimal_changes,
                                      const TemplateURL* template_url,
                                      const base::string16& remaining_input) {
  return false;
}

void KeywordExtensionsDelegate::EnterExtensionKeywordMode(
    const std::string& extension_id) {
}

void KeywordExtensionsDelegate::MaybeEndExtensionKeywordMode() {
}
