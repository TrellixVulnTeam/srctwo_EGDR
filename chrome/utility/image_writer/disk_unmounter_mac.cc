// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/utility/image_writer/disk_unmounter_mac.h"

#include <IOKit/storage/IOStorageProtocolCharacteristics.h>
#include <sys/socket.h>

#include "base/message_loop/message_pump_mac.h"
#include "base/posix/eintr_wrapper.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "chrome/utility/image_writer/error_messages.h"
#include "chrome/utility/image_writer/image_writer.h"

namespace image_writer {

DiskUnmounterMac::DiskUnmounterMac() : cf_thread_("ImageWriterDiskArb") {
  base::Thread::Options options;
  options.message_pump_factory = base::Bind(&CreateMessagePump);

  cf_thread_.StartWithOptions(options);
}

DiskUnmounterMac::~DiskUnmounterMac() {
  if (disk_)
    DADiskUnclaim(disk_);
}

void DiskUnmounterMac::Unmount(const std::string& device_path,
                               const base::Closure& success_continuation,
                               const base::Closure& failure_continuation) {
  // Should only be used once.
  DCHECK(!original_thread_.get());
  original_thread_ = base::ThreadTaskRunnerHandle::Get();
  success_continuation_ = success_continuation;
  failure_continuation_ = failure_continuation;

  cf_thread_.task_runner()->PostTask(
      FROM_HERE, base::Bind(&DiskUnmounterMac::UnmountOnWorker,
                            base::Unretained(this), device_path));
}

// static
void DiskUnmounterMac::DiskClaimed(DADiskRef disk,
                                   DADissenterRef dissenter,
                                   void* context) {
  DiskUnmounterMac* disk_unmounter = static_cast<DiskUnmounterMac*>(context);

  if (dissenter) {
    LOG(ERROR) << "Unable to claim disk.";
    disk_unmounter->Error();
    return;
  }

  DADiskUnmount(disk,
                kDADiskUnmountOptionForce | kDADiskUnmountOptionWhole,
                DiskUnmounted,
                disk_unmounter);
}

// static
DADissenterRef DiskUnmounterMac::DiskClaimRevoked(DADiskRef disk,
                                                  void* context) {
  CFStringRef reason = CFSTR(
      "Hi. Sorry to bother you, but I'm busy overwriting the entire disk "
      "here. There's nothing to claim but the smoldering ruins of bytes "
      "that were in flash memory. Trust me, it's nothing that you want. "
      "All the best. Toodles!");
  return DADissenterCreate(kCFAllocatorDefault, kDAReturnBusy, reason);
}

// static
void DiskUnmounterMac::DiskUnmounted(DADiskRef disk,
                                     DADissenterRef dissenter,
                                     void* context) {
  DiskUnmounterMac* disk_unmounter = static_cast<DiskUnmounterMac*>(context);

  if (dissenter) {
    LOG(ERROR) << "Unable to unmount disk.";
    disk_unmounter->Error();
    return;
  }

  disk_unmounter->original_thread_->PostTask(
      FROM_HERE, disk_unmounter->success_continuation_);
}

// static
std::unique_ptr<base::MessagePump> DiskUnmounterMac::CreateMessagePump() {
  return std::unique_ptr<base::MessagePump>(new base::MessagePumpCFRunLoop);
}

void DiskUnmounterMac::UnmountOnWorker(const std::string& device_path) {
  DCHECK(cf_thread_.task_runner()->BelongsToCurrentThread());

  session_.reset(DASessionCreate(NULL));

  DASessionScheduleWithRunLoop(
      session_, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

  disk_.reset(DADiskCreateFromBSDName(
      kCFAllocatorDefault, session_, device_path.c_str()));

  if (!disk_) {
    LOG(ERROR) << "Unable to get disk reference.";
    Error();
    return;
  }

  DADiskClaim(disk_,
              kDADiskClaimOptionDefault,
              DiskClaimRevoked,
              this,
              DiskClaimed,
              this);
}

void DiskUnmounterMac::Error() {
  original_thread_->PostTask(FROM_HERE, failure_continuation_);
}

}  // namespace image_writer
