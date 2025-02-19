// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/renderer_host/text_input_client_message_filter.h"

#include "base/strings/string16.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/text_input_client_mac.h"
#include "content/common/text_input_client_messages.h"
#include "content/public/browser/render_widget_host_view.h"
#include "ipc/ipc_message_macros.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/range/range.h"

namespace content {

TextInputClientMessageFilter::TextInputClientMessageFilter()
    : BrowserMessageFilter(TextInputClientMsgStart) {}

bool TextInputClientMessageFilter::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(TextInputClientMessageFilter, message)
    IPC_MESSAGE_HANDLER(TextInputClientReplyMsg_GotStringAtPoint,
                        OnGotStringAtPoint)
    IPC_MESSAGE_HANDLER(TextInputClientReplyMsg_GotCharacterIndexForPoint,
                        OnGotCharacterIndexForPoint)
    IPC_MESSAGE_HANDLER(TextInputClientReplyMsg_GotFirstRectForRange,
                        OnGotFirstRectForRange)
    IPC_MESSAGE_HANDLER(TextInputClientReplyMsg_GotStringForRange,
                        OnGotStringFromRange)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void TextInputClientMessageFilter::OverrideThreadForMessage(
    const IPC::Message& message,
    BrowserThread::ID* thread) {
  switch (message.type()) {
    case TextInputClientReplyMsg_GotStringAtPoint::ID:
    case TextInputClientReplyMsg_GotStringForRange::ID:
      *thread = BrowserThread::UI;
      break;
  }
}

TextInputClientMessageFilter::~TextInputClientMessageFilter() {}

void TextInputClientMessageFilter::OnGotStringAtPoint(
    const mac::AttributedStringCoder::EncodedString& encoded_string,
    const gfx::Point& point) {
  TextInputClientMac* service = TextInputClientMac::GetInstance();
  NSAttributedString* string =
      mac::AttributedStringCoder::Decode(&encoded_string);
  service->GetStringAtPointReply(string, NSPointFromCGPoint(point.ToCGPoint()));
}

void TextInputClientMessageFilter::OnGotCharacterIndexForPoint(uint32_t index) {
  TextInputClientMac* service = TextInputClientMac::GetInstance();
  // |index| could be WTF::notFound (-1) and its value is different from
  // NSNotFound so we need to convert it.
  size_t char_index;
  if (index == UINT32_MAX) {
    char_index = NSNotFound;
  } else {
    char_index = index;
  }
  service->SetCharacterIndexAndSignal(char_index);
}

void TextInputClientMessageFilter::OnGotFirstRectForRange(
    const gfx::Rect& rect) {
  TextInputClientMac* service = TextInputClientMac::GetInstance();
  service->SetFirstRectAndSignal(NSRectFromCGRect(rect.ToCGRect()));
}

void TextInputClientMessageFilter::OnGotStringFromRange(
    const mac::AttributedStringCoder::EncodedString& encoded_string,
    const gfx::Point& point) {
  TextInputClientMac* service = TextInputClientMac::GetInstance();
  NSAttributedString* string =
      mac::AttributedStringCoder::Decode(&encoded_string);
  service->GetStringFromRangeReply(
      string, NSPointFromCGPoint(point.ToCGPoint()));
}

}  // namespace content
