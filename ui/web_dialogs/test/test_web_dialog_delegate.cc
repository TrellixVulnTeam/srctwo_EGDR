// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/web_dialogs/test/test_web_dialog_delegate.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

using content::WebContents;
using content::WebUIMessageHandler;

namespace ui {
namespace test {

TestWebDialogDelegate::TestWebDialogDelegate(const GURL& url)
    : url_(url), size_(400, 400), did_delete_(nullptr) {
}

TestWebDialogDelegate::~TestWebDialogDelegate() {
  if (did_delete_) {
    CHECK(!*did_delete_);
    *did_delete_ = true;
  }
}

void TestWebDialogDelegate::SetDeleteOnClosedAndObserve(
    bool* destroy_observer) {
  CHECK(destroy_observer);
  did_delete_ = destroy_observer;
}

ModalType TestWebDialogDelegate::GetDialogModalType() const {
  return MODAL_TYPE_WINDOW;
}

base::string16 TestWebDialogDelegate::GetDialogTitle() const {
  return base::UTF8ToUTF16("Test");
}

GURL TestWebDialogDelegate::GetDialogContentURL() const {
  return url_;
}

void TestWebDialogDelegate::GetWebUIMessageHandlers(
    std::vector<WebUIMessageHandler*>* handlers) const {
}

void TestWebDialogDelegate::GetDialogSize(gfx::Size* size) const {
  *size = size_;
}

std::string TestWebDialogDelegate::GetDialogArgs() const {
  return std::string();
}

void TestWebDialogDelegate::OnDialogClosed(const std::string& json_retval) {
  if (did_delete_)
    delete this;
}

void TestWebDialogDelegate::OnCloseContents(WebContents* source,
                                            bool* out_close_dialog) {
  *out_close_dialog = true;
}

bool TestWebDialogDelegate::ShouldShowDialogTitle() const {
  return true;
}

}  // namespace test
}  // namespace ui
