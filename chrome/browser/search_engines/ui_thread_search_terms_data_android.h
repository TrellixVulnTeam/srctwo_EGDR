// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SEARCH_ENGINES_UI_THREAD_SEARCH_TERMS_DATA_ANDROID_H_
#define CHROME_BROWSER_SEARCH_ENGINES_UI_THREAD_SEARCH_TERMS_DATA_ANDROID_H_

#include <string>

#include "base/lazy_instance.h"
#include "base/strings/string16.h"

// Additional data needed by TemplateURLRef::ReplaceSearchTerms on Android.
struct SearchTermsDataAndroid {
  static base::LazyInstance<base::string16>::Leaky rlz_parameter_value_;
  static base::LazyInstance<std::string>::Leaky search_client_;
};

#endif  // CHROME_BROWSER_SEARCH_ENGINES_UI_THREAD_SEARCH_TERMS_DATA_ANDROID_H_
