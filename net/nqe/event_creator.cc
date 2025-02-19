// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/nqe/event_creator.h"

#include <stdlib.h>
#include <memory>
#include <utility>

#include "base/bind.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "net/log/net_log_capture_mode.h"
#include "net/log/net_log_with_source.h"

namespace net {

namespace nqe {

namespace internal {

namespace {

std::unique_ptr<base::Value> NetworkQualityChangedNetLogCallback(
    base::TimeDelta http_rtt,
    base::TimeDelta transport_rtt,
    int32_t downstream_throughput_kbps,
    EffectiveConnectionType effective_connection_type,
    NetLogCaptureMode capture_mode) {
  std::unique_ptr<base::DictionaryValue> dict(new base::DictionaryValue());
  dict->SetInteger("http_rtt_ms", http_rtt.InMilliseconds());
  dict->SetInteger("transport_rtt_ms", transport_rtt.InMilliseconds());
  dict->SetInteger("downstream_throughput_kbps", downstream_throughput_kbps);
  dict->SetString("effective_connection_type",
                  GetNameForEffectiveConnectionType(effective_connection_type));
  return std::move(dict);
}

bool MetricChangedMeaningfully(int32_t past_value, int32_t current_value) {
  if ((past_value == INVALID_RTT_THROUGHPUT) !=
      (current_value == INVALID_RTT_THROUGHPUT)) {
    return true;
  }

  if (past_value == INVALID_RTT_THROUGHPUT &&
      current_value == INVALID_RTT_THROUGHPUT) {
    return false;
  }

  // Create a new entry only if (i) the difference between the two values exceed
  // the threshold; and, (ii) the ratio of the values also exceeds the
  // threshold.
  static const int kMinDifferenceInMetrics = 100;
  static const float kMinRatio = 1.2f;

  if (std::abs(past_value - current_value) < kMinDifferenceInMetrics) {
    // The absolute change in the value is not sufficient enough.
    return false;
  }

  if (past_value < (kMinRatio * current_value) &&
      current_value < (kMinRatio * past_value)) {
    // The relative change in the value is not sufficient enough.
    return false;
  }

  return true;
}

}  // namespace

EventCreator::EventCreator(NetLogWithSource net_log)
    : net_log_(net_log),
      past_effective_connection_type_(EFFECTIVE_CONNECTION_TYPE_UNKNOWN) {}

EventCreator::~EventCreator() {
  DCHECK(thread_checker_.CalledOnValidThread());
}

void EventCreator::MaybeAddNetworkQualityChangedEventToNetLog(
    EffectiveConnectionType effective_connection_type,
    const NetworkQuality& network_quality) {
  DCHECK(thread_checker_.CalledOnValidThread());

  // Check if any of the network quality metrics changed meaningfully.
  bool effective_connection_type_changed =
      past_effective_connection_type_ != effective_connection_type;
  bool http_rtt_changed = MetricChangedMeaningfully(
      past_network_quality_.http_rtt().InMilliseconds(),
      network_quality.http_rtt().InMilliseconds());

  bool transport_rtt_changed = MetricChangedMeaningfully(
      past_network_quality_.transport_rtt().InMilliseconds(),
      network_quality.transport_rtt().InMilliseconds());
  bool kbps_changed = MetricChangedMeaningfully(
      past_network_quality_.downstream_throughput_kbps(),
      network_quality.downstream_throughput_kbps());

  if (!effective_connection_type_changed && !http_rtt_changed &&
      !transport_rtt_changed && !kbps_changed) {
    // Return since none of the metrics changed meaningfully.
    return;
  }

  past_effective_connection_type_ = effective_connection_type;
  past_network_quality_ = network_quality;

  net_log_.AddEvent(
      NetLogEventType::NETWORK_QUALITY_CHANGED,
      base::Bind(&NetworkQualityChangedNetLogCallback,
                 network_quality.http_rtt(), network_quality.transport_rtt(),
                 network_quality.downstream_throughput_kbps(),
                 effective_connection_type));
}

}  // namespace internal

}  // namespace nqe

}  // namespace net
