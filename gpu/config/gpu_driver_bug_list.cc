// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/config/gpu_driver_bug_list.h"

#include "base/logging.h"
#include "gpu/config/gpu_driver_bug_list_autogen.h"
#include "gpu/config/gpu_driver_bug_workaround_type.h"
#include "gpu/config/gpu_switches.h"
#include "gpu/config/gpu_util.h"

namespace gpu {

namespace {

struct GpuDriverBugWorkaroundInfo {
  GpuDriverBugWorkaroundType type;
  const char* name;
};

const GpuDriverBugWorkaroundInfo kFeatureList[] = {
#define GPU_OP(type, name) { type, #name },
  GPU_DRIVER_BUG_WORKAROUNDS(GPU_OP)
#undef GPU_OP
};

}  // namespace anonymous

GpuDriverBugList::GpuDriverBugList(const GpuControlListData& data)
    : GpuControlList(data) {}

GpuDriverBugList::~GpuDriverBugList() {
}

// static
std::unique_ptr<GpuDriverBugList> GpuDriverBugList::Create() {
  GpuControlListData data(kGpuDriverBugListVersion, kGpuDriverBugListEntryCount,
                          kGpuDriverBugListEntries);
  return Create(data);
}

// static
std::unique_ptr<GpuDriverBugList> GpuDriverBugList::Create(
    const GpuControlListData& data) {
  std::unique_ptr<GpuDriverBugList> list(new GpuDriverBugList(data));

  DCHECK_EQ(static_cast<int>(arraysize(kFeatureList)),
            NUMBER_OF_GPU_DRIVER_BUG_WORKAROUND_TYPES);
  for (int i = 0; i < NUMBER_OF_GPU_DRIVER_BUG_WORKAROUND_TYPES; ++i) {
    list->AddSupportedFeature(kFeatureList[i].name,
                              kFeatureList[i].type);
  }
  return list;
}

std::string GpuDriverBugWorkaroundTypeToString(
    GpuDriverBugWorkaroundType type) {
  if (type < NUMBER_OF_GPU_DRIVER_BUG_WORKAROUND_TYPES)
    return kFeatureList[type].name;
  else
    return "unknown";
}

// static
void GpuDriverBugList::AppendWorkaroundsFromCommandLine(
    std::set<int>* workarounds,
    const base::CommandLine& command_line) {
  DCHECK(workarounds);
  for (int i = 0; i < NUMBER_OF_GPU_DRIVER_BUG_WORKAROUND_TYPES; i++) {
    if (command_line.HasSwitch(kFeatureList[i].name)) {
      // Check for disabling workaround flag.
      if (command_line.GetSwitchValueASCII(kFeatureList[i].name) == "0") {
        workarounds->erase(kFeatureList[i].type);
        continue;
      }

      // Removing conflicting workarounds.
      switch (kFeatureList[i].type) {
        case FORCE_DISCRETE_GPU:
          workarounds->erase(FORCE_INTEGRATED_GPU);
          workarounds->insert(FORCE_DISCRETE_GPU);
          break;
        case FORCE_INTEGRATED_GPU:
          workarounds->erase(FORCE_DISCRETE_GPU);
          workarounds->insert(FORCE_INTEGRATED_GPU);
          break;
        default:
          workarounds->insert(kFeatureList[i].type);
          break;
      }
    }
  }
}

// static
void GpuDriverBugList::AppendAllWorkarounds(
    std::vector<const char*>* workarounds) {
  workarounds->resize(workarounds->size() +
                      NUMBER_OF_GPU_DRIVER_BUG_WORKAROUND_TYPES);

#define GPU_OP(type, name) workarounds->push_back(#name);
  GPU_DRIVER_BUG_WORKAROUNDS(GPU_OP)
#undef GPU_OP
}

}  // namespace gpu
