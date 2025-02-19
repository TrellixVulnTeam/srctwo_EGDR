// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_PAGE_LOAD_STRATEGY_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_PAGE_LOAD_STRATEGY_H_

#include "chrome/test/chromedriver/chrome/status.h"

struct BrowserInfo;
class DevToolsClient;
class JavaScriptDialogManager;
class Status;
class Timeout;

class PageLoadStrategy {

public:
  enum LoadingState {
    kUnknown,
    kLoading,
    kNotLoading,
  };

  virtual ~PageLoadStrategy() {}

  static PageLoadStrategy* Create(
      std::string strategy,
      DevToolsClient* client,
      const BrowserInfo* browser_info,
      const JavaScriptDialogManager* dialog_manager);

  virtual Status IsPendingNavigation(const std::string& frame_id,
                                     const Timeout* timeout,
                                     bool* is_pending) = 0;

  virtual void set_timed_out(bool timed_out) = 0;

  // Types of page load strategies.
  static const char kNormal[];
  static const char kNone[];
  static const char kEager[];
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_PAGE_LOAD_STRATEGY_H_
