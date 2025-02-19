// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromecast/base/metrics/grouped_histogram.h"

#include <stddef.h>
#include <stdint.h>

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/metrics/histogram.h"
#include "base/metrics/statistics_recorder.h"
#include "base/strings/stringprintf.h"
#include "base/synchronization/lock.h"
#include "base/time/time.h"

namespace chromecast {
namespace metrics {

namespace {

const char kAppNameErrorNoApp[] = "ERROR_NO_APP_REGISTERED";

// Current app name guarded by lock.
struct CurrentAppNameWithLock {
  base::Lock lock;
  std::string app_name;
};

base::LazyInstance<CurrentAppNameWithLock>::DestructorAtExit g_current_app =
    LAZY_INSTANCE_INITIALIZER;

std::string GetAppName() {
  base::AutoLock lock(g_current_app.Get().lock);
  const std::string& app_name = g_current_app.Get().app_name;
  return app_name.empty() ? kAppNameErrorNoApp : app_name;
}

struct HistogramArgs {
  const char* name;
  int minimum;
  int maximum;
  uint32_t bucket_count;
};

// List of metrics to collect using a GroupedHistogram.
//
// When adding more Histograms to this list, find the source of the
// Histogram and look for the construction arguments it uses to add it in.
const HistogramArgs kHistogramsToGroup[] = {
  {
    "DNS.TotalTime",
    1,
    1000 * 60 * 60,
    100,
  },
  {
    "Net.DNS_Resolution_And_TCP_Connection_Latency2",
    1,
    1000 * 60 * 10,
    100,
  },
  {
    "Net.SSL_Connection_Latency2",
    1,
    1000 * 60,
    100,
  },
  {
    "Net.TCP_Connection_Latency",
    1,
    1000 * 60 * 10,
    100,
  },
  {
    "Net.HttpJob.TotalTime",
    1,
    1000 * 10,
    50,
  },
};

// This class is used to override a Histogram to generate per-app metrics.
// It intercepts calls to Add() for a given metric and generates new metrics
// of the form "<metric-name>.<app-name>".
class GroupedHistogram : public base::Histogram {
 public:
  GroupedHistogram(const std::string& metric_to_override,
                   Sample minimum,
                   Sample maximum,
                   const base::BucketRanges* ranges)
      : Histogram(metric_to_override, minimum, maximum, ranges),
        metric_to_group_(metric_to_override),
        minimum_(minimum),
        maximum_(maximum),
        bucket_count_(ranges->bucket_count()) {
  }

  ~GroupedHistogram() override {
  }

  // base::Histogram implementation:
  void Add(Sample value) override {
    Histogram::Add(value);
    std::string name(base::StringPrintf("%s.%s",
                                        histogram_name().c_str(),
                                        GetAppName().c_str()));
    HistogramBase* grouped_histogram =
        base::Histogram::FactoryGet(name,
                                    minimum_,
                                    maximum_,
                                    bucket_count_,
                                    flags());
    DCHECK(grouped_histogram);
    grouped_histogram->Add(value);
  }

 private:
  // Saved construction arguments for reconstructing the Histogram later (with
  // a suffixed app name).
  std::string metric_to_group_;
  Sample minimum_;
  Sample maximum_;
  uint32_t bucket_count_;

  DISALLOW_COPY_AND_ASSIGN(GroupedHistogram);
};

// Registers a GroupedHistogram with StatisticsRecorder.  Must be called
// before any Histogram of the same name has been used.
// It acts similarly to Histogram::FactoryGet but checks that
// the histogram is being newly created and does not already exist.
void PreregisterHistogram(const std::string& name,
                          GroupedHistogram::Sample minimum,
                          GroupedHistogram::Sample maximum,
                          uint32_t bucket_count,
                          int32_t flags) {
  DCHECK(base::StatisticsRecorder::IsActive());
  DCHECK(base::Histogram::InspectConstructionArguments(
      name, &minimum, &maximum, &bucket_count));
  DCHECK(!base::StatisticsRecorder::FindHistogram(name))
      << "Failed to preregister " << name << ", Histogram already exists.";

  // To avoid racy destruction at shutdown, the following will be leaked.
  base::BucketRanges* ranges = new base::BucketRanges(bucket_count + 1);
  base::Histogram::InitializeBucketRanges(minimum, maximum, ranges);
  const base::BucketRanges* registered_ranges =
      base::StatisticsRecorder::RegisterOrDeleteDuplicateRanges(ranges);

  GroupedHistogram* tentative_histogram =
      new GroupedHistogram(name, minimum, maximum, registered_ranges);

  tentative_histogram->SetFlags(flags);
  base::HistogramBase* histogram =
      base::StatisticsRecorder::RegisterOrDeleteDuplicate(tentative_histogram);

  DCHECK_EQ(histogram, tentative_histogram);
  DCHECK_EQ(base::HISTOGRAM, histogram->GetHistogramType());
  DCHECK(histogram->HasConstructionArguments(minimum, maximum, bucket_count));
}

} // namespace

void PreregisterAllGroupedHistograms() {
  base::StatisticsRecorder::Initialize();
  for (size_t i = 0; i < arraysize(kHistogramsToGroup); ++i) {
    PreregisterHistogram(
        kHistogramsToGroup[i].name,
        kHistogramsToGroup[i].minimum,
        kHistogramsToGroup[i].maximum,
        kHistogramsToGroup[i].bucket_count,
        base::HistogramBase::kUmaTargetedHistogramFlag);
  }
}

void TagAppStartForGroupedHistograms(const std::string& app_name) {
  base::AutoLock lock(g_current_app.Get().lock);
  g_current_app.Get().app_name = app_name;
}

} // namespace metrics
} // namespace chromecast
