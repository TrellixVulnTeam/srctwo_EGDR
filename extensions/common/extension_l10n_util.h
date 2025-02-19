// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file declares extension specific l10n utils.

#ifndef EXTENSIONS_COMMON_EXTENSION_L10N_UTIL_H_
#define EXTENSIONS_COMMON_EXTENSION_L10N_UTIL_H_

#include <set>
#include <string>
#include <vector>

namespace base {
class DictionaryValue;
class FilePath;
}

namespace extensions {
class MessageBundle;
}

namespace extension_l10n_util {

// Set the locale for this process to a fixed value, rather than using the
// normal file-based lookup mechanisms. This is used to set the locale inside
// the sandboxed utility process, where file reading is not allowed.
void SetProcessLocale(const std::string& locale);

// Returns default locale in form "en-US" or "sr" or empty string if
// "default_locale" section was not defined in the manifest.json file.
std::string GetDefaultLocaleFromManifest(const base::DictionaryValue& manifest,
                                         std::string* error);

// Returns true iff the extension was localized, and the current locale
// doesn't match the locale written into info.extension_manifest.
bool ShouldRelocalizeManifest(const base::DictionaryValue* manifest);

// Localize extension name, description, browser_action and other fields
// in the manifest.
bool LocalizeManifest(const extensions::MessageBundle& messages,
                      base::DictionaryValue* manifest,
                      std::string* error);

// Load message catalogs, localize manifest and attach message bundle to the
// extension.
bool LocalizeExtension(const base::FilePath& extension_path,
                       base::DictionaryValue* manifest,
                       std::string* error);

// Adds locale_name to the extension if it's in chrome_locales, and
// if messages file is present (we don't check content of messages file here).
// Returns false if locale_name was not found in chrome_locales, and sets
// error with locale_name.
// If file name starts with . return true (helps testing extensions under svn).
bool AddLocale(const std::set<std::string>& chrome_locales,
               const base::FilePath& locale_folder,
               const std::string& locale_name,
               std::set<std::string>* valid_locales,
               std::string* error);

// Returns normalized current locale, or default locale - en_US.
std::string CurrentLocaleOrDefault();

// Extends list of Chrome locales to them and their parents, so we can do
// proper fallback.
void GetAllLocales(std::set<std::string>* all_locales);

// Provides a vector of all fallback locales for message localization.
// The vector is ordered by priority of locale - |application_locale|,
// first_parent, ..., |default_locale|.
void GetAllFallbackLocales(const std::string& application_locale,
                           const std::string& default_locale,
                           std::vector<std::string>* all_fallback_locales);

// Fill |valid_locales| with all valid locales under |locale_path|.
// |valid_locales| is the intersection of the set of locales supported by
// Chrome and the set of locales specified by |locale_path|.
// Returns true if vaild_locales contains at least one locale, false otherwise.
// |error| contains an error message when a locale is corrupt or missing.
bool GetValidLocales(const base::FilePath& locale_path,
                     std::set<std::string>* valid_locales,
                     std::string* error);

// Loads messages file for default locale, and application locales (application
// locales doesn't have to exist). Application locale is current locale and its
// parents.
// Returns message bundle if it can load default locale messages file, and all
// messages are valid, else returns NULL and sets error.
extensions::MessageBundle* LoadMessageCatalogs(
    const base::FilePath& locale_path,
    const std::string& default_locale,
    const std::string& app_locale,
    std::string* error);

// Loads message catalogs for all locales to check for validity.
bool ValidateExtensionLocales(const base::FilePath& extension_path,
                              const base::DictionaryValue* manifest,
                              std::string* error);

// Returns true if directory has "." in the name (for .svn) or if it doesn't
// belong to Chrome locales.
// |locales_path| is extension_id/_locales
// |locale_path| is extension_id/_locales/xx
// |all_locales| is a set of all valid Chrome locales.
bool ShouldSkipValidation(const base::FilePath& locales_path,
                          const base::FilePath& locale_path,
                          const std::set<std::string>& all_locales);

// Sets the process locale for the duration of the current scope, then reverts
// back to whatever the current locale was before constructing this.
// For testing purposed only!
class ScopedLocaleForTest {
 public:
  // Only revert back to current locale at end of scope, don't set locale.
  ScopedLocaleForTest();

  // Set temporary locale for the current scope
  explicit ScopedLocaleForTest(const std::string& locale);

  ~ScopedLocaleForTest();

 private:
  std::string locale_;  // The current locale at ctor time.
};

}  // namespace extension_l10n_util

#endif  // EXTENSIONS_COMMON_EXTENSION_L10N_UTIL_H_
