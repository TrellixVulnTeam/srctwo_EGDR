// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PREDICTORS_LOADING_STATS_COLLECTOR_H_
#define CHROME_BROWSER_PREDICTORS_LOADING_STATS_COLLECTOR_H_

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "base/macros.h"
#include "base/time/time.h"
#include "chrome/browser/predictors/resource_prefetcher.h"
#include "url/gurl.h"

namespace predictors {

class ResourcePrefetchPredictor;
struct PreconnectStats;
struct LoadingPredictorConfig;
struct PageRequestSummary;

namespace internal {
constexpr char kResourcePrefetchPredictorPrecisionHistogram[] =
    "ResourcePrefetchPredictor.LearningPrecision";
constexpr char kResourcePrefetchPredictorRecallHistogram[] =
    "ResourcePrefetchPredictor.LearningRecall";
constexpr char kResourcePrefetchPredictorCountHistogram[] =
    "ResourcePrefetchPredictor.LearningCount";
constexpr char kResourcePrefetchPredictorPrefetchMissesCountCached[] =
    "ResourcePrefetchPredictor.PrefetchMissesCount.Cached";
constexpr char kResourcePrefetchPredictorPrefetchMissesCountNotCached[] =
    "ResourcePrefetchPredictor.PrefetchMissesCount.NotCached";
constexpr char kResourcePrefetchPredictorPrefetchHitsCountCached[] =
    "ResourcePrefetchPredictor.PrefetchHitsCount.Cached";
constexpr char kResourcePrefetchPredictorPrefetchHitsCountNotCached[] =
    "ResourcePrefetchPredictor.PrefetchHitsCount.NotCached";
constexpr char kResourcePrefetchPredictorPrefetchHitsSize[] =
    "ResourcePrefetchPredictor.PrefetchHitsSizeKB";
constexpr char kResourcePrefetchPredictorPrefetchMissesSize[] =
    "ResourcePrefetchPredictor.PrefetchMissesSizeKB";
constexpr char kResourcePrefetchPredictorRedirectStatusHistogram[] =
    "ResourcePrefetchPredictor.RedirectStatus";
constexpr char kLoadingPredictorPreresolveHitsPercentage[] =
    "LoadingPredictor.PreresolveHitsPercentage";
constexpr char kLoadingPredictorPreconnectHitsPercentage[] =
    "LoadingPredictor.PreconnectHitsPercentage";
constexpr char kLoadingPredictorPreresolveCount[] =
    "LoadingPredictor.PreresolveCount";
constexpr char kLoadingPredictorPreconnectCount[] =
    "LoadingPredictor.PreconnectCount";
}  // namespace internal

// Accumulates data from different speculative actions and collates this data
// with a summary of actual page load into statistical reports.
class LoadingStatsCollector {
 public:
  LoadingStatsCollector(ResourcePrefetchPredictor* predictor,
                        const LoadingPredictorConfig& config);
  ~LoadingStatsCollector();

  // Records statistics about a finished prefetching operation.
  void RecordPrefetcherStats(
      std::unique_ptr<ResourcePrefetcher::PrefetcherStats> stats);
  // Records statistics about a finished preconnect operation.
  void RecordPreconnectStats(std::unique_ptr<PreconnectStats> stats);
  // Records a summary of a page load. The summary is collated with speculative
  // actions taken for a given page load if any. The summary is compared with a
  // prediction by ResourcePrefetchPredictor as well.
  // All results are reported to UMA.
  void RecordPageRequestSummary(const PageRequestSummary& summary);
  // Evicts all stale stats that are kept in memory. All speculative actions are
  // reported and considered as waste.
  void CleanupAbandonedStats();

 private:
  ResourcePrefetchPredictor* predictor_;
  base::TimeDelta max_stats_age_;
  std::map<GURL, std::unique_ptr<ResourcePrefetcher::PrefetcherStats>>
      prefetcher_stats_;
  std::map<GURL, std::unique_ptr<PreconnectStats>> preconnect_stats_;

  DISALLOW_COPY_AND_ASSIGN(LoadingStatsCollector);
};

}  // namespace predictors

#endif  // CHROME_BROWSER_PREDICTORS_LOADING_STATS_COLLECTOR_H_
