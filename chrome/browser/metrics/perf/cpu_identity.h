// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_METRICS_PERF_CPU_IDENTITY_H_
#define CHROME_BROWSER_METRICS_PERF_CPU_IDENTITY_H_

#include <string>

// Struct containing the CPU identity fields used to choose perf commands.
// These are populated from base::CPU, but having them in a settable struct
// makes things testable.
struct CPUIdentity {
  CPUIdentity();
  CPUIdentity(const CPUIdentity& other);
  ~CPUIdentity();

  // The system architecture based on uname().
  // (Technically, not a property of the CPU.)
  std::string arch;
  // The kernel release version.
  std::string release;
  // CUID fields:
  std::string vendor;  // e.g. "GenuineIntel"
  int family;
  int model;
  // CPU model name. e.g. "Intel(R) Celeron(R) 2955U @ 1.40GHz"
  std::string model_name;
};

// Get the CPUIdentity based on the actual system.
CPUIdentity GetCPUIdentity();

// Return the Intel microarchitecture based on the family and model derived
// from |cpuid|, and kIntelUarchTable, or the empty string for non-Intel or
// unknown microarchitectures.
std::string GetIntelUarch(const CPUIdentity& cpuid);

// Simplify a CPU model name. The rules are:
// - Replace spaces with hyphens.
// - Strip all "(R)" symbols.
// - Convert to lower case.
std::string SimplifyCPUModelName(const std::string& model_name);

namespace internal {

// Exposed for unit testing.

struct IntelUarchTableEntry {
  const char *family_model;
  const char *uarch;
};

bool IntelUarchTableCmp(const IntelUarchTableEntry& a,
                        const IntelUarchTableEntry& b);

extern const IntelUarchTableEntry kIntelUarchTable[];
extern const IntelUarchTableEntry* kIntelUarchTableEnd;

}  // namespace internal

#endif  // CHROME_BROWSER_METRICS_PERF_CPU_IDENTITY_H_
