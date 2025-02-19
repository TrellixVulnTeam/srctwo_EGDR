// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_IMPORTER_IN_PROCESS_IMPORTER_BRIDGE_H_
#define CHROME_BROWSER_IMPORTER_IN_PROCESS_IMPORTER_BRIDGE_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "build/build_config.h"
#include "chrome/browser/importer/profile_writer.h"
#include "chrome/common/importer/importer_bridge.h"

class GURL;
struct ImportedBookmarkEntry;
class ExternalProcessImporterHost;

namespace importer {
#if defined(OS_WIN)
struct ImporterIE7PasswordInfo;
#endif
struct SearchEngineInfo;
}

class InProcessImporterBridge : public ImporterBridge {
 public:
  InProcessImporterBridge(ProfileWriter* writer,
                          base::WeakPtr<ExternalProcessImporterHost> host);

  // Begin ImporterBridge implementation:
  void AddBookmarks(const std::vector<ImportedBookmarkEntry>& bookmarks,
                    const base::string16& first_folder_name) override;

  void AddHomePage(const GURL& home_page) override;

#if defined(OS_WIN)
  void AddIE7PasswordInfo(
      const importer::ImporterIE7PasswordInfo& password_info) override;
#endif

  void SetFavicons(const favicon_base::FaviconUsageDataList& favicons) override;

  void SetHistoryItems(const std::vector<ImporterURLRow>& rows,
                       importer::VisitSource visit_source) override;

  void SetKeywords(
      const std::vector<importer::SearchEngineInfo>& search_engines,
      bool unique_on_host_and_path) override;

  void SetFirefoxSearchEnginesXMLData(
      const std::vector<std::string>& search_engine_data) override;

  void SetPasswordForm(const autofill::PasswordForm& form) override;

  void SetAutofillFormData(
      const std::vector<ImporterAutofillFormDataEntry>& entries) override;

  void NotifyStarted() override;
  void NotifyItemStarted(importer::ImportItem item) override;
  void NotifyItemEnded(importer::ImportItem item) override;
  void NotifyEnded() override;

  base::string16 GetLocalizedString(int message_id) override;
  // End ImporterBridge implementation.

 private:
  ~InProcessImporterBridge() override;

  ProfileWriter* const writer_;  // weak
  const base::WeakPtr<ExternalProcessImporterHost> host_;

  DISALLOW_COPY_AND_ASSIGN(InProcessImporterBridge);
};

#endif  // CHROME_BROWSER_IMPORTER_IN_PROCESS_IMPORTER_BRIDGE_H_
