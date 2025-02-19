// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_TEST_RUNNER_TEXT_INPUT_CONTROLLER_H_
#define CONTENT_SHELL_TEST_RUNNER_TEXT_INPUT_CONTROLLER_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"

namespace blink {
class WebInputMethodController;
class WebLocalFrame;
class WebView;
}

namespace test_runner {

class WebViewTestProxyBase;

// TextInputController is bound to window.textInputController in Javascript
// when content_shell is running. Layout tests use it to exercise various
// corners of text input.
class TextInputController {
 public:
  explicit TextInputController(WebViewTestProxyBase* web_view_test_proxy_base);
  ~TextInputController();

  void Install(blink::WebLocalFrame* frame);

 private:
  friend class TextInputControllerBindings;

  void InsertText(const std::string& text);
  void UnmarkText();
  void DoCommand(const std::string& text);
  void SetMarkedText(const std::string& text, int start, int length);
  bool HasMarkedText();
  std::vector<int> MarkedRange();
  std::vector<int> SelectedRange();
  std::vector<int> FirstRectForCharacterRange(unsigned location,
                                              unsigned length);
  void SetComposition(const std::string& text);
  void ForceTextInputStateUpdate();

  blink::WebView* view();
  // Returns the WebInputMethodController corresponding to the focused frame
  // accepting IME. Could return nullptr if no such frame exists.
  blink::WebInputMethodController* GetInputMethodController();

  WebViewTestProxyBase* web_view_test_proxy_base_;

  base::WeakPtrFactory<TextInputController> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(TextInputController);
};

}  // namespace test_runner

#endif  // CONTENT_SHELL_TEST_RUNNER_TEXT_INPUT_CONTROLLER_H_
