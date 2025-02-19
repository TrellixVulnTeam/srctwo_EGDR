// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/edk/system/request_context.h"

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/threading/thread_local.h"

namespace mojo {
namespace edk {

namespace {

base::LazyInstance<base::ThreadLocalPointer<RequestContext>>::Leaky
    g_current_context = LAZY_INSTANCE_INITIALIZER;

}  // namespace

RequestContext::RequestContext() : RequestContext(Source::LOCAL_API_CALL) {}

RequestContext::RequestContext(Source source)
    : source_(source), tls_context_(g_current_context.Pointer()) {
  // We allow nested RequestContexts to exist as long as they aren't actually
  // used for anything.
  if (!tls_context_->Get())
    tls_context_->Set(this);
}

RequestContext::~RequestContext() {
  if (IsCurrent()) {
    // NOTE: Callbacks invoked by this destructor are allowed to initiate new
    // EDK requests on this thread, so we need to reset the thread-local context
    // pointer before calling them. We persist the original notification source
    // since we're starting over at the bottom of the stack.
    tls_context_->Set(nullptr);

    MojoWatcherNotificationFlags flags = MOJO_WATCHER_NOTIFICATION_FLAG_NONE;
    if (source_ == Source::SYSTEM)
      flags |= MOJO_WATCHER_NOTIFICATION_FLAG_FROM_SYSTEM;

    // We send all cancellation notifications first. This is necessary because
    // it's possible that cancelled watches have other pending notifications
    // attached to this RequestContext.
    //
    // From the application's perspective the watch is cancelled as soon as this
    // notification is received, and dispatching the cancellation notification
    // updates some internal Watch state to ensure no further notifications
    // fire. Because notifications on a single Watch are mutually exclusive,
    // this is sufficient to guarantee that MOJO_RESULT_CANCELLED is the last
    // notification received; which is the guarantee the API makes.
    for (const scoped_refptr<Watch>& watch :
         watch_cancel_finalizers_.container()) {
      static const HandleSignalsState closed_state = {0, 0};

      // Establish a new RequestContext to capture and run any new notifications
      // triggered by the callback invocation.
      RequestContext inner_context(source_);
      watch->InvokeCallback(MOJO_RESULT_CANCELLED, closed_state, flags);
    }

    for (const WatchNotifyFinalizer& watch :
         watch_notify_finalizers_.container()) {
      RequestContext inner_context(source_);
      watch.watch->InvokeCallback(watch.result, watch.state, flags);
    }
  } else {
    // It should be impossible for nested contexts to have finalizers.
    DCHECK(watch_notify_finalizers_.container().empty());
    DCHECK(watch_cancel_finalizers_.container().empty());
  }
}

// static
RequestContext* RequestContext::current() {
  DCHECK(g_current_context.Pointer()->Get());
  return g_current_context.Pointer()->Get();
}

void RequestContext::AddWatchNotifyFinalizer(scoped_refptr<Watch> watch,
                                             MojoResult result,
                                             const HandleSignalsState& state) {
  DCHECK(IsCurrent());
  watch_notify_finalizers_->push_back(
      WatchNotifyFinalizer(std::move(watch), result, state));
}

void RequestContext::AddWatchCancelFinalizer(scoped_refptr<Watch> watch) {
  DCHECK(IsCurrent());
  watch_cancel_finalizers_->push_back(std::move(watch));
}

bool RequestContext::IsCurrent() const {
  return tls_context_->Get() == this;
}

RequestContext::WatchNotifyFinalizer::WatchNotifyFinalizer(
    scoped_refptr<Watch> watch,
    MojoResult result,
    const HandleSignalsState& state)
    : watch(std::move(watch)), result(result), state(state) {}

RequestContext::WatchNotifyFinalizer::WatchNotifyFinalizer(
    const WatchNotifyFinalizer& other) = default;

RequestContext::WatchNotifyFinalizer::~WatchNotifyFinalizer() {}

}  // namespace edk
}  // namespace mojo
