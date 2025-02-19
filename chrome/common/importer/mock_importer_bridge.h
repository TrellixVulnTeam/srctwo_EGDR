// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_IMPORTER_MOCK_IMPORTER_BRIDGE_H_
#define CHROME_COMMON_IMPORTER_MOCK_IMPORTER_BRIDGE_H_

#include <string>
#include <vector>

#include "chrome/common/importer/imported_bookmark_entry.h"
#include "chrome/common/importer/importer_autofill_form_data_entry.h"
#include "chrome/common/importer/importer_bridge.h"
#include "components/autofill/core/common/password_form.h"
#include "testing/gmock/include/gmock/gmock.h"

class MockImporterBridge : public ImporterBridge {
 public:
  MockImporterBridge();

  MOCK_METHOD2(AddBookmarks,
               void(const std::vector<ImportedBookmarkEntry>&,
                    const base::string16&));
  MOCK_METHOD1(AddHomePage, void(const GURL&));
#if defined(OS_WIN)
  MOCK_METHOD1(AddIE7PasswordInfo,
               void(const importer::ImporterIE7PasswordInfo&));
#endif
  MOCK_METHOD1(SetFavicons, void(const favicon_base::FaviconUsageDataList&));
  MOCK_METHOD2(SetHistoryItems,
               void(const std::vector<ImporterURLRow>&, importer::VisitSource));
  MOCK_METHOD2(SetKeywords,
               void(const std::vector<importer::SearchEngineInfo>&, bool));
  MOCK_METHOD1(SetFirefoxSearchEnginesXMLData,
               void(const std::vector<std::string>&));
  MOCK_METHOD1(SetPasswordForm, void(const autofill::PasswordForm&));
  MOCK_METHOD1(SetAutofillFormData,
               void(const std::vector<ImporterAutofillFormDataEntry>&));
  MOCK_METHOD0(NotifyStarted, void());
  MOCK_METHOD1(NotifyItemStarted, void(importer::ImportItem));
  MOCK_METHOD1(NotifyItemEnded, void(importer::ImportItem));
  MOCK_METHOD0(NotifyEnded, void());
  MOCK_METHOD1(GetLocalizedString, base::string16(int));

 private:
  ~MockImporterBridge() override;
};

#endif  // CHROME_COMMON_IMPORTER_MOCK_IMPORTER_BRIDGE_H_
