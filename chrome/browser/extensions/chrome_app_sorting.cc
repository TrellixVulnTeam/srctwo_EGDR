// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/chrome_app_sorting.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "base/macros.h"
#include "build/build_config.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/extensions/extension_sync_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/extensions/extension_constants.h"
#include "content/public/browser/notification_service.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_scoped_prefs.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"

#if defined(OS_CHROMEOS)
#include "chrome/browser/chromeos/extensions/default_app_order.h"
#endif

namespace extensions {

namespace {

// The number of apps per page. This isn't a hard limit, but new apps installed
// from the webstore will overflow onto a new page if this limit is reached.
const size_t kNaturalAppPageSize = 18;

// A preference determining the order of which the apps appear on the NTP.
const char kPrefAppLaunchIndexDeprecated[] = "app_launcher_index";
const char kPrefAppLaunchOrdinal[] = "app_launcher_ordinal";

// A preference determining the page on which an app appears in the NTP.
const char kPrefPageIndexDeprecated[] = "page_index";
const char kPrefPageOrdinal[] = "page_ordinal";

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// ChromeAppSorting::AppOrdinals

ChromeAppSorting::AppOrdinals::AppOrdinals() {}

ChromeAppSorting::AppOrdinals::AppOrdinals(const AppOrdinals& other) = default;

ChromeAppSorting::AppOrdinals::~AppOrdinals() {}

////////////////////////////////////////////////////////////////////////////////
// ChromeAppSorting

ChromeAppSorting::ChromeAppSorting(content::BrowserContext* browser_context)
    : browser_context_(browser_context),
      default_ordinals_created_(false) {
  ExtensionIdList extensions;
  ExtensionPrefs::Get(browser_context_)->GetExtensions(&extensions);
  InitializePageOrdinalMap(extensions);
  MigrateAppIndex(extensions);
}

ChromeAppSorting::~ChromeAppSorting() {
}

void ChromeAppSorting::CreateOrdinalsIfNecessary(size_t minimum_size) {
  // Create StringOrdinal values as required to ensure |ntp_ordinal_map_| has at
  // least |minimum_size| entries.
  if (ntp_ordinal_map_.empty() && minimum_size > 0)
    ntp_ordinal_map_[syncer::StringOrdinal::CreateInitialOrdinal()];

  while (ntp_ordinal_map_.size() < minimum_size) {
    syncer::StringOrdinal filler =
        ntp_ordinal_map_.rbegin()->first.CreateAfter();
    AppLaunchOrdinalMap empty_ordinal_map;
    ntp_ordinal_map_.insert(std::make_pair(filler, empty_ordinal_map));
  }
}

void ChromeAppSorting::MigrateAppIndex(
    const extensions::ExtensionIdList& extension_ids) {
  if (extension_ids.empty())
    return;

  ExtensionPrefs* prefs = ExtensionPrefs::Get(browser_context_);

  // Convert all the page index values to page ordinals. If there are any
  // app launch values that need to be migrated, inserted them into a sorted
  // set to be dealt with later.
  typedef std::map<syncer::StringOrdinal, std::map<int, const std::string*>,
                   syncer::StringOrdinal::LessThanFn> AppPositionToIdMapping;
  AppPositionToIdMapping app_launches_to_convert;
  for (extensions::ExtensionIdList::const_iterator ext_id =
           extension_ids.begin(); ext_id != extension_ids.end(); ++ext_id) {
    int old_page_index = 0;
    syncer::StringOrdinal page = GetPageOrdinal(*ext_id);
    if (prefs->ReadPrefAsInteger(*ext_id,
                                 kPrefPageIndexDeprecated,
                                 &old_page_index)) {
      // Some extensions have invalid page index, so we don't
      // attempt to convert them.
      if (old_page_index < 0) {
        DLOG(WARNING) << "Extension " << *ext_id
                      << " has an invalid page index " << old_page_index
                      << ". Aborting attempt to convert its index.";
        break;
      }

      CreateOrdinalsIfNecessary(static_cast<size_t>(old_page_index) + 1);

      page = PageIntegerAsStringOrdinal(old_page_index);
      SetPageOrdinal(*ext_id, page);
      prefs->UpdateExtensionPref(*ext_id, kPrefPageIndexDeprecated, nullptr);
    }

    int old_app_launch_index = 0;
    if (prefs->ReadPrefAsInteger(*ext_id,
                                 kPrefAppLaunchIndexDeprecated,
                                 &old_app_launch_index)) {
      // We can't update the app launch index value yet, because we use
      // GetNextAppLaunchOrdinal to get the new ordinal value and it requires
      // all the ordinals with lower values to have already been migrated.
      // A valid page ordinal is also required because otherwise there is
      // no page to add the app to.
      if (page.IsValid())
        app_launches_to_convert[page][old_app_launch_index] = &*ext_id;

      prefs->UpdateExtensionPref(*ext_id, kPrefAppLaunchIndexDeprecated,
                                 nullptr);
    }
  }

  // Remove any empty pages that may have been added. This shouldn't occur,
  // but double check here to prevent future problems with conversions between
  // integers and StringOrdinals.
  for (PageOrdinalMap::iterator it = ntp_ordinal_map_.begin();
       it != ntp_ordinal_map_.end();) {
    if (it->second.empty()) {
      PageOrdinalMap::iterator prev_it = it;
      ++it;
      ntp_ordinal_map_.erase(prev_it);
    } else {
      ++it;
    }
  }

  if (app_launches_to_convert.empty())
    return;

  // Create the new app launch ordinals and remove the old preferences. Since
  // the set is sorted, each time we migrate an apps index, we know that all of
  // the remaining apps will appear further down the NTP than it or on a
  // different page.
  for (AppPositionToIdMapping::const_iterator page_it =
           app_launches_to_convert.begin();
       page_it != app_launches_to_convert.end(); ++page_it) {
    syncer::StringOrdinal page = page_it->first;
    for (std::map<int, const std::string*>::const_iterator launch_it =
            page_it->second.begin(); launch_it != page_it->second.end();
        ++launch_it) {
      SetAppLaunchOrdinal(*(launch_it->second),
                          CreateNextAppLaunchOrdinal(page));
    }
  }
}

void ChromeAppSorting::FixNTPOrdinalCollisions() {
  for (PageOrdinalMap::iterator page_it = ntp_ordinal_map_.begin();
       page_it != ntp_ordinal_map_.end(); ++page_it) {
    AppLaunchOrdinalMap& page = page_it->second;

    AppLaunchOrdinalMap::iterator app_launch_it = page.begin();
    while (app_launch_it != page.end()) {
      int app_count = page.count(app_launch_it->first);
      if (app_count == 1) {
        ++app_launch_it;
        continue;
      }

      syncer::StringOrdinal repeated_ordinal = app_launch_it->first;

      // Sort the conflicting keys by their extension id, this is how
      // the order is decided.
      std::vector<std::string> conflicting_ids;
      for (int i = 0; i < app_count; ++i, ++app_launch_it)
        conflicting_ids.push_back(app_launch_it->second);
      std::sort(conflicting_ids.begin(), conflicting_ids.end());

      syncer::StringOrdinal upper_bound_ordinal = app_launch_it == page.end() ?
          syncer::StringOrdinal() :
          app_launch_it->first;
      syncer::StringOrdinal lower_bound_ordinal = repeated_ordinal;

      // Start at position 1 because the first extension can keep the conflicted
      // value.
      for (int i = 1; i < app_count; ++i) {
        syncer::StringOrdinal unique_app_launch;
        if (upper_bound_ordinal.IsValid()) {
          unique_app_launch =
              lower_bound_ordinal.CreateBetween(upper_bound_ordinal);
        } else {
          unique_app_launch = lower_bound_ordinal.CreateAfter();
        }

        SetAppLaunchOrdinal(conflicting_ids[i], unique_app_launch);
        lower_bound_ordinal = unique_app_launch;
      }
    }
  }

  content::NotificationService::current()->Notify(
      chrome::NOTIFICATION_APP_LAUNCHER_REORDERED,
      content::Source<ChromeAppSorting>(this),
      content::NotificationService::NoDetails());
}

void ChromeAppSorting::EnsureValidOrdinals(
    const std::string& extension_id,
    const syncer::StringOrdinal& suggested_page) {
  syncer::StringOrdinal page_ordinal = GetPageOrdinal(extension_id);
  if (!page_ordinal.IsValid()) {
    if (suggested_page.IsValid()) {
      page_ordinal = suggested_page;
    } else if (!GetDefaultOrdinals(extension_id, &page_ordinal, NULL) ||
        !page_ordinal.IsValid()) {
      page_ordinal = GetNaturalAppPageOrdinal();
    }

    SetPageOrdinal(extension_id, page_ordinal);
  }

  syncer::StringOrdinal app_launch_ordinal = GetAppLaunchOrdinal(extension_id);
  if (!app_launch_ordinal.IsValid()) {
    // If using default app launcher ordinal, make sure there is no collision.
    if (GetDefaultOrdinals(extension_id, NULL, &app_launch_ordinal) &&
        app_launch_ordinal.IsValid())
      app_launch_ordinal = ResolveCollision(page_ordinal, app_launch_ordinal);
    else
      app_launch_ordinal = CreateNextAppLaunchOrdinal(page_ordinal);

    SetAppLaunchOrdinal(extension_id, app_launch_ordinal);
  }
}

bool ChromeAppSorting::GetDefaultOrdinals(
    const std::string& extension_id,
    syncer::StringOrdinal* page_ordinal,
    syncer::StringOrdinal* app_launch_ordinal) {
  CreateDefaultOrdinals();
  AppOrdinalsMap::const_iterator it = default_ordinals_.find(extension_id);
  if (it == default_ordinals_.end())
    return false;

  if (page_ordinal)
    *page_ordinal = it->second.page_ordinal;
  if (app_launch_ordinal)
    *app_launch_ordinal = it->second.app_launch_ordinal;
  return true;
}

void ChromeAppSorting::OnExtensionMoved(
    const std::string& moved_extension_id,
    const std::string& predecessor_extension_id,
    const std::string& successor_extension_id) {
  // We only need to change the StringOrdinal if there are neighbours.
  if (!predecessor_extension_id.empty() || !successor_extension_id.empty()) {
    if (predecessor_extension_id.empty()) {
      // Only a successor.
      SetAppLaunchOrdinal(
          moved_extension_id,
          GetAppLaunchOrdinal(successor_extension_id).CreateBefore());
    } else if (successor_extension_id.empty()) {
      // Only a predecessor.
      SetAppLaunchOrdinal(
          moved_extension_id,
          GetAppLaunchOrdinal(predecessor_extension_id).CreateAfter());
    } else {
      // Both a successor and predecessor
      const syncer::StringOrdinal& predecessor_ordinal =
          GetAppLaunchOrdinal(predecessor_extension_id);
      const syncer::StringOrdinal& successor_ordinal =
          GetAppLaunchOrdinal(successor_extension_id);
      SetAppLaunchOrdinal(moved_extension_id,
                          predecessor_ordinal.CreateBetween(successor_ordinal));
    }
  }

  SyncIfNeeded(moved_extension_id);

  content::NotificationService::current()->Notify(
      chrome::NOTIFICATION_APP_LAUNCHER_REORDERED,
      content::Source<ChromeAppSorting>(this),
      content::Details<const std::string>(&moved_extension_id));
}


syncer::StringOrdinal ChromeAppSorting::GetAppLaunchOrdinal(
    const std::string& extension_id) const {
  std::string raw_value;
  // If the preference read fails then raw_value will still be unset and we
  // will return an invalid StringOrdinal to signal that no app launch ordinal
  // was found.
  ExtensionPrefs::Get(browser_context_)->ReadPrefAsString(
      extension_id, kPrefAppLaunchOrdinal, &raw_value);
  return syncer::StringOrdinal(raw_value);
}

void ChromeAppSorting::SetAppLaunchOrdinal(
    const std::string& extension_id,
    const syncer::StringOrdinal& new_app_launch_ordinal) {
  // No work is required if the old and new values are the same.
  if (new_app_launch_ordinal.EqualsOrBothInvalid(
          GetAppLaunchOrdinal(extension_id))) {
    return;
  }

  syncer::StringOrdinal page_ordinal = GetPageOrdinal(extension_id);
  RemoveOrdinalMapping(
      extension_id, page_ordinal, GetAppLaunchOrdinal(extension_id));
  AddOrdinalMapping(extension_id, page_ordinal, new_app_launch_ordinal);

  std::unique_ptr<base::Value> new_value =
      new_app_launch_ordinal.IsValid()
          ? base::MakeUnique<base::Value>(
                new_app_launch_ordinal.ToInternalValue())
          : nullptr;

  ExtensionPrefs::Get(browser_context_)
      ->UpdateExtensionPref(extension_id, kPrefAppLaunchOrdinal,
                            std::move(new_value));
  SyncIfNeeded(extension_id);
}

syncer::StringOrdinal ChromeAppSorting::CreateFirstAppLaunchOrdinal(
    const syncer::StringOrdinal& page_ordinal) const {
  const syncer::StringOrdinal& min_ordinal =
      GetMinOrMaxAppLaunchOrdinalsOnPage(page_ordinal,
                                         ChromeAppSorting::MIN_ORDINAL);

  if (min_ordinal.IsValid())
    return min_ordinal.CreateBefore();
  else
    return syncer::StringOrdinal::CreateInitialOrdinal();
}

syncer::StringOrdinal ChromeAppSorting::CreateNextAppLaunchOrdinal(
    const syncer::StringOrdinal& page_ordinal) const {
  const syncer::StringOrdinal& max_ordinal =
      GetMinOrMaxAppLaunchOrdinalsOnPage(page_ordinal,
                                         ChromeAppSorting::MAX_ORDINAL);

  if (max_ordinal.IsValid())
    return max_ordinal.CreateAfter();
  else
    return syncer::StringOrdinal::CreateInitialOrdinal();
}

syncer::StringOrdinal ChromeAppSorting::CreateFirstAppPageOrdinal() const {
  if (ntp_ordinal_map_.empty())
    return syncer::StringOrdinal::CreateInitialOrdinal();

  return ntp_ordinal_map_.begin()->first;
}

syncer::StringOrdinal ChromeAppSorting::GetNaturalAppPageOrdinal() const {
  if (ntp_ordinal_map_.empty())
    return syncer::StringOrdinal::CreateInitialOrdinal();

  for (PageOrdinalMap::const_iterator it = ntp_ordinal_map_.begin();
       it != ntp_ordinal_map_.end(); ++it) {
    if (CountItemsVisibleOnNtp(it->second) < kNaturalAppPageSize)
      return it->first;
  }

  // Add a new page as all existing pages are full.
  syncer::StringOrdinal last_element = ntp_ordinal_map_.rbegin()->first;
  return last_element.CreateAfter();
}

syncer::StringOrdinal ChromeAppSorting::GetPageOrdinal(
    const std::string& extension_id) const {
  std::string raw_data;
  // If the preference read fails then raw_data will still be unset and we will
  // return an invalid StringOrdinal to signal that no page ordinal was found.
  ExtensionPrefs::Get(browser_context_)->ReadPrefAsString(
      extension_id, kPrefPageOrdinal, &raw_data);
  return syncer::StringOrdinal(raw_data);
}

void ChromeAppSorting::SetPageOrdinal(
    const std::string& extension_id,
    const syncer::StringOrdinal& new_page_ordinal) {
  // No work is required if the old and new values are the same.
  if (new_page_ordinal.EqualsOrBothInvalid(GetPageOrdinal(extension_id)))
    return;

  syncer::StringOrdinal app_launch_ordinal = GetAppLaunchOrdinal(extension_id);
  RemoveOrdinalMapping(
      extension_id, GetPageOrdinal(extension_id), app_launch_ordinal);
  AddOrdinalMapping(extension_id, new_page_ordinal, app_launch_ordinal);

  std::unique_ptr<base::Value> new_value =
      new_page_ordinal.IsValid()
          ? base::MakeUnique<base::Value>(new_page_ordinal.ToInternalValue())
          : nullptr;

  ExtensionPrefs::Get(browser_context_)
      ->UpdateExtensionPref(extension_id, kPrefPageOrdinal,
                            std::move(new_value));
  SyncIfNeeded(extension_id);
}

void ChromeAppSorting::ClearOrdinals(const std::string& extension_id) {
  RemoveOrdinalMapping(extension_id,
                       GetPageOrdinal(extension_id),
                       GetAppLaunchOrdinal(extension_id));

  ExtensionPrefs* prefs = ExtensionPrefs::Get(browser_context_);
  prefs->UpdateExtensionPref(extension_id, kPrefPageOrdinal, nullptr);
  prefs->UpdateExtensionPref(extension_id, kPrefAppLaunchOrdinal, nullptr);
}

int ChromeAppSorting::PageStringOrdinalAsInteger(
    const syncer::StringOrdinal& page_ordinal) const {
  if (!page_ordinal.IsValid())
    return -1;

  PageOrdinalMap::const_iterator it = ntp_ordinal_map_.find(page_ordinal);
  return it != ntp_ordinal_map_.end() ?
      std::distance(ntp_ordinal_map_.begin(), it) : -1;
}

syncer::StringOrdinal ChromeAppSorting::PageIntegerAsStringOrdinal(
    size_t page_index) {
  if (page_index < ntp_ordinal_map_.size()) {
    PageOrdinalMap::const_iterator it = ntp_ordinal_map_.begin();
    std::advance(it, page_index);
    return it->first;
  }

  CreateOrdinalsIfNecessary(page_index + 1);
  return ntp_ordinal_map_.rbegin()->first;
}

void ChromeAppSorting::SetExtensionVisible(const std::string& extension_id,
                                           bool visible) {
  if (visible)
    ntp_hidden_extensions_.erase(extension_id);
  else
    ntp_hidden_extensions_.insert(extension_id);
}

syncer::StringOrdinal ChromeAppSorting::GetMinOrMaxAppLaunchOrdinalsOnPage(
    const syncer::StringOrdinal& target_page_ordinal,
    AppLaunchOrdinalReturn return_type) const {
  CHECK(target_page_ordinal.IsValid());

  syncer::StringOrdinal return_value;

  PageOrdinalMap::const_iterator page =
      ntp_ordinal_map_.find(target_page_ordinal);
  if (page != ntp_ordinal_map_.end()) {
    const AppLaunchOrdinalMap& app_list = page->second;

    if (app_list.empty())
      return syncer::StringOrdinal();

    if (return_type == ChromeAppSorting::MAX_ORDINAL)
      return_value = app_list.rbegin()->first;
    else if (return_type == ChromeAppSorting::MIN_ORDINAL)
      return_value = app_list.begin()->first;
  }

  return return_value;
}

void ChromeAppSorting::InitializePageOrdinalMap(
    const extensions::ExtensionIdList& extension_ids) {
  for (extensions::ExtensionIdList::const_iterator ext_it =
           extension_ids.begin(); ext_it != extension_ids.end(); ++ext_it) {
    AddOrdinalMapping(*ext_it,
                      GetPageOrdinal(*ext_it),
                      GetAppLaunchOrdinal(*ext_it));

    // Ensure that the web store app still isn't found in this list, since
    // it is added after this loop.
    DCHECK(*ext_it != extensions::kWebStoreAppId);
    DCHECK(*ext_it != extension_misc::kChromeAppId);
  }

  // Include the Web Store App since it is displayed on the NTP.
  syncer::StringOrdinal web_store_app_page =
      GetPageOrdinal(extensions::kWebStoreAppId);
  if (web_store_app_page.IsValid()) {
    AddOrdinalMapping(extensions::kWebStoreAppId,
                      web_store_app_page,
                      GetAppLaunchOrdinal(extensions::kWebStoreAppId));
  }
  // Include the Chrome App since it is displayed in the app launcher.
  syncer::StringOrdinal chrome_app_page =
      GetPageOrdinal(extension_misc::kChromeAppId);
  if (chrome_app_page.IsValid()) {
    AddOrdinalMapping(extension_misc::kChromeAppId,
                      chrome_app_page,
                      GetAppLaunchOrdinal(extension_misc::kChromeAppId));
  }
}

void ChromeAppSorting::AddOrdinalMapping(
    const std::string& extension_id,
    const syncer::StringOrdinal& page_ordinal,
    const syncer::StringOrdinal& app_launch_ordinal) {
  if (!page_ordinal.IsValid() || !app_launch_ordinal.IsValid())
    return;

  ntp_ordinal_map_[page_ordinal].insert(
      std::make_pair(app_launch_ordinal, extension_id));
}

void ChromeAppSorting::RemoveOrdinalMapping(
    const std::string& extension_id,
    const syncer::StringOrdinal& page_ordinal,
    const syncer::StringOrdinal& app_launch_ordinal) {
  if (!page_ordinal.IsValid() || !app_launch_ordinal.IsValid())
    return;

  // Check that the page exists using find to prevent creating a new page
  // if |page_ordinal| isn't a used page.
  PageOrdinalMap::iterator page_map = ntp_ordinal_map_.find(page_ordinal);
  if (page_map == ntp_ordinal_map_.end())
    return;

  for (AppLaunchOrdinalMap::iterator it =
           page_map->second.find(app_launch_ordinal);
       it != page_map->second.end(); ++it) {
    if (it->second == extension_id) {
      page_map->second.erase(it);
      break;
    }
  }
}

void ChromeAppSorting::SyncIfNeeded(const std::string& extension_id) {
  // Can be null in tests.
  if (!browser_context_)
    return;

  ExtensionRegistry* registry = ExtensionRegistry::Get(browser_context_);
  const Extension* extension = registry->GetInstalledExtension(extension_id);
  if (extension) {
    Profile* profile = Profile::FromBrowserContext(browser_context_);
    ExtensionSyncService::Get(profile)->SyncExtensionChangeIfNeeded(*extension);
  }
}

void ChromeAppSorting::CreateDefaultOrdinals() {
  if (default_ordinals_created_)
    return;
  default_ordinals_created_ = true;

  // The following defines the default order of apps.
#if defined(OS_CHROMEOS)
  std::vector<std::string> app_ids;
  chromeos::default_app_order::Get(&app_ids);
#else
  const char* const kDefaultAppOrder[] = {
    extension_misc::kChromeAppId,
    extensions::kWebStoreAppId,
  };
  const std::vector<const char*> app_ids(
      kDefaultAppOrder, kDefaultAppOrder + arraysize(kDefaultAppOrder));
#endif

  syncer::StringOrdinal page_ordinal = CreateFirstAppPageOrdinal();
  syncer::StringOrdinal app_launch_ordinal =
      CreateFirstAppLaunchOrdinal(page_ordinal);
  for (size_t i = 0; i < app_ids.size(); ++i) {
    const std::string extension_id = app_ids[i];
    default_ordinals_[extension_id].page_ordinal = page_ordinal;
    default_ordinals_[extension_id].app_launch_ordinal = app_launch_ordinal;
    app_launch_ordinal = app_launch_ordinal.CreateAfter();
  }
}

syncer::StringOrdinal ChromeAppSorting::ResolveCollision(
    const syncer::StringOrdinal& page_ordinal,
    const syncer::StringOrdinal& app_launch_ordinal) const {
  DCHECK(page_ordinal.IsValid() && app_launch_ordinal.IsValid());

  PageOrdinalMap::const_iterator page_it = ntp_ordinal_map_.find(page_ordinal);
  if (page_it == ntp_ordinal_map_.end())
    return app_launch_ordinal;

  const AppLaunchOrdinalMap& page = page_it->second;
  AppLaunchOrdinalMap::const_iterator app_it = page.find(app_launch_ordinal);
  if (app_it == page.end())
    return app_launch_ordinal;

  // Finds the next app launcher ordinal. This is done by the following loop
  // because this function could be called before FixNTPOrdinalCollisions and
  // thus |page| might contains multiple entries with the same app launch
  // ordinal. See http://crbug.com/155603
  while (app_it != page.end() && app_launch_ordinal.Equals(app_it->first))
    ++app_it;

  // If there is no next after the collision, returns the next ordinal.
  if (app_it == page.end())
    return app_launch_ordinal.CreateAfter();

  // Otherwise, returns the ordinal between the collision and the next ordinal.
  return app_launch_ordinal.CreateBetween(app_it->first);
}

size_t ChromeAppSorting::CountItemsVisibleOnNtp(
    const AppLaunchOrdinalMap& m) const {
  size_t result = 0;
  for (AppLaunchOrdinalMap::const_iterator it = m.begin(); it != m.end();
       ++it) {
    const std::string& id = it->second;
    if (ntp_hidden_extensions_.count(id) == 0)
      result++;
  }
  return result;
}

}  // namespace extensions
