// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PushSubscriptionCallbacks_h
#define PushSubscriptionCallbacks_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Noncopyable.h"
#include "public/platform/modules/push_messaging/WebPushProvider.h"

namespace blink {

class ServiceWorkerRegistration;
class ScriptPromiseResolver;
struct WebPushError;
struct WebPushSubscription;

// This class is an implementation of WebPushSubscriptionCallbacks that will
// resolve the underlying promise depending on the result passed to the
// callback. It takes a ServiceWorkerRegistration in its constructor and will
// pass it to the PushSubscription.
class PushSubscriptionCallbacks final : public WebPushSubscriptionCallbacks {
  WTF_MAKE_NONCOPYABLE(PushSubscriptionCallbacks);
  USING_FAST_MALLOC(PushSubscriptionCallbacks);

 public:
  PushSubscriptionCallbacks(ScriptPromiseResolver*, ServiceWorkerRegistration*);
  ~PushSubscriptionCallbacks() override;

  // WebPushSubscriptionCallbacks interface.
  void OnSuccess(std::unique_ptr<WebPushSubscription>) override;
  void OnError(const WebPushError&) override;

 private:
  Persistent<ScriptPromiseResolver> resolver_;
  Persistent<ServiceWorkerRegistration> service_worker_registration_;
};

}  // namespace blink

#endif  // PushSubscriptionCallbacks_h
