// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync/test/integration/printers_helper.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "base/run_loop.h"
#include "base/strings/stringprintf.h"
#include "base/threading/sequenced_worker_pool.h"
#include "chrome/browser/chromeos/printing/synced_printers_manager.h"
#include "chrome/browser/chromeos/printing/synced_printers_manager_factory.h"
#include "chrome/browser/sync/test/integration/sync_datatype_helper.h"
#include "chrome/browser/sync/test/integration/sync_test.h"
#include "content/public/test/test_utils.h"

using sync_datatype_helper::test;

namespace printers_helper {

namespace {

using PrinterList = std::vector<chromeos::Printer>;

// Returns true if Printer#id, Printer#description, and Printer#uri all match.
bool PrintersAreMostlyEqual(const chromeos::Printer& left,
                            const chromeos::Printer& right) {
  return left.id() == right.id() && left.description() == right.description() &&
         left.uri() == right.uri();
}

// Returns true if both lists have the same elements irrespective of order.
bool ListsContainTheSamePrinters(const PrinterList& list_a,
                                 const PrinterList& list_b) {
  std::unordered_multimap<std::string, const chromeos::Printer*> map_b;
  for (const auto& b : list_b) {
    map_b.insert({b.id(), &b});
  }

  for (const auto& a : list_a) {
    auto range = map_b.equal_range(a.id());

    auto it = std::find_if(
        range.first, range.second,
        [&a](const std::pair<std::string, const chromeos::Printer*>& entry)
            -> bool { return PrintersAreMostlyEqual(a, *(entry.second)); });

    if (it == range.second) {
      // Element in a does not match an element in b. Lists do not contain the
      // same elements.
      return false;
    }

    map_b.erase(it);
  }

  return map_b.empty();
}

std::string PrinterId(int index) {
  return base::StringPrintf("printer%d", index);
}

chromeos::SyncedPrintersManager* GetPrinterStore(
    content::BrowserContext* context) {
  chromeos::SyncedPrintersManager* manager =
      chromeos::SyncedPrintersManagerFactory::GetForBrowserContext(context);

  // TODO(sync): crbug.com/709094: Remove all of this once the bug is fixed.
  // Must wait for ModelTypeStore initialization. It is fairly difficult to get
  // to the particular SequencedTaskRunner created inside of ModelTypeStoreImpl,
  // so run everything!
  content::RunAllBlockingPoolTasksUntilIdle();
  // Wait for UI thread task completion to make sure PrintersSyncBridge received
  // ModelTypeStore.
  base::RunLoop().RunUntilIdle();

  return manager;
}

}  // namespace

void AddPrinter(chromeos::SyncedPrintersManager* manager,
                const chromeos::Printer& printer) {
  manager->UpdateConfiguredPrinter(printer);
}

void RemovePrinter(chromeos::SyncedPrintersManager* manager, int index) {
  chromeos::Printer testPrinter(CreateTestPrinter(index));
  manager->RemoveConfiguredPrinter(testPrinter.id());
}

bool EditPrinterDescription(chromeos::SyncedPrintersManager* manager,
                            int index,
                            const std::string& description) {
  PrinterList printers = manager->GetConfiguredPrinters();
  std::string printer_id = PrinterId(index);
  auto found =
      std::find_if(printers.begin(), printers.end(),
                   [&printer_id](const chromeos::Printer& printer) -> bool {
                     return printer.id() == printer_id;
                   });

  if (found == printers.end())
    return false;

  found->set_description(description);
  manager->UpdateConfiguredPrinter(*found);

  return true;
}

chromeos::Printer CreateTestPrinter(int index) {
  chromeos::Printer printer(PrinterId(index));
  printer.set_description("Description");
  printer.set_uri(base::StringPrintf("ipp://192.168.1.%d", index));

  return printer;
}

chromeos::SyncedPrintersManager* GetVerifierPrinterStore() {
  chromeos::SyncedPrintersManager* manager =
      GetPrinterStore(sync_datatype_helper::test()->verifier());

  return manager;
}

chromeos::SyncedPrintersManager* GetPrinterStore(int index) {
  chromeos::SyncedPrintersManager* manager =
      GetPrinterStore(sync_datatype_helper::test()->GetProfile(index));

  return manager;
}

int GetVerifierPrinterCount() {
  return GetVerifierPrinterStore()->GetConfiguredPrinters().size();
}

int GetPrinterCount(int index) {
  return GetPrinterStore(index)->GetConfiguredPrinters().size();
}

bool AllProfilesContainSamePrinters() {
  auto reference_printers = GetPrinterStore(0)->GetConfiguredPrinters();
  for (int i = 1; i < test()->num_clients(); ++i) {
    auto printers = GetPrinterStore(i)->GetConfiguredPrinters();
    if (!ListsContainTheSamePrinters(reference_printers, printers)) {
      VLOG(1) << "Printers in client [" << i << "] don't match client 0";
      return false;
    }
  }

  return true;
}

bool ProfileContainsSamePrintersAsVerifier(int index) {
  return ListsContainTheSamePrinters(
      GetVerifierPrinterStore()->GetConfiguredPrinters(),
      GetPrinterStore(index)->GetConfiguredPrinters());
}

PrintersMatchChecker::PrintersMatchChecker()
    : AwaitMatchStatusChangeChecker(
          base::Bind(&printers_helper::AllProfilesContainSamePrinters),
          "All printers match") {}

PrintersMatchChecker::~PrintersMatchChecker() {}

}  // namespace printers_helper
