// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// KeywordExtensionsDelegate contains the extensions-only logic used by
// KeywordProvider.
// This file contains the dummy implementation of KeywordExtensionsDelegate,
// which does nothing.

#ifndef COMPONENTS_OMNIBOX_BROWSER_KEYWORD_EXTENSIONS_DELEGATE_H_
#define COMPONENTS_OMNIBOX_BROWSER_KEYWORD_EXTENSIONS_DELEGATE_H_

#include <string>

#include "base/macros.h"
#include "base/strings/string16.h"

class AutocompleteInput;
class KeywordProvider;
class TemplateURL;

class KeywordExtensionsDelegate {
 public:
  explicit KeywordExtensionsDelegate(KeywordProvider* provider);
  virtual ~KeywordExtensionsDelegate();

  // Increments the input ID used to identify if the suggest results from an
  // extension are current.
  virtual void IncrementInputId();

  // Returns true if an extension is enabled.
  virtual bool IsEnabledExtension(const std::string& extension_id);

  // Handles the extensions portion of KeywordProvider::Start().
  // Depending on |minimal_changes| and whether |input| wants matches
  // synchronous or not, either updates the KeywordProvider's matches with
  // the existing suggestions or asks the |template_url|'s extension to provide
  // matches.
  // Returns true if this delegate should stay in extension keyword mode.
  virtual bool Start(const AutocompleteInput& input,
                     bool minimal_changes,
                     const TemplateURL* template_url,
                     const base::string16& remaining_input);

  // Tells the extension with |extension_id| that the user typed the omnibox
  // keyword.
  virtual void EnterExtensionKeywordMode(const std::string& extension_id);

  // If an extension previously entered extension keyword mode, exits extension
  // keyword mode. This happens when the user has cleared the keyword or closed
  // the omnibox popup.
  virtual void MaybeEndExtensionKeywordMode();

 private:
  DISALLOW_COPY_AND_ASSIGN(KeywordExtensionsDelegate);
};

#endif  // COMPONENTS_OMNIBOX_BROWSER_KEYWORD_EXTENSIONS_DELEGATE_H_
