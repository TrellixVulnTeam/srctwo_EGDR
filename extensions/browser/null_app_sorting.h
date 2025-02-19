// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_NULL_APP_SORTING_H_
#define EXTENSIONS_BROWSER_NULL_APP_SORTING_H_

#include <stddef.h>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "extensions/browser/app_sorting.h"

namespace extensions {

// An AppSorting that doesn't provide any ordering.
class NullAppSorting : public AppSorting {
 public:
  NullAppSorting();
  ~NullAppSorting() override;

  // AppSorting overrides:
  void FixNTPOrdinalCollisions() override;
  void EnsureValidOrdinals(
      const std::string& extension_id,
      const syncer::StringOrdinal& suggested_page) override;
  bool GetDefaultOrdinals(const std::string& extension_id,
                          syncer::StringOrdinal* page_ordinal,
                          syncer::StringOrdinal* app_launch_ordinal) override;
  void OnExtensionMoved(const std::string& moved_extension_id,
                        const std::string& predecessor_extension_id,
                        const std::string& successor_extension_id) override;
  syncer::StringOrdinal GetAppLaunchOrdinal(
      const std::string& extension_id) const override;
  void SetAppLaunchOrdinal(
      const std::string& extension_id,
      const syncer::StringOrdinal& new_app_launch_ordinal) override;
  syncer::StringOrdinal CreateFirstAppLaunchOrdinal(
      const syncer::StringOrdinal& page_ordinal) const override;
  syncer::StringOrdinal CreateNextAppLaunchOrdinal(
      const syncer::StringOrdinal& page_ordinal) const override;
  syncer::StringOrdinal CreateFirstAppPageOrdinal() const override;
  syncer::StringOrdinal GetNaturalAppPageOrdinal() const override;
  syncer::StringOrdinal GetPageOrdinal(
      const std::string& extension_id) const override;
  void SetPageOrdinal(const std::string& extension_id,
                      const syncer::StringOrdinal& new_page_ordinal) override;
  void ClearOrdinals(const std::string& extension_id) override;
  int PageStringOrdinalAsInteger(
      const syncer::StringOrdinal& page_ordinal) const override;
  syncer::StringOrdinal PageIntegerAsStringOrdinal(size_t page_index) override;
  void SetExtensionVisible(const std::string& extension_id,
                           bool visible) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(NullAppSorting);
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_NULL_APP_SORTING_H_
