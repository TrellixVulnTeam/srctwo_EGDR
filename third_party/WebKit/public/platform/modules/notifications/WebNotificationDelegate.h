// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebNotificationDelegate_h
#define WebNotificationDelegate_h

namespace blink {

// A delegate through which the embedder can trigger events on a Document-bound
// Web Notifications object. Service Worker-bound Web Notifications will not
// have a delegate, as their events will be fired on a Service Worker instead.
class WebNotificationDelegate {
 public:
  virtual void DispatchClickEvent() = 0;
  virtual void DispatchShowEvent() = 0;
  virtual void DispatchErrorEvent() = 0;
  virtual void DispatchCloseEvent() = 0;
};

}  // namespace blink

#endif  // WebNotificationDelegate_h
