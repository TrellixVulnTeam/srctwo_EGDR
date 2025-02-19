// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/guest_view/mime_handler_view/test_mime_handler_view_guest.h"

#include "base/time/time.h"
#include "content/public/test/test_utils.h"

using guest_view::GuestViewBase;

namespace extensions {

TestMimeHandlerViewGuest::TestMimeHandlerViewGuest(
    content::WebContents* owner_web_contents)
    : MimeHandlerViewGuest(owner_web_contents),
      weak_ptr_factory_(this) {}

TestMimeHandlerViewGuest::~TestMimeHandlerViewGuest() {}

// static
GuestViewBase* TestMimeHandlerViewGuest::Create(
    content::WebContents* owner_web_contents) {
  return new TestMimeHandlerViewGuest(owner_web_contents);
}

// static
void TestMimeHandlerViewGuest::DelayNextCreateWebContents(int delay) {
  TestMimeHandlerViewGuest::delay_ = delay;
}

void TestMimeHandlerViewGuest::WaitForGuestAttached() {
  if (attached())
    return;
  created_message_loop_runner_ = new content::MessageLoopRunner;
  created_message_loop_runner_->Run();
}

void TestMimeHandlerViewGuest::CreateWebContents(
    const base::DictionaryValue& create_params,
    const WebContentsCreatedCallback& callback) {
  // Delay the creation of the guest's WebContents if |delay_| is set.
  if (delay_) {
    auto delta = base::TimeDelta::FromMilliseconds(
        delay_);
    std::unique_ptr<base::DictionaryValue> params(create_params.DeepCopy());
    content::BrowserThread::PostDelayedTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::Bind(&TestMimeHandlerViewGuest::CallBaseCreateWebContents,
                   weak_ptr_factory_.GetWeakPtr(),
                   base::Passed(&params),
                   callback),
        delta);

    // Reset the delay for the next creation.
    delay_ = 0;
    return;
  }

  MimeHandlerViewGuest::CreateWebContents(create_params, callback);
}

void TestMimeHandlerViewGuest::DidAttachToEmbedder() {
  MimeHandlerViewGuest::DidAttachToEmbedder();
  if (created_message_loop_runner_.get())
    created_message_loop_runner_->Quit();
}

void TestMimeHandlerViewGuest::CallBaseCreateWebContents(
    std::unique_ptr<base::DictionaryValue> create_params,
    const WebContentsCreatedCallback& callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  MimeHandlerViewGuest::CreateWebContents(*create_params, callback);
}

// static
int TestMimeHandlerViewGuest::delay_ = 0;

}  // namespace extensions
