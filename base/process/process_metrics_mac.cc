// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/process/process_metrics.h"

#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/shared_region.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/sysctl.h>

#include "base/containers/hash_tables.h"
#include "base/logging.h"
#include "base/mac/mac_util.h"
#include "base/mac/mach_logging.h"
#include "base/mac/scoped_mach_port.h"
#include "base/memory/ptr_util.h"
#include "base/numerics/safe_conversions.h"
#include "base/numerics/safe_math.h"
#include "base/sys_info.h"

namespace base {

namespace {

#if !defined(MAC_OS_X_VERSION_10_11) || \
    MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_11
// The |phys_footprint| field was introduced in 10.11.
struct ChromeTaskVMInfo {
  mach_vm_size_t virtual_size;
  integer_t region_count;
  integer_t page_size;
  mach_vm_size_t resident_size;
  mach_vm_size_t resident_size_peak;
  mach_vm_size_t device;
  mach_vm_size_t device_peak;
  mach_vm_size_t internal;
  mach_vm_size_t internal_peak;
  mach_vm_size_t external;
  mach_vm_size_t external_peak;
  mach_vm_size_t reusable;
  mach_vm_size_t reusable_peak;
  mach_vm_size_t purgeable_volatile_pmap;
  mach_vm_size_t purgeable_volatile_resident;
  mach_vm_size_t purgeable_volatile_virtual;
  mach_vm_size_t compressed;
  mach_vm_size_t compressed_peak;
  mach_vm_size_t compressed_lifetime;
  mach_vm_size_t phys_footprint;
};
#else
using ChromeTaskVMInfo = task_vm_info;
#endif  // MAC_OS_X_VERSION_10_11
mach_msg_type_number_t ChromeTaskVMInfoCount =
    sizeof(ChromeTaskVMInfo) / sizeof(natural_t);

bool GetTaskInfo(mach_port_t task, task_basic_info_64* task_info_data) {
  if (task == MACH_PORT_NULL)
    return false;
  mach_msg_type_number_t count = TASK_BASIC_INFO_64_COUNT;
  kern_return_t kr = task_info(task,
                               TASK_BASIC_INFO_64,
                               reinterpret_cast<task_info_t>(task_info_data),
                               &count);
  // Most likely cause for failure: |task| is a zombie.
  return kr == KERN_SUCCESS;
}

bool GetCPUType(cpu_type_t* cpu_type) {
  size_t len = sizeof(*cpu_type);
  int result = sysctlbyname("sysctl.proc_cputype",
                            cpu_type,
                            &len,
                            NULL,
                            0);
  if (result != 0) {
    DPLOG(ERROR) << "sysctlbyname(""sysctl.proc_cputype"")";
    return false;
  }

  return true;
}

bool IsAddressInSharedRegion(mach_vm_address_t addr, cpu_type_t type) {
  if (type == CPU_TYPE_I386) {
    return addr >= SHARED_REGION_BASE_I386 &&
           addr < (SHARED_REGION_BASE_I386 + SHARED_REGION_SIZE_I386);
  } else if (type == CPU_TYPE_X86_64) {
    return addr >= SHARED_REGION_BASE_X86_64 &&
           addr < (SHARED_REGION_BASE_X86_64 + SHARED_REGION_SIZE_X86_64);
  } else {
    return false;
  }
}

MachVMRegionResult ParseOutputFromMachVMRegion(kern_return_t kr) {
  if (kr == KERN_INVALID_ADDRESS) {
    // We're at the end of the address space.
    return MachVMRegionResult::Finished;
  } else if (kr != KERN_SUCCESS) {
    return MachVMRegionResult::Error;
  }
  return MachVMRegionResult::Success;
}

}  // namespace

// Getting a mach task from a pid for another process requires permissions in
// general, so there doesn't really seem to be a way to do these (and spinning
// up ps to fetch each stats seems dangerous to put in a base api for anyone to
// call). Child processes ipc their port, so return something if available,
// otherwise return 0.

// static
std::unique_ptr<ProcessMetrics> ProcessMetrics::CreateProcessMetrics(
    ProcessHandle process,
    PortProvider* port_provider) {
  return WrapUnique(new ProcessMetrics(process, port_provider));
}

size_t ProcessMetrics::GetPagefileUsage() const {
  task_basic_info_64 task_info_data;
  if (!GetTaskInfo(TaskForPid(process_), &task_info_data))
    return 0;
  return task_info_data.virtual_size;
}

size_t ProcessMetrics::GetPeakPagefileUsage() const {
  return 0;
}

size_t ProcessMetrics::GetWorkingSetSize() const {
  size_t resident_bytes = 0;
  if (!GetMemoryBytes(nullptr, nullptr, &resident_bytes, nullptr))
    return 0;
  return resident_bytes;
}

size_t ProcessMetrics::GetPeakWorkingSetSize() const {
  return 0;
}

bool ProcessMetrics::GetMemoryBytes(size_t* private_bytes,
                                    size_t* shared_bytes) const {
  return GetMemoryBytes(private_bytes, shared_bytes, nullptr, nullptr);
}

// This is a rough approximation of the algorithm that libtop uses.
// private_bytes is the size of private resident memory.
// shared_bytes is the size of shared resident memory.
bool ProcessMetrics::GetMemoryBytes(size_t* private_bytes,
                                    size_t* shared_bytes,
                                    size_t* resident_bytes,
                                    size_t* locked_bytes) const {
  size_t private_pages_count = 0;
  size_t shared_pages_count = 0;
  size_t wired_pages_count = 0;

  mach_port_t task = TaskForPid(process_);
  if (task == MACH_PORT_NULL) {
    DLOG(ERROR) << "Invalid process";
    return false;
  }

  cpu_type_t cpu_type;
  if (!GetCPUType(&cpu_type))
    return false;

  // The same region can be referenced multiple times. To avoid double counting
  // we need to keep track of which regions we've already counted.
  hash_set<int> seen_objects;

  // We iterate through each VM region in the task's address map. For shared
  // memory we add up all the pages that are marked as shared. Like libtop we
  // try to avoid counting pages that are also referenced by other tasks. Since
  // we don't have access to the VM regions of other tasks the only hint we have
  // is if the address is in the shared region area.
  //
  // Private memory is much simpler. We simply count the pages that are marked
  // as private or copy on write (COW).
  //
  // See libtop_update_vm_regions in
  // http://www.opensource.apple.com/source/top/top-67/libtop.c
  mach_vm_size_t size = 0;
  mach_vm_address_t address = MACH_VM_MIN_ADDRESS;
  while (true) {
    base::CheckedNumeric<mach_vm_address_t> next_address(address);
    next_address += size;
    if (!next_address.IsValid())
      return false;
    address = next_address.ValueOrDie();

    mach_vm_address_t address_copy = address;
    vm_region_top_info_data_t info;
    MachVMRegionResult result = GetTopInfo(task, &size, &address, &info);
    if (result == MachVMRegionResult::Error)
      return false;
    if (result == MachVMRegionResult::Finished)
      break;

    vm_region_basic_info_64 basic_info;
    mach_vm_size_t dummy_size = 0;
    result = GetBasicInfo(task, &dummy_size, &address_copy, &basic_info);
    if (result == MachVMRegionResult::Error)
      return false;
    if (result == MachVMRegionResult::Finished)
      break;

    bool is_wired = basic_info.user_wired_count > 0;

    if (IsAddressInSharedRegion(address, cpu_type) &&
        info.share_mode != SM_PRIVATE)
      continue;

    if (info.share_mode == SM_COW && info.ref_count == 1)
      info.share_mode = SM_PRIVATE;

    switch (info.share_mode) {
      case SM_LARGE_PAGE:
      case SM_PRIVATE:
        private_pages_count += info.private_pages_resident;
        private_pages_count += info.shared_pages_resident;
        break;
      case SM_COW:
        private_pages_count += info.private_pages_resident;
        // Fall through
      case SM_SHARED:
      case SM_PRIVATE_ALIASED:
      case SM_TRUESHARED:
      case SM_SHARED_ALIASED:
        if (seen_objects.count(info.obj_id) == 0) {
          // Only count the first reference to this region.
          seen_objects.insert(info.obj_id);
          shared_pages_count += info.shared_pages_resident;
        }
        break;
      default:
        break;
    }
    if (is_wired) {
      wired_pages_count +=
          info.private_pages_resident + info.shared_pages_resident;
    }
  }

  if (private_bytes)
    *private_bytes = private_pages_count * PAGE_SIZE;
  if (shared_bytes)
    *shared_bytes = shared_pages_count * PAGE_SIZE;
  if (resident_bytes)
    *resident_bytes = (private_pages_count + shared_pages_count) * PAGE_SIZE;
  if (locked_bytes)
    *locked_bytes = wired_pages_count * PAGE_SIZE;

  return true;
}

void ProcessMetrics::GetCommittedKBytes(CommittedKBytes* usage) const {
  WorkingSetKBytes unused;
  if (!GetCommittedAndWorkingSetKBytes(usage, &unused)) {
    *usage = CommittedKBytes();
  }
}

bool ProcessMetrics::GetWorkingSetKBytes(WorkingSetKBytes* ws_usage) const {
  CommittedKBytes unused;
  return GetCommittedAndWorkingSetKBytes(&unused, ws_usage);
}

bool ProcessMetrics::GetCommittedAndWorkingSetKBytes(
    CommittedKBytes* usage,
    WorkingSetKBytes* ws_usage) const {
  task_basic_info_64 task_info_data;
  if (!GetTaskInfo(TaskForPid(process_), &task_info_data))
    return false;

  usage->priv = task_info_data.virtual_size / 1024;
  usage->mapped = 0;
  usage->image = 0;

  ws_usage->priv = task_info_data.resident_size / 1024;
  ws_usage->shareable = 0;
  ws_usage->shared = 0;

  return true;
}

ProcessMetrics::TaskVMInfo ProcessMetrics::GetTaskVMInfo() const {
  TaskVMInfo info;
  ChromeTaskVMInfo task_vm_info;
  mach_msg_type_number_t count = ChromeTaskVMInfoCount;
  kern_return_t result =
      task_info(TaskForPid(process_), TASK_VM_INFO,
                reinterpret_cast<task_info_t>(&task_vm_info), &count);
  if (result != KERN_SUCCESS)
    return info;

  info.internal = task_vm_info.internal;
  info.compressed = task_vm_info.compressed;
  if (count == ChromeTaskVMInfoCount)
    info.phys_footprint = task_vm_info.phys_footprint;
  return info;
}

#define TIME_VALUE_TO_TIMEVAL(a, r) do {  \
  (r)->tv_sec = (a)->seconds;             \
  (r)->tv_usec = (a)->microseconds;       \
} while (0)

double ProcessMetrics::GetCPUUsage() {
  mach_port_t task = TaskForPid(process_);
  if (task == MACH_PORT_NULL)
    return 0;

  // Libtop explicitly loops over the threads (libtop_pinfo_update_cpu_usage()
  // in libtop.c), but this is more concise and gives the same results:
  task_thread_times_info thread_info_data;
  mach_msg_type_number_t thread_info_count = TASK_THREAD_TIMES_INFO_COUNT;
  kern_return_t kr = task_info(task,
                               TASK_THREAD_TIMES_INFO,
                               reinterpret_cast<task_info_t>(&thread_info_data),
                               &thread_info_count);
  if (kr != KERN_SUCCESS) {
    // Most likely cause: |task| is a zombie.
    return 0;
  }

  task_basic_info_64 task_info_data;
  if (!GetTaskInfo(task, &task_info_data))
    return 0;

  /* Set total_time. */
  // thread info contains live time...
  struct timeval user_timeval, system_timeval, task_timeval;
  TIME_VALUE_TO_TIMEVAL(&thread_info_data.user_time, &user_timeval);
  TIME_VALUE_TO_TIMEVAL(&thread_info_data.system_time, &system_timeval);
  timeradd(&user_timeval, &system_timeval, &task_timeval);

  // ... task info contains terminated time.
  TIME_VALUE_TO_TIMEVAL(&task_info_data.user_time, &user_timeval);
  TIME_VALUE_TO_TIMEVAL(&task_info_data.system_time, &system_timeval);
  timeradd(&user_timeval, &task_timeval, &task_timeval);
  timeradd(&system_timeval, &task_timeval, &task_timeval);

  TimeTicks time = TimeTicks::Now();
  int64_t task_time = TimeValToMicroseconds(task_timeval);

  if (last_system_time_ == 0) {
    // First call, just set the last values.
    last_cpu_time_ = time;
    last_system_time_ = task_time;
    return 0;
  }

  int64_t system_time_delta = task_time - last_system_time_;
  int64_t time_delta = (time - last_cpu_time_).InMicroseconds();
  DCHECK_NE(0U, time_delta);
  if (time_delta == 0)
    return 0;

  last_cpu_time_ = time;
  last_system_time_ = task_time;

  return static_cast<double>(system_time_delta * 100.0) / time_delta;
}

int ProcessMetrics::GetIdleWakeupsPerSecond() {
  mach_port_t task = TaskForPid(process_);
  if (task == MACH_PORT_NULL)
    return 0;

  task_power_info power_info_data;
  mach_msg_type_number_t power_info_count = TASK_POWER_INFO_COUNT;
  kern_return_t kr = task_info(task,
                               TASK_POWER_INFO,
                               reinterpret_cast<task_info_t>(&power_info_data),
                               &power_info_count);
  if (kr != KERN_SUCCESS) {
    // Most likely cause: |task| is a zombie, or this is on a pre-10.8.4 system
    // where TASK_POWER_INFO isn't supported yet.
    return 0;
  }

  // The task_power_info struct contains two wakeup counters:
  // task_interrupt_wakeups and task_platform_idle_wakeups.
  // task_interrupt_wakeups is the total number of wakeups generated by the
  // process, and is the number that Activity Monitor reports.
  // task_platform_idle_wakeups is a subset of task_interrupt_wakeups that
  // tallies the number of times the processor was taken out of its low-power
  // idle state to handle a wakeup. task_platform_idle_wakeups therefore result
  // in a greater power increase than the other interrupts which occur while the
  // CPU is already working, and reducing them has a greater overall impact on
  // power usage. See the powermetrics man page for more info.
  return CalculateIdleWakeupsPerSecond(
      power_info_data.task_platform_idle_wakeups);
}

bool ProcessMetrics::GetIOCounters(IoCounters* io_counters) const {
  return false;
}

ProcessMetrics::ProcessMetrics(ProcessHandle process,
                               PortProvider* port_provider)
    : process_(process),
      last_system_time_(0),
      last_absolute_idle_wakeups_(0),
      port_provider_(port_provider) {
  processor_count_ = SysInfo::NumberOfProcessors();
}

mach_port_t ProcessMetrics::TaskForPid(ProcessHandle process) const {
  mach_port_t task = MACH_PORT_NULL;
  if (port_provider_)
    task = port_provider_->TaskForPid(process_);
  if (task == MACH_PORT_NULL && process_ == getpid())
    task = mach_task_self();
  return task;
}

// Bytes committed by the system.
size_t GetSystemCommitCharge() {
  base::mac::ScopedMachSendRight host(mach_host_self());
  mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
  vm_statistics_data_t data;
  kern_return_t kr = host_statistics(host.get(), HOST_VM_INFO,
                                     reinterpret_cast<host_info_t>(&data),
                                     &count);
  if (kr != KERN_SUCCESS) {
    MACH_DLOG(WARNING, kr) << "host_statistics";
    return 0;
  }

  return (data.active_count * PAGE_SIZE) / 1024;
}

bool GetSystemMemoryInfo(SystemMemoryInfoKB* meminfo) {
  struct host_basic_info hostinfo;
  mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
  base::mac::ScopedMachSendRight host(mach_host_self());
  int result = host_info(host.get(), HOST_BASIC_INFO,
                         reinterpret_cast<host_info_t>(&hostinfo), &count);
  if (result != KERN_SUCCESS)
    return false;

  DCHECK_EQ(HOST_BASIC_INFO_COUNT, count);
  meminfo->total = static_cast<int>(hostinfo.max_mem / 1024);

  vm_statistics64_data_t vm_info;
  count = HOST_VM_INFO64_COUNT;

  if (host_statistics64(host.get(), HOST_VM_INFO64,
                        reinterpret_cast<host_info64_t>(&vm_info),
                        &count) != KERN_SUCCESS) {
    return false;
  }
  DCHECK_EQ(HOST_VM_INFO64_COUNT, count);

  static_assert(PAGE_SIZE % 1024 == 0, "Invalid page size");
  meminfo->free = saturated_cast<int>(
      PAGE_SIZE / 1024 * (vm_info.free_count - vm_info.speculative_count));
  meminfo->speculative =
      saturated_cast<int>(PAGE_SIZE / 1024 * vm_info.speculative_count);
  meminfo->file_backed =
      saturated_cast<int>(PAGE_SIZE / 1024 * vm_info.external_page_count);
  meminfo->purgeable =
      saturated_cast<int>(PAGE_SIZE / 1024 * vm_info.purgeable_count);

  return true;
}

// Both |size| and |address| are in-out parameters.
// |info| is an output parameter, only valid on Success.
MachVMRegionResult GetTopInfo(mach_port_t task,
                              mach_vm_size_t* size,
                              mach_vm_address_t* address,
                              vm_region_top_info_data_t* info) {
  mach_msg_type_number_t info_count = VM_REGION_TOP_INFO_COUNT;
  mach_port_t object_name;
  kern_return_t kr = mach_vm_region(task, address, size, VM_REGION_TOP_INFO,
                                    reinterpret_cast<vm_region_info_t>(info),
                                    &info_count, &object_name);
  // The kernel always returns a null object for VM_REGION_TOP_INFO, but
  // balance it with a deallocate in case this ever changes. See 10.9.2
  // xnu-2422.90.20/osfmk/vm/vm_map.c vm_map_region.
  mach_port_deallocate(task, object_name);
  return ParseOutputFromMachVMRegion(kr);
}

MachVMRegionResult GetBasicInfo(mach_port_t task,
                                mach_vm_size_t* size,
                                mach_vm_address_t* address,
                                vm_region_basic_info_64* info) {
  mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT_64;
  mach_port_t object_name;
  kern_return_t kr = mach_vm_region(
      task, address, size, VM_REGION_BASIC_INFO_64,
      reinterpret_cast<vm_region_info_t>(info), &info_count, &object_name);
  // The kernel always returns a null object for VM_REGION_BASIC_INFO_64, but
  // balance it with a deallocate in case this ever changes. See 10.9.2
  // xnu-2422.90.20/osfmk/vm/vm_map.c vm_map_region.
  mach_port_deallocate(task, object_name);
  return ParseOutputFromMachVMRegion(kr);
}

}  // namespace base
