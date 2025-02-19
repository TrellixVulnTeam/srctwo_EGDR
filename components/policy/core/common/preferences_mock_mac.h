// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_POLICY_CORE_COMMON_PREFERENCES_MOCK_MAC_H_
#define COMPONENTS_POLICY_CORE_COMMON_PREFERENCES_MOCK_MAC_H_

#include "base/mac/scoped_cftyperef.h"
#include "components/policy/core/common/preferences_mac.h"
#include "components/policy/policy_export.h"

// Mock preferences wrapper for testing code that interacts with CFPreferences.
class POLICY_EXPORT MockPreferences : public MacPreferences {
 public:
  MockPreferences();
  ~MockPreferences() override;

  Boolean AppSynchronize(CFStringRef applicationID) override;

  CFPropertyListRef CopyAppValue(CFStringRef key,
                                 CFStringRef applicationID) override;

  Boolean AppValueIsForced(CFStringRef key, CFStringRef applicationID) override;

  // Adds a preference item with the given info to the test set.
  void AddTestItem(CFStringRef key, CFPropertyListRef value, bool is_forced);

 private:
  base::ScopedCFTypeRef<CFMutableDictionaryRef> values_;
  base::ScopedCFTypeRef<CFMutableSetRef> forced_;
};

#endif  // COMPONENTS_POLICY_CORE_COMMON_PREFERENCES_MOCK_MAC_H_
