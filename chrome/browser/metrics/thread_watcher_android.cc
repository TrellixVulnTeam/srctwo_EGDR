// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/metrics/thread_watcher_android.h"

#include "base/android/application_status_listener.h"
#include "base/command_line.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "chrome/browser/metrics/thread_watcher.h"

namespace {

// For most of the activities, the C++ side is initialized asynchronously
// and the very first APPLICATION_STATE_HAS_RUNNING_ACTIVITIES is never received
// whilst the ThreadWatcherList is initiated higher up in the stack.
// However, some activities are initialized synchronously, and it'll receive
// an APPLICATION_STATE_HAS_RUNNING_ACTIVITIES here as well.
// Protect against this case, and only let
// APPLICATION_STATE_HAS_RUNNING_ACTIVITIES turn on the
// watchdog if it was previously handled by an
// APPLICATION_STATE_HAS_STOPPED_ACTIVITIES (which is always handled here).
bool g_application_has_stopped = false;

void OnApplicationStateChange(
    base::android::ApplicationState application_state) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  if (application_state ==
      base::android::APPLICATION_STATE_HAS_STOPPED_ACTIVITIES) {
    g_application_has_stopped = true;
    ThreadWatcherList::StopWatchingAll();
  } else if (application_state ==
             base::android::APPLICATION_STATE_HAS_RUNNING_ACTIVITIES &&
             g_application_has_stopped) {
    g_application_has_stopped = false;
    ThreadWatcherList::StartWatchingAll(
        *base::CommandLine::ForCurrentProcess());
  }
}

struct LeakyApplicationStatusListenerTraits {
  static const bool kRegisterOnExit = false;
#if DCHECK_IS_ON()
  static const bool kAllowedToAccessOnNonjoinableThread = true;
#endif

  static base::android::ApplicationStatusListener* New(void* instance) {
    ANNOTATE_SCOPED_MEMORY_LEAK;
    return new (instance) base::android::ApplicationStatusListener(
            base::Bind(&OnApplicationStateChange));
  }

  static void Delete(base::android::ApplicationStatusListener* instance) {
  }
};

base::LazyInstance<base::android::ApplicationStatusListener,
                   LeakyApplicationStatusListenerTraits>
    g_application_status_listener = LAZY_INSTANCE_INITIALIZER;

}  // namespace

void ThreadWatcherAndroid::RegisterApplicationStatusListener() {
  // Leaky.
  g_application_status_listener.Get();
}
