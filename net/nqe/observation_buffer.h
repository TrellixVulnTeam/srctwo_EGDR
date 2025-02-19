// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_NQE_OBSERVATION_BUFFER_H_
#define NET_NQE_OBSERVATION_BUFFER_H_

#include <stdint.h>

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "base/optional.h"
#include "base/time/tick_clock.h"
#include "net/base/net_export.h"
#include "net/nqe/network_quality_estimator_util.h"
#include "net/nqe/network_quality_observation.h"
#include "net/nqe/network_quality_observation_source.h"

namespace base {

class TimeTicks;

}  // namespace base

namespace net {

namespace nqe {

namespace internal {

namespace {

// Maximum number of observations that can be held in the ObservationBuffer.
static const size_t kMaximumObservationsBufferSize = 300;

}  // namespace

struct WeightedObservation;

// Stores observations sorted by time and provides utility functions for
// computing weighted and non-weighted summary statistics.
class NET_EXPORT_PRIVATE ObservationBuffer {
 public:
  ObservationBuffer(double weight_multiplier_per_second,
                    double weight_multiplier_per_signal_level);

  ~ObservationBuffer();

  // Adds |observation| to the buffer. The oldest observation in the buffer
  // will be evicted to make room if the buffer is already full.
  void AddObservation(const Observation& observation);

  // Returns the number of observations in this buffer.
  size_t Size() const { return static_cast<size_t>(observations_.size()); }

  // Returns the capacity of this buffer.
  size_t Capacity() const { return kMaximumObservationsBufferSize; }

  // Clears the observations stored in this buffer.
  void Clear() { observations_.clear(); }

  // Returns true iff the |percentile| value of the observations in this
  // buffer is available. Sets |result| to the computed |percentile|
  // value of all observations made on or after |begin_timestamp|. If the
  // value is unavailable, false is returned and |result| is not modified.
  // Percentile value is unavailable if all the values in observation buffer are
  // older than |begin_timestamp|. |current_signal_strength| is the current
  // signal strength. |result| must not be null.
  // TODO(tbansal): Move out param |result| as the last param of the function.
  base::Optional<int32_t> GetPercentile(
      base::TimeTicks begin_timestamp,
      const base::Optional<int32_t>& current_signal_strength,
      int percentile,
      const std::vector<NetworkQualityObservationSource>&
          disallowed_observation_sources) const;

  void SetTickClockForTesting(std::unique_ptr<base::TickClock> tick_clock) {
    tick_clock_ = std::move(tick_clock);
  }

  // Computes percentiles separately for each host. Observations without
  // a host tag are skipped. Only data from the hosts present in |host_filter|
  // are considered. Observations before |begin_timestamp| are skipped. The
  // percentile value for each host is returned in |host_keyed_percentiles|. The
  // number of valid observations for each host used for the computation is
  // returned in |host_keyed_counts|.
  void GetPercentileForEachHostWithCounts(
      base::TimeTicks begin_timestamp,
      int percentile,
      const std::vector<NetworkQualityObservationSource>&
          disallowed_observation_sources,
      const base::Optional<std::set<IPHash>>& host_filter,
      std::map<IPHash, int32_t>* host_keyed_percentiles,
      std::map<IPHash, size_t>* host_keyed_counts) const;

 private:
  // Computes the weighted observations and stores them in
  // |weighted_observations| sorted by ascending |WeightedObservation.value|.
  // Only the observations with timestamp later than |begin_timestamp| are
  // considered. |current_signal_strength| is the current signal strength
  // when the observation was taken. This method also sets |total_weight| to
  // the total weight of all observations. Should be called only when there is
  // at least one observation in the buffer.
  void ComputeWeightedObservations(
      const base::TimeTicks& begin_timestamp,
      const base::Optional<int32_t>& current_signal_strength,
      std::vector<WeightedObservation>* weighted_observations,
      double* total_weight,
      const std::vector<NetworkQualityObservationSource>&
          disallowed_observation_sources) const;

  // Holds observations sorted by time, with the oldest observation at the
  // front of the queue.
  std::deque<Observation> observations_;

  // The factor by which the weight of an observation reduces every second.
  // For example, if an observation is 6 seconds old, its weight would be:
  //     weight_multiplier_per_second_ ^ 6
  // Calculated from |kHalfLifeSeconds| by solving the following equation:
  //     weight_multiplier_per_second_ ^ kHalfLifeSeconds = 0.5
  const double weight_multiplier_per_second_;

  // The factor by which the weight of an observation reduces for every unit
  // difference in the current signal strength, and the signal strength at
  // which the observation was taken.
  // For example, if the observation was taken at 1 unit, and current signal
  // strength is 4 units, the weight of the observation would be:
  // |weight_multiplier_per_signal_level_| ^ 3.
  const double weight_multiplier_per_signal_level_;

  std::unique_ptr<base::TickClock> tick_clock_;

  DISALLOW_COPY_AND_ASSIGN(ObservationBuffer);
};

}  // namespace internal

}  // namespace nqe

}  // namespace net

#endif  // NET_NQE_OBSERVATION_BUFFER_H_
