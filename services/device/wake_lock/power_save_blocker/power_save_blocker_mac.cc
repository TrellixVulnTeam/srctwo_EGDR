// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/wake_lock/power_save_blocker/power_save_blocker.h"

#include <IOKit/pwr_mgt/IOPMLib.h>

#include "base/bind.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/mac/scoped_cftyperef.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/sys_string_conversions.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread.h"

namespace device {
namespace {

// Power management cannot be done on the UI thread. IOPMAssertionCreate does a
// synchronous MIG call to configd, so if it is called on the main thread the UI
// is at the mercy of another process. See http://crbug.com/79559 and
// http://www.opensource.apple.com/source/IOKitUser/IOKitUser-514.16.31/pwr_mgt.subproj/IOPMLibPrivate.c
// .
struct PowerSaveBlockerLazyInstanceTraits {
  static const bool kRegisterOnExit = false;
#if DCHECK_IS_ON()
  static const bool kAllowedToAccessOnNonjoinableThread = true;
#endif

  static base::Thread* New(void* instance) {
    base::Thread* thread = new (instance) base::Thread("PowerSaveBlocker");
    thread->Start();
    return thread;
  }
  static void Delete(base::Thread* instance) {}
};
base::LazyInstance<base::Thread, PowerSaveBlockerLazyInstanceTraits>
    g_power_thread = LAZY_INSTANCE_INITIALIZER;

}  // namespace

class PowerSaveBlocker::Delegate
    : public base::RefCountedThreadSafe<PowerSaveBlocker::Delegate> {
 public:
  Delegate(PowerSaveBlockerType type, const std::string& description)
      : type_(type),
        description_(description),
        assertion_(kIOPMNullAssertionID) {}

  // Does the actual work to apply or remove the desired power save block.
  void ApplyBlock();
  void RemoveBlock();

 private:
  friend class base::RefCountedThreadSafe<Delegate>;
  ~Delegate() {}
  PowerSaveBlockerType type_;
  std::string description_;
  IOPMAssertionID assertion_;
};

void PowerSaveBlocker::Delegate::ApplyBlock() {
  DCHECK_EQ(base::PlatformThread::CurrentId(),
            g_power_thread.Pointer()->GetThreadId());

  CFStringRef level = NULL;
  // See QA1340 <http://developer.apple.com/library/mac/#qa/qa1340/> for more
  // details.
  switch (type_) {
    case PowerSaveBlocker::kPowerSaveBlockPreventAppSuspension:
      level = kIOPMAssertionTypeNoIdleSleep;
      break;
    case PowerSaveBlocker::kPowerSaveBlockPreventDisplaySleep:
      level = kIOPMAssertionTypeNoDisplaySleep;
      break;
    default:
      NOTREACHED();
      break;
  }
  if (level) {
    base::ScopedCFTypeRef<CFStringRef> cf_description(
        base::SysUTF8ToCFStringRef(description_));
    IOReturn result = IOPMAssertionCreateWithName(level, kIOPMAssertionLevelOn,
                                                  cf_description, &assertion_);
    LOG_IF(ERROR, result != kIOReturnSuccess)
        << "IOPMAssertionCreate: " << result;
  }
}

void PowerSaveBlocker::Delegate::RemoveBlock() {
  DCHECK_EQ(base::PlatformThread::CurrentId(),
            g_power_thread.Pointer()->GetThreadId());

  if (assertion_ != kIOPMNullAssertionID) {
    IOReturn result = IOPMAssertionRelease(assertion_);
    LOG_IF(ERROR, result != kIOReturnSuccess)
        << "IOPMAssertionRelease: " << result;
  }
}

PowerSaveBlocker::PowerSaveBlocker(
    PowerSaveBlockerType type,
    Reason reason,
    const std::string& description,
    scoped_refptr<base::SequencedTaskRunner> ui_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> blocking_task_runner)
    : delegate_(new Delegate(type, description)),
      ui_task_runner_(ui_task_runner),
      blocking_task_runner_(blocking_task_runner) {
  g_power_thread.Pointer()->task_runner()->PostTask(
      FROM_HERE, base::Bind(&Delegate::ApplyBlock, delegate_));
}

PowerSaveBlocker::~PowerSaveBlocker() {
  g_power_thread.Pointer()->task_runner()->PostTask(
      FROM_HERE, base::Bind(&Delegate::RemoveBlock, delegate_));
}

}  // namespace device
