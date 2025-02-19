// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebSpellCheckPanelHostClient_h
#define WebSpellCheckPanelHostClient_h

namespace blink {

class WebString;

class WebSpellCheckPanelHostClient {
 public:
  // Show or hide the spelling panel UI.
  virtual void ShowSpellingUI(bool show) = 0;

  // Returns true if the spelling panel UI is showing.
  virtual bool IsShowingSpellingUI() = 0;

  // Update the spelling panel UI with the given |word|.
  virtual void UpdateSpellingUIWithMisspelledWord(const WebString& word) = 0;
};

}  // namespace blink

#endif
