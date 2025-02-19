// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SEARCH_ENGINES_TEMPLATE_URL_PREPOPULATE_DATA_H_
#define COMPONENTS_SEARCH_ENGINES_TEMPLATE_URL_PREPOPULATE_DATA_H_

#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "components/search_engines/search_engine_type.h"

class GURL;
class PrefService;
struct TemplateURLData;

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace TemplateURLPrepopulateData {

struct PrepopulatedEngine;

extern const int kMaxPrepopulatedEngineID;

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

// Returns the current version of the prepopulate data, so callers can know when
// they need to re-merge. If the prepopulate data comes from the preferences
// file then it returns the version specified there.
int GetDataVersion(PrefService* prefs);

// Returns the prepopulated URLs for the current country.
// If |default_search_provider_index| is non-null, it is set to the index of the
// default search provider within the returned vector.
std::vector<std::unique_ptr<TemplateURLData>> GetPrepopulatedEngines(
    PrefService* prefs,
    size_t* default_search_provider_index);

// Returns the prepopulated search engine with the given |prepopulated_id|.
std::unique_ptr<TemplateURLData> GetPrepopulatedEngine(PrefService* prefs,
                                                       int prepopulated_id);

#if defined(OS_ANDROID)
// Returns the prepopulated URLs associated with |locale|.  |locale| should be a
// two-character uppercase ISO 3166-1 country code.
std::vector<std::unique_ptr<TemplateURLData>> GetLocalPrepopulatedEngines(
    const std::string& locale,
    PrefService* prefs);
#endif

// Returns all prepopulated engines for all locales. Used only by tests.
std::vector<const PrepopulatedEngine*> GetAllPrepopulatedEngines();

// Removes prepopulated engines and their version stored in user prefs.
void ClearPrepopulatedEnginesInPrefs(PrefService* prefs);

// Returns the default search provider specified by the prepopulate data, which
// may be NULL.
// If |prefs| is NULL, any search provider overrides from the preferences are
// not used.
std::unique_ptr<TemplateURLData> GetPrepopulatedDefaultSearch(
    PrefService* prefs);

// Like the above, but takes a GURL which is expected to represent a search URL.
// This may be called on any thread.
SearchEngineType GetEngineType(const GURL& url);

// Returns the identifier for the user current country. Used to update the list
// of search engines when user switches device region settings. For use on iOS
// only.
// TODO(ios): Once user can customize search engines ( http://crbug.com/153047 )
// this declaration should be removed and the definition in the .cc file be
// moved back to the anonymous namespace.
int GetCurrentCountryID();

}  // namespace TemplateURLPrepopulateData

#endif  // COMPONENTS_SEARCH_ENGINES_TEMPLATE_URL_PREPOPULATE_DATA_H_
