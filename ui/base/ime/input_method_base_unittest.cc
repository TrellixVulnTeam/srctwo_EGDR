// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/ime/input_method_base.h"

#include <memory>

#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/scoped_observer.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/ime/dummy_text_input_client.h"
#include "ui/base/ime/input_method_observer.h"
#include "ui/events/event.h"

namespace ui {
namespace {

class ClientChangeVerifier {
 public:
  ClientChangeVerifier()
     : previous_client_(NULL),
       next_client_(NULL),
       call_expected_(false),
       on_will_change_focused_client_called_(false),
       on_did_change_focused_client_called_(false),
       on_text_input_state_changed_(false) {
  }

  // Expects that focused text input client will not be changed.
  void ExpectClientDoesNotChange() {
    previous_client_ = NULL;
    next_client_ = NULL;
    call_expected_ = false;
    on_will_change_focused_client_called_ = false;
    on_did_change_focused_client_called_ = false;
    on_text_input_state_changed_ = false;
  }

  // Expects that focused text input client will be changed from
  // |previous_client| to |next_client|.
  void ExpectClientChange(TextInputClient* previous_client,
                          TextInputClient* next_client) {
    previous_client_ = previous_client;
    next_client_ = next_client;
    call_expected_ = true;
    on_will_change_focused_client_called_ = false;
    on_did_change_focused_client_called_ = false;
    on_text_input_state_changed_ = false;
  }

  // Verifies the result satisfies the expectation or not.
  void Verify() {
    EXPECT_EQ(call_expected_, on_will_change_focused_client_called_);
    EXPECT_EQ(call_expected_, on_did_change_focused_client_called_);
    EXPECT_EQ(call_expected_, on_text_input_state_changed_);
  }

  void OnWillChangeFocusedClient(TextInputClient* focused_before,
                                 TextInputClient* focused) {
    EXPECT_TRUE(call_expected_);

    // Check arguments
    EXPECT_EQ(previous_client_, focused_before);
    EXPECT_EQ(next_client_, focused);

    // Check call order
    EXPECT_FALSE(on_will_change_focused_client_called_);
    EXPECT_FALSE(on_did_change_focused_client_called_);
    EXPECT_FALSE(on_text_input_state_changed_);

    on_will_change_focused_client_called_ = true;
  }

  void OnDidChangeFocusedClient(TextInputClient* focused_before,
                                TextInputClient* focused) {
    EXPECT_TRUE(call_expected_);

    // Check arguments
    EXPECT_EQ(previous_client_, focused_before);
    EXPECT_EQ(next_client_, focused);

    // Check call order
    EXPECT_TRUE(on_will_change_focused_client_called_);
    EXPECT_FALSE(on_did_change_focused_client_called_);
    EXPECT_FALSE(on_text_input_state_changed_);

    on_did_change_focused_client_called_ = true;
 }

  void OnTextInputStateChanged(const TextInputClient* client) {
    EXPECT_TRUE(call_expected_);

    // Check arguments
    EXPECT_EQ(next_client_, client);

    // Check call order
    EXPECT_TRUE(on_will_change_focused_client_called_);
    EXPECT_TRUE(on_did_change_focused_client_called_);
    EXPECT_FALSE(on_text_input_state_changed_);

    on_text_input_state_changed_ = true;
 }

 private:
  TextInputClient* previous_client_;
  TextInputClient* next_client_;
  bool call_expected_;
  bool on_will_change_focused_client_called_;
  bool on_did_change_focused_client_called_;
  bool on_text_input_state_changed_;

  DISALLOW_COPY_AND_ASSIGN(ClientChangeVerifier);
};

class InputMethodBaseTest : public testing::Test {
 protected:
  InputMethodBaseTest() {
  }
  ~InputMethodBaseTest() override {}

  void SetUp() override { message_loop_.reset(new base::MessageLoopForUI); }

  void TearDown() override { message_loop_.reset(); }

 private:
  std::unique_ptr<base::MessageLoop> message_loop_;
  DISALLOW_COPY_AND_ASSIGN(InputMethodBaseTest);
};

class MockInputMethodBase : public InputMethodBase {
 public:
  // Note: this class does not take the ownership of |verifier|.
  MockInputMethodBase(ClientChangeVerifier* verifier) : verifier_(verifier) {
  }
  ~MockInputMethodBase() override {}

 private:
  // Overriden from InputMethod.
  bool OnUntranslatedIMEMessage(
      const base::NativeEvent& event,
      InputMethod::NativeEventResult* result) override {
    return false;
  }
  ui::EventDispatchDetails DispatchKeyEvent(ui::KeyEvent*) override {
    return ui::EventDispatchDetails();
  }
  void OnCaretBoundsChanged(const TextInputClient* client) override {}
  void CancelComposition(const TextInputClient* client) override {}
  void OnInputLocaleChanged() override {}
  bool IsInputLocaleCJK() const override { return false; }
  bool IsCandidatePopupOpen() const override { return false; }
  // Overriden from InputMethodBase.
  void OnWillChangeFocusedClient(TextInputClient* focused_before,
                                 TextInputClient* focused) override {
    verifier_->OnWillChangeFocusedClient(focused_before, focused);
  }

  void OnDidChangeFocusedClient(TextInputClient* focused_before,
                                TextInputClient* focused) override {
    verifier_->OnDidChangeFocusedClient(focused_before, focused);
  }

  ClientChangeVerifier* verifier_;

  FRIEND_TEST_ALL_PREFIXES(InputMethodBaseTest, CandidateWindowEvents);
  DISALLOW_COPY_AND_ASSIGN(MockInputMethodBase);
};

class MockInputMethodObserver : public InputMethodObserver {
 public:
  // Note: this class does not take the ownership of |verifier|.
  explicit MockInputMethodObserver(ClientChangeVerifier* verifier)
      : verifier_(verifier) {
  }
  ~MockInputMethodObserver() override {}

 private:
  void OnFocus() override {}
  void OnBlur() override {}
  void OnCaretBoundsChanged(const TextInputClient* client) override {}
  void OnTextInputStateChanged(const TextInputClient* client) override {
    verifier_->OnTextInputStateChanged(client);
  }
  void OnShowImeIfNeeded() override {}
  void OnInputMethodDestroyed(const InputMethod* client) override {}

  ClientChangeVerifier* verifier_;
  DISALLOW_COPY_AND_ASSIGN(MockInputMethodObserver);
};

typedef ScopedObserver<InputMethod, InputMethodObserver>
    InputMethodScopedObserver;

void SetFocusedTextInputClient(InputMethod* input_method,
                               TextInputClient* text_input_client) {
  input_method->SetFocusedTextInputClient(text_input_client);
}

TEST_F(InputMethodBaseTest, SetFocusedTextInputClient) {
  DummyTextInputClient text_input_client_1st;
  DummyTextInputClient text_input_client_2nd;

  ClientChangeVerifier verifier;
  MockInputMethodBase input_method(&verifier);
  MockInputMethodObserver input_method_observer(&verifier);
  InputMethodScopedObserver scoped_observer(&input_method_observer);
  scoped_observer.Add(&input_method);

  // Assume that the top-level-widget gains focus.
  input_method.OnFocus();

  {
    SCOPED_TRACE("Focus from NULL to 1st TextInputClient");

    ASSERT_EQ(NULL, input_method.GetTextInputClient());
    verifier.ExpectClientChange(NULL, &text_input_client_1st);
    SetFocusedTextInputClient(&input_method, &text_input_client_1st);
    EXPECT_EQ(&text_input_client_1st, input_method.GetTextInputClient());
    verifier.Verify();
  }

  {
    SCOPED_TRACE("Redundant focus events must be ignored");
    verifier.ExpectClientDoesNotChange();
    SetFocusedTextInputClient(&input_method, &text_input_client_1st);
    verifier.Verify();
  }

  {
    SCOPED_TRACE("Focus from 1st to 2nd TextInputClient");

    ASSERT_EQ(&text_input_client_1st, input_method.GetTextInputClient());
    verifier.ExpectClientChange(&text_input_client_1st,
                                &text_input_client_2nd);
    SetFocusedTextInputClient(&input_method, &text_input_client_2nd);
    EXPECT_EQ(&text_input_client_2nd, input_method.GetTextInputClient());
    verifier.Verify();
  }

  {
    SCOPED_TRACE("Focus from 2nd TextInputClient to NULL");

    ASSERT_EQ(&text_input_client_2nd, input_method.GetTextInputClient());
    verifier.ExpectClientChange(&text_input_client_2nd, NULL);
    SetFocusedTextInputClient(&input_method, NULL);
    EXPECT_EQ(NULL, input_method.GetTextInputClient());
    verifier.Verify();
  }

  {
    SCOPED_TRACE("Redundant focus events must be ignored");
    verifier.ExpectClientDoesNotChange();
    SetFocusedTextInputClient(&input_method, NULL);
    verifier.Verify();
  }
}

TEST_F(InputMethodBaseTest, DetachTextInputClient) {
  DummyTextInputClient text_input_client;
  DummyTextInputClient text_input_client_the_other;

  ClientChangeVerifier verifier;
  MockInputMethodBase input_method(&verifier);
  MockInputMethodObserver input_method_observer(&verifier);
  InputMethodScopedObserver scoped_observer(&input_method_observer);
  scoped_observer.Add(&input_method);

  // Assume that the top-level-widget gains focus.
  input_method.OnFocus();

  // Initialize for the next test.
  {
    verifier.ExpectClientChange(NULL, &text_input_client);
    input_method.SetFocusedTextInputClient(&text_input_client);
    verifier.Verify();
  }

  {
    SCOPED_TRACE("DetachTextInputClient must be ignored for other clients");
    ASSERT_EQ(&text_input_client, input_method.GetTextInputClient());
    verifier.ExpectClientDoesNotChange();
    input_method.DetachTextInputClient(&text_input_client_the_other);
    EXPECT_EQ(&text_input_client, input_method.GetTextInputClient());
    verifier.Verify();
  }

  {
    SCOPED_TRACE("DetachTextInputClient must succeed even after the "
                 "top-level loses the focus");

    ASSERT_EQ(&text_input_client, input_method.GetTextInputClient());
    input_method.OnBlur();
    input_method.OnFocus();
    verifier.ExpectClientChange(&text_input_client, NULL);
    input_method.DetachTextInputClient(&text_input_client);
    EXPECT_EQ(NULL, input_method.GetTextInputClient());
    verifier.Verify();
  }
}

}  // namespace
}  // namespace ui
