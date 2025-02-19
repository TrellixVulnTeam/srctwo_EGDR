// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromecast/browser/metrics/cast_stability_metrics_provider.h"

#include <vector>

#include "base/logging.h"
#include "base/metrics/histogram.h"
#include "base/metrics/histogram_macros.h"
#include "chromecast/base/pref_names.h"
#include "chromecast/browser/cast_browser_process.h"
#include "chromecast/browser/metrics/cast_metrics_service_client.h"
#include "components/metrics/metrics_service.h"
#include "components/metrics/proto/system_profile.pb.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/child_process_data.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_process_host.h"

namespace chromecast {
namespace metrics {

namespace {

enum RendererType {
  RENDERER_TYPE_RENDERER = 1,
  RENDERER_TYPE_EXTENSION, //  Not used, but needed for correct histogram count.
  // NOTE: Add new action types only immediately above this line. Also,
  // make sure the enum list in tools/metrics/histograms/histograms.xml is
  // updated with any change in here.
  RENDERER_TYPE_COUNT
};

void IncrementPrefValue(const char* path) {
  PrefService* pref = shell::CastBrowserProcess::GetInstance()->pref_service();
  DCHECK(pref);
  int value = pref->GetInteger(path);
  pref->SetInteger(path, value + 1);
}

// Converts an exit code into something that can be inserted into our
// histograms (which expect non-negative numbers less than MAX_INT).
int MapCrashExitCodeForHistogram(int exit_code) {
  return std::abs(exit_code);
}

}  // namespace

// static
void CastStabilityMetricsProvider::RegisterPrefs(PrefRegistrySimple* registry) {
  registry->RegisterIntegerPref(prefs::kStabilityRendererCrashCount, 0);
  registry->RegisterIntegerPref(prefs::kStabilityRendererFailedLaunchCount, 0);
  registry->RegisterIntegerPref(prefs::kStabilityRendererHangCount, 0);
  registry->RegisterIntegerPref(prefs::kStabilityChildProcessCrashCount, 0);
}

CastStabilityMetricsProvider::CastStabilityMetricsProvider(
    ::metrics::MetricsService* metrics_service)
    : metrics_service_(metrics_service) {
  BrowserChildProcessObserver::Add(this);
}

CastStabilityMetricsProvider::~CastStabilityMetricsProvider() {
  BrowserChildProcessObserver::Remove(this);
}

void CastStabilityMetricsProvider::OnRecordingEnabled() {
  registrar_.Add(this,
                 content::NOTIFICATION_RENDERER_PROCESS_CLOSED,
                 content::NotificationService::AllSources());
  registrar_.Add(this,
                 content::NOTIFICATION_RENDER_WIDGET_HOST_HANG,
                 content::NotificationService::AllSources());
}

void CastStabilityMetricsProvider::OnRecordingDisabled() {
  registrar_.RemoveAll();
}

void CastStabilityMetricsProvider::ProvideStabilityMetrics(
    ::metrics::SystemProfileProto* system_profile_proto) {
  PrefService* pref = shell::CastBrowserProcess::GetInstance()->pref_service();
  ::metrics::SystemProfileProto_Stability* stability_proto =
      system_profile_proto->mutable_stability();

  int count = pref->GetInteger(prefs::kStabilityChildProcessCrashCount);
  if (count) {
    stability_proto->set_child_process_crash_count(count);
    pref->SetInteger(prefs::kStabilityChildProcessCrashCount, 0);
  }

  count = pref->GetInteger(prefs::kStabilityRendererCrashCount);
  if (count) {
    stability_proto->set_renderer_crash_count(count);
    pref->SetInteger(prefs::kStabilityRendererCrashCount, 0);
  }

  count = pref->GetInteger(prefs::kStabilityRendererFailedLaunchCount);
  if (count) {
    stability_proto->set_renderer_failed_launch_count(count);
    pref->SetInteger(prefs::kStabilityRendererFailedLaunchCount, 0);
  }

  count = pref->GetInteger(prefs::kStabilityRendererHangCount);
  if (count) {
    stability_proto->set_renderer_hang_count(count);
    pref->SetInteger(prefs::kStabilityRendererHangCount, 0);
  }
}

void CastStabilityMetricsProvider::LogExternalCrash(
    const std::string& crash_type) {
  if (crash_type == "user")
    IncrementPrefValue(prefs::kStabilityOtherUserCrashCount);
  else if (crash_type == "kernel")
    IncrementPrefValue(prefs::kStabilityKernelCrashCount);
  else if (crash_type == "uncleanshutdown")
    IncrementPrefValue(prefs::kStabilitySystemUncleanShutdownCount);
  else
    NOTREACHED() << "Unexpected crash type " << crash_type;

  // Wake up metrics logs sending if necessary now that new
  // log data is available.
  metrics_service_->OnApplicationNotIdle();
}

void CastStabilityMetricsProvider::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  switch (type) {
    case content::NOTIFICATION_RENDERER_PROCESS_CLOSED: {
      content::RenderProcessHost::RendererClosedDetails* process_details =
          content::Details<content::RenderProcessHost::RendererClosedDetails>(
              details).ptr();
      content::RenderProcessHost* host =
          content::Source<content::RenderProcessHost>(source).ptr();
      LogRendererCrash(
          host, process_details->status, process_details->exit_code);
      break;
    }

    case content::NOTIFICATION_RENDER_WIDGET_HOST_HANG:
      LogRendererHang();
      break;

    default:
      NOTREACHED();
      break;
  }
}

void CastStabilityMetricsProvider::BrowserChildProcessCrashed(
    const content::ChildProcessData& data,
    int exit_code) {
  IncrementPrefValue(prefs::kStabilityChildProcessCrashCount);
}

void CastStabilityMetricsProvider::LogRendererCrash(
    content::RenderProcessHost* host,
    base::TerminationStatus status,
    int exit_code) {
  if (status == base::TERMINATION_STATUS_PROCESS_CRASHED ||
      status == base::TERMINATION_STATUS_ABNORMAL_TERMINATION) {
    IncrementPrefValue(prefs::kStabilityRendererCrashCount);

    UMA_HISTOGRAM_SPARSE_SLOWLY("CrashExitCodes.Renderer",
                                MapCrashExitCodeForHistogram(exit_code));
    UMA_HISTOGRAM_ENUMERATION("BrowserRenderProcessHost.ChildCrashes",
                              RENDERER_TYPE_RENDERER, RENDERER_TYPE_COUNT);
  } else if (status == base::TERMINATION_STATUS_PROCESS_WAS_KILLED) {
    UMA_HISTOGRAM_ENUMERATION("BrowserRenderProcessHost.ChildKills",
                              RENDERER_TYPE_RENDERER, RENDERER_TYPE_COUNT);
  } else if (status == base::TERMINATION_STATUS_STILL_RUNNING) {
    UMA_HISTOGRAM_ENUMERATION("BrowserRenderProcessHost.DisconnectedAlive",
                              RENDERER_TYPE_RENDERER, RENDERER_TYPE_COUNT);
  } else if (status == base::TERMINATION_STATUS_LAUNCH_FAILED) {
    IncrementPrefValue(prefs::kStabilityRendererFailedLaunchCount);
  }
}

void CastStabilityMetricsProvider::LogRendererHang() {
  IncrementPrefValue(prefs::kStabilityRendererHangCount);
}

}  // namespace metrics
}  // namespace chromecast
