// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/experiments/memory_ablation_experiment.h"

#include <algorithm>
#include <limits>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/debug/alias.h"
#include "base/metrics/field_trial_params.h"
#include "base/process/process_metrics.h"
#include "base/sequenced_task_runner.h"
#include "base/sys_info.h"

const base::Feature kMemoryAblationFeature{"MemoryAblation",
                                           base::FEATURE_DISABLED_BY_DEFAULT};

const char kMemoryAblationFeatureSizeParam[] = "Size";
const char kMemoryAblationFeatureMinRAMParam[] = "MinRAM";
const char kMemoryAblationFeatureMaxRAMParam[] = "MaxRAM";

// Since MaybeStart() is called during startup, we delay the allocation
// to avoid slowing things down.
constexpr size_t kAllocationDelaySeconds = 5;

// We need to touch allocated memory to "dirty" it. However, touching
// large chunks of memory (even a single byte per page) can be costly
// (~60ms per 10MiB on Nexus 5). So we touch in chunks to allow other
// things to happen in between. With the following values we'll touch
// 2MiB per second, spending ~10% of 250ms time slice per chunk.
constexpr size_t kTouchDelayMilliseconds = 250;
constexpr size_t kTouchChunkSize = 512 * 1024;

MemoryAblationExperiment::MemoryAblationExperiment() {}

MemoryAblationExperiment::~MemoryAblationExperiment() {}

MemoryAblationExperiment* MemoryAblationExperiment::GetInstance() {
  static auto* instance = new MemoryAblationExperiment();
  return instance;
}

void MemoryAblationExperiment::MaybeStart(
    scoped_refptr<base::SequencedTaskRunner> task_runner) {
  int min_ram_mib = base::GetFieldTrialParamByFeatureAsInt(
      kMemoryAblationFeature, kMemoryAblationFeatureMinRAMParam,
      0 /* default value */);
  int max_ram_mib = base::GetFieldTrialParamByFeatureAsInt(
      kMemoryAblationFeature, kMemoryAblationFeatureMaxRAMParam,
      std::numeric_limits<int>::max() /* default value */);
  if (base::SysInfo::AmountOfPhysicalMemoryMB() > max_ram_mib ||
      base::SysInfo::AmountOfPhysicalMemoryMB() <= min_ram_mib) {
    return;
  }

  int size = base::GetFieldTrialParamByFeatureAsInt(
      kMemoryAblationFeature, kMemoryAblationFeatureSizeParam,
      0 /* default value */);
  if (size > 0) {
    GetInstance()->Start(task_runner, size);
  }
}

void MemoryAblationExperiment::Start(
    scoped_refptr<base::SequencedTaskRunner> task_runner,
    size_t memory_size) {
  DCHECK(task_runner_ == nullptr) << "Already started";
  task_runner_ = task_runner;
  // This class is a singleton, so "Unretained(this)" below is fine.
  task_runner_->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&MemoryAblationExperiment::AllocateMemory,
                     base::Unretained(this), memory_size),
      base::TimeDelta::FromSeconds(kAllocationDelaySeconds));
}

void MemoryAblationExperiment::AllocateMemory(size_t size) {
  memory_size_ = size;
  memory_.reset(new uint8_t[size]);
  ScheduleTouchMemory(0);
}

void MemoryAblationExperiment::TouchMemory(size_t offset) {
  if (memory_) {
    size_t page_size = base::GetPageSize();
    auto* memory = static_cast<volatile uint8_t*>(memory_.get());
    size_t max_offset = std::min(offset + kTouchChunkSize, memory_size_);
    for (; offset < max_offset; offset += page_size) {
      memory[offset] = static_cast<uint8_t>(offset);
    }
    base::debug::Alias(memory_.get());
    if (offset < memory_size_) {
      ScheduleTouchMemory(offset);
    }
  }
}

void MemoryAblationExperiment::ScheduleTouchMemory(size_t offset) {
  // This class is a singleton, so "Unretained(this)" below is fine.
  task_runner_->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&MemoryAblationExperiment::TouchMemory,
                     base::Unretained(this), offset),
      base::TimeDelta::FromMilliseconds(kTouchDelayMilliseconds));
}
