// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/nacl/browser/bad_message.h"

#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "content/public/browser/browser_message_filter.h"

namespace nacl {
namespace bad_message {

void ReceivedBadMessage(content::BrowserMessageFilter* filter,
                        BadMessageReason reason) {
  LOG(ERROR) << "Terminating renderer for bad IPC message, reason " << reason;
  UMA_HISTOGRAM_SPARSE_SLOWLY("Stability.BadMessageTerminated.NaCl", reason);
  filter->ShutdownForBadMessage();
}

}  // namespace bad_message
}  // namespace nacl
