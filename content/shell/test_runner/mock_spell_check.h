// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_TEST_RUNNER_MOCK_SPELL_CHECK_H_
#define CONTENT_SHELL_TEST_RUNNER_MOCK_SPELL_CHECK_H_

#include <vector>

#include "base/macros.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/platform/WebVector.h"
#include "third_party/WebKit/public/web/WebTextCheckingResult.h"

namespace test_runner {

// A mock implementation of a spell-checker used for WebKit tests.
// This class only implements the minimal functionalities required by WebKit
// tests, i.e. this class just compares the given string with known misspelled
// words in webkit tests and mark them as misspelled.
// Even though this is sufficient for webkit tests, this class is not suitable
// for any other usages.
class MockSpellCheck {
 public:
  static void FillSuggestionList(
      const blink::WebString& word,
      blink::WebVector<blink::WebString>* suggestions);

  MockSpellCheck();
  ~MockSpellCheck();

  // Checks the spellings of the specified text.
  // This function returns true if the text consists of valid words, and
  // returns false if it includes invalid words.
  // When the given text includes invalid words, this function sets the
  // position of the first invalid word to misspelledOffset, and the length of
  // the first invalid word to misspelledLength, respectively.
  // For example, when the given text is "   zz zz", this function sets 3 to
  // misspelledOffset and 2 to misspelledLength, respectively.
  bool SpellCheckWord(const blink::WebString& text,
                      int* misspelled_offset,
                      int* misspelled_length);

  // Checks whether the specified text can be spell checked immediately using
  // the spell checker cache.
  bool HasInCache(const blink::WebString& text);

  // Checks whether the specified text is a misspelling comprised of more
  // than one word. If it is, append multiple results to the results vector.
  bool IsMultiWordMisspelling(
      const blink::WebString& text,
      std::vector<blink::WebTextCheckingResult>* results);

 private:
  // Initialize the internal resources if we need to initialize it.
  // Initializing this object may take long time. To prevent from hurting
  // the performance of test_shell, we initialize this object when
  // SpellCheckWord() is called for the first time.
  // To be compliant with SpellCheck:InitializeIfNeeded(), this function
  // returns true if this object is downloading a dictionary, otherwise
  // it returns false.
  bool InitializeIfNeeded();

  // A table that consists of misspelled words.
  std::vector<base::string16> misspelled_words_;

  // A flag representing whether or not this object is initialized.
  bool initialized_;

  DISALLOW_COPY_AND_ASSIGN(MockSpellCheck);
};

}  // namespace test_runner

#endif  // CONTENT_SHELL_TEST_RUNNER_MOCK_SPELL_CHECK_H_
