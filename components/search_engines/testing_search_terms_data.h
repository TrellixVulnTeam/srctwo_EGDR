// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SEARCH_ENGINES_TESTING_SEARCH_TERMS_DATA_H_
#define COMPONENTS_SEARCH_ENGINES_TESTING_SEARCH_TERMS_DATA_H_

#include "base/macros.h"
#include "components/search_engines/search_terms_data.h"

class TestingSearchTermsData : public SearchTermsData {
 public:
  explicit TestingSearchTermsData(const std::string& google_base_url);
  ~TestingSearchTermsData() override;

  std::string GoogleBaseURLValue() const override;
  base::string16 GetRlzParameterValue(bool from_app_list) const override;
  std::string GetSearchClient() const override;
  std::string GoogleImageSearchSource() const override;

  void set_google_base_url(const std::string& google_base_url) {
    google_base_url_ = google_base_url;
  }
  void set_search_client(const std::string& search_client) {
    search_client_ = search_client;
  }

 private:
  std::string google_base_url_;
  std::string search_client_;

  DISALLOW_COPY_AND_ASSIGN(TestingSearchTermsData);
};

#endif  // COMPONENTS_SEARCH_ENGINES_TESTING_SEARCH_TERMS_DATA_H_
