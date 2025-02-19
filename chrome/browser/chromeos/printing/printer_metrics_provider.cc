// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/printing/printer_metrics_provider.h"

#include "chrome/browser/browser_process.h"
#include "chrome/browser/chromeos/printing/printer_event_tracker.h"
#include "chrome/browser/chromeos/printing/printer_event_tracker_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "components/metrics/proto/chrome_user_metrics_extension.pb.h"
#include "components/metrics/proto/printer_event.pb.h"

namespace chromeos {

PrinterMetricsProvider::PrinterMetricsProvider() = default;
PrinterMetricsProvider::~PrinterMetricsProvider() = default;

void PrinterMetricsProvider::OnRecordingEnabled() {
  PrinterEventTrackerFactory::GetInstance()->SetLogging(true);
}

void PrinterMetricsProvider::OnRecordingDisabled() {
  PrinterEventTrackerFactory::GetInstance()->SetLogging(false);
}

void PrinterMetricsProvider::ProvideCurrentSessionData(
    metrics::ChromeUserMetricsExtension* uma_proto) {
  PrinterEventTrackerFactory* factory =
      PrinterEventTrackerFactory::GetInstance();
  std::vector<Profile*> profiles =
      g_browser_process->profile_manager()->GetLoadedProfiles();
  std::vector<metrics::PrinterEventProto> events;
  for (Profile* profile : profiles) {
    factory->GetForBrowserContext(profile)->FlushPrinterEvents(&events);

    for (metrics::PrinterEventProto& event : events) {
      uma_proto->add_printer_event()->Swap(&event);
    }
    events.clear();
  }
}

}  // namespace chromeos
