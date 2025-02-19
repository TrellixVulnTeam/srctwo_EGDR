// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_NTP_SNIPPETS_OFFLINE_PAGES_OFFLINE_PAGES_TEST_UTILS_H_
#define COMPONENTS_NTP_SNIPPETS_OFFLINE_PAGES_OFFLINE_PAGES_TEST_UTILS_H_

#include "components/ntp_snippets/content_suggestion.h"
#include "components/offline_pages/core/offline_page_item.h"
#include "components/offline_pages/core/stub_offline_page_model.h"

namespace ntp_snippets {
namespace test {

class FakeOfflinePageModel : public offline_pages::StubOfflinePageModel {
 public:
  FakeOfflinePageModel();
  ~FakeOfflinePageModel() override;

  void GetPagesMatchingQuery(
      std::unique_ptr<offline_pages::OfflinePageModelQuery> query,
      const offline_pages::MultipleOfflinePageItemCallback& callback) override;

  void GetAllPages(
      const offline_pages::MultipleOfflinePageItemCallback& callback) override;

  const std::vector<offline_pages::OfflinePageItem>& items();
  std::vector<offline_pages::OfflinePageItem>* mutable_items();

  bool is_loaded() const override;
  void set_is_loaded(bool value);

 private:
  std::vector<offline_pages::OfflinePageItem> items_;
  bool is_loaded_;

  DISALLOW_COPY_AND_ASSIGN(FakeOfflinePageModel);
};

offline_pages::OfflinePageItem CreateDummyOfflinePageItem(
    int id,
    const offline_pages::ClientId& client_id);

offline_pages::OfflinePageItem CreateDummyOfflinePageItem(
    int id,
    const std::string& name_space);

void CaptureDismissedSuggestions(
    std::vector<ntp_snippets::ContentSuggestion>* captured_suggestions,
    std::vector<ntp_snippets::ContentSuggestion> dismissed_suggestions);

}  // namespace test
}  // namespace ntp_snippets

#endif  // COMPONENTS_NTP_SNIPPETS_OFFLINE_PAGES_OFFLINE_PAGES_TEST_UTILS_H_
