// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gl/angle_platform_impl.h"

#include "base/base64.h"
#include "base/callback.h"
#include "base/lazy_instance.h"
#include "base/metrics/histogram.h"
#include "base/metrics/histogram_macros.h"
#include "base/trace_event/trace_event.h"
#include "third_party/angle/include/platform/Platform.h"
#include "ui/gl/gl_bindings.h"

namespace angle {

namespace {

// This platform context stores user data accessible inside the impl methods.
struct PlatformContext {
  CacheProgramCallback cache_program_callback;
};

base::LazyInstance<PlatformContext>::DestructorAtExit g_platform_context =
    LAZY_INSTANCE_INITIALIZER;
ResetDisplayPlatformFunc g_angle_reset_platform = nullptr;

double ANGLEPlatformImpl_currentTime(PlatformMethods* platform) {
  return base::Time::Now().ToDoubleT();
}

double ANGLEPlatformImpl_monotonicallyIncreasingTime(
    PlatformMethods* platform) {
  return (base::TimeTicks::Now() - base::TimeTicks()).InSecondsF();
}

const unsigned char* ANGLEPlatformImpl_getTraceCategoryEnabledFlag(
    PlatformMethods* platform,
    const char* category_group) {
  return TRACE_EVENT_API_GET_CATEGORY_GROUP_ENABLED(category_group);
}

void ANGLEPlatformImpl_logError(PlatformMethods* platform,
                                const char* errorMessage) {
  LOG(ERROR) << errorMessage;
}

void ANGLEPlatformImpl_logWarning(PlatformMethods* platform,
                                  const char* warningMessage) {
  LOG(WARNING) << warningMessage;
}

TraceEventHandle ANGLEPlatformImpl_addTraceEvent(
    PlatformMethods* platform,
    char phase,
    const unsigned char* category_group_enabled,
    const char* name,
    unsigned long long id,
    double timestamp,
    int num_args,
    const char** arg_names,
    const unsigned char* arg_types,
    const unsigned long long* arg_values,
    unsigned char flags) {
  base::TimeTicks timestamp_tt =
      base::TimeTicks() + base::TimeDelta::FromSecondsD(timestamp);
  base::trace_event::TraceEventHandle handle =
      TRACE_EVENT_API_ADD_TRACE_EVENT_WITH_THREAD_ID_AND_TIMESTAMP(
          phase, category_group_enabled, name,
          trace_event_internal::kGlobalScope, id, trace_event_internal::kNoId,
          base::PlatformThread::CurrentId(), timestamp_tt, num_args, arg_names,
          arg_types, arg_values, nullptr, flags);
  TraceEventHandle result;
  memcpy(&result, &handle, sizeof(result));
  return result;
}

void ANGLEPlatformImpl_updateTraceEventDuration(
    PlatformMethods* platform,
    const unsigned char* category_group_enabled,
    const char* name,
    TraceEventHandle handle) {
  base::trace_event::TraceEventHandle trace_event_handle;
  memcpy(&trace_event_handle, &handle, sizeof(handle));
  TRACE_EVENT_API_UPDATE_TRACE_EVENT_DURATION(category_group_enabled, name,
                                              trace_event_handle);
}

void ANGLEPlatformImpl_histogramCustomCounts(PlatformMethods* platform,
                                             const char* name,
                                             int sample,
                                             int min,
                                             int max,
                                             int bucket_count) {
  // Copied from histogram macro, but without the static variable caching
  // the histogram because name is dynamic.
  base::HistogramBase* counter = base::Histogram::FactoryGet(
      name, min, max, bucket_count,
      base::HistogramBase::kUmaTargetedHistogramFlag);
  DCHECK_EQ(name, counter->histogram_name());
  counter->Add(sample);
}

void ANGLEPlatformImpl_histogramEnumeration(PlatformMethods* platform,
                                            const char* name,
                                            int sample,
                                            int boundary_value) {
  // Copied from histogram macro, but without the static variable caching
  // the histogram because name is dynamic.
  base::HistogramBase* counter = base::LinearHistogram::FactoryGet(
      name, 1, boundary_value, boundary_value + 1,
      base::HistogramBase::kUmaTargetedHistogramFlag);
  DCHECK_EQ(name, counter->histogram_name());
  counter->Add(sample);
}

void ANGLEPlatformImpl_histogramSparse(PlatformMethods* platform,
                                       const char* name,
                                       int sample) {
  // For sparse histograms, we can use the macro, as it does not incorporate a
  // static.
  UMA_HISTOGRAM_SPARSE_SLOWLY(name, sample);
}

void ANGLEPlatformImpl_histogramBoolean(PlatformMethods* platform,
                                        const char* name,
                                        bool sample) {
  ANGLEPlatformImpl_histogramEnumeration(platform, name, sample ? 1 : 0, 2);
}

void ANGLEPlatformImpl_cacheProgram(PlatformMethods* platform,
                                    const ProgramKeyType& key,
                                    size_t program_size,
                                    const uint8_t* program_bytes) {
  PlatformContext* context =
      reinterpret_cast<PlatformContext*>(platform->context);
  if (context && context->cache_program_callback) {
    // Convert the key and binary to string form.
    std::string key_string(reinterpret_cast<const char*>(&key[0]),
                           sizeof(ProgramKeyType));
    std::string value_string(reinterpret_cast<const char*>(program_bytes),
                             program_size);
    std::string key_string_64;
    std::string value_string_64;
    base::Base64Encode(key_string, &key_string_64);
    base::Base64Encode(value_string, &value_string_64);
    context->cache_program_callback.Run(key_string_64, value_string_64);
  }
}

}  // anonymous namespace

bool InitializePlatform(EGLDisplay display) {
  GetDisplayPlatformFunc angle_get_platform =
      reinterpret_cast<GetDisplayPlatformFunc>(
          eglGetProcAddress("ANGLEGetDisplayPlatform"));
  if (!angle_get_platform)
    return false;

  // Save the pointer to the destroy function here to avoid crash.
  g_angle_reset_platform = reinterpret_cast<ResetDisplayPlatformFunc>(
      eglGetProcAddress("ANGLEResetDisplayPlatform"));

  PlatformMethods* platformMethods = nullptr;
  if (!angle_get_platform(static_cast<EGLDisplayType>(display),
                          g_PlatformMethodNames, g_NumPlatformMethods,
                          &g_platform_context.Get(), &platformMethods))
    return false;
  platformMethods->currentTime = ANGLEPlatformImpl_currentTime;
  platformMethods->addTraceEvent = ANGLEPlatformImpl_addTraceEvent;
  platformMethods->currentTime = ANGLEPlatformImpl_currentTime;
  platformMethods->getTraceCategoryEnabledFlag =
      ANGLEPlatformImpl_getTraceCategoryEnabledFlag;
  platformMethods->histogramBoolean = ANGLEPlatformImpl_histogramBoolean;
  platformMethods->histogramCustomCounts =
      ANGLEPlatformImpl_histogramCustomCounts;
  platformMethods->histogramEnumeration =
      ANGLEPlatformImpl_histogramEnumeration;
  platformMethods->histogramSparse = ANGLEPlatformImpl_histogramSparse;
  platformMethods->logError = ANGLEPlatformImpl_logError;
  platformMethods->logWarning = ANGLEPlatformImpl_logWarning;
  platformMethods->monotonicallyIncreasingTime =
      ANGLEPlatformImpl_monotonicallyIncreasingTime;
  platformMethods->updateTraceEventDuration =
      ANGLEPlatformImpl_updateTraceEventDuration;
  platformMethods->cacheProgram = ANGLEPlatformImpl_cacheProgram;
  return true;
}

void ResetPlatform(EGLDisplay display) {
  if (!g_angle_reset_platform)
    return;
  g_angle_reset_platform(static_cast<EGLDisplayType>(display));
  ResetCacheProgramCallback();
}

void SetCacheProgramCallback(CacheProgramCallback callback) {
  g_platform_context.Get().cache_program_callback = callback;
}

void ResetCacheProgramCallback() {
  g_platform_context.Get().cache_program_callback.Reset();
}

}  // namespace angle
