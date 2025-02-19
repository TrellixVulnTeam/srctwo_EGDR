// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_BAD_MESSAGE_H_
#define CHROME_BROWSER_BAD_MESSAGE_H_

namespace content {
class BrowserMessageFilter;
class RenderProcessHost;
}

namespace bad_message {

// The browser process often chooses to terminate a renderer if it receives
// a bad IPC message. The reasons are tracked for metrics.
//
// See also content/browser/bad_message.h.
//
// NOTE: Do not remove or reorder elements in this list. Add new entries at the
// end. Items may be renamed but do not change the values. We rely on the enum
// values in histograms.
enum BadMessageReason {
  WRLHH_LOGGING_STOPPED_BAD_STATE = 0,
  PPH_EXTRA_PREVIEW_MESSAGE,

  // Please add new elements here. The naming convention is abbreviated class
  // name (e.g. RenderFrameHost becomes RFH) plus a unique description of the
  // reason. After making changes, you MUST update histograms.xml by running:
  // "python tools/metrics/histograms/update_bad_message_reasons.py"
  BAD_MESSAGE_MAX
};

// Called when the browser receives a bad IPC message from a renderer process on
// the UI thread. Logs the event, records a histogram metric for the |reason|,
// and terminates the process for |host|.
void ReceivedBadMessage(content::RenderProcessHost* host,
                        BadMessageReason reason);

// Called when a browser message filter receives a bad IPC message from a
// renderer or other child process. Logs the event, records a histogram metric
// for the |reason|, and terminates the process for |filter|.
void ReceivedBadMessage(content::BrowserMessageFilter* filter,
                        BadMessageReason reason);

}  // namespace bad_message

#endif  // CHROME_BROWSER_BAD_MESSAGE_H_
