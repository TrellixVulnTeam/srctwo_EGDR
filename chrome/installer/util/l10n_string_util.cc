// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file defines utility functions for fetching localized resources.

#include "chrome/installer/util/l10n_string_util.h"

#include <atlbase.h>
#include <stdint.h>

#include <algorithm>
#include <limits>
#include <memory>

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "chrome/install_static/install_details.h"
#include "chrome/install_static/install_modes.h"
#include "chrome/installer/util/installer_util_strings.h"
#include "chrome/installer/util/language_selector.h"

namespace {

const installer::LanguageSelector& GetLanguageSelector() {
  static const installer::LanguageSelector instance;

  return instance;
}

installer::TranslationDelegate* g_translation_delegate = NULL;

}  // namespace

namespace installer {

TranslationDelegate::~TranslationDelegate() {
}

void SetTranslationDelegate(TranslationDelegate* delegate) {
  g_translation_delegate = delegate;
}

std::wstring GetLocalizedString(int base_message_id) {
  // Map |base_message_id| to the base id for the current install mode.
  base_message_id = GetBaseMessageIdForMode(base_message_id);

  if (g_translation_delegate)
    return g_translation_delegate->GetLocalizedString(base_message_id);

  std::wstring localized_string;

  int message_id = base_message_id + GetLanguageSelector().offset();
  const ATLSTRINGRESOURCEIMAGE* image = AtlGetStringResourceImage(
      _AtlBaseModule.GetModuleInstance(), message_id);
  if (image) {
    localized_string = std::wstring(image->achString, image->nLength);
  } else {
    NOTREACHED() << "Unable to find resource id " << message_id;
  }

  return localized_string;
}

base::string16 GetLocalizedStringF(int base_message_id,
                                   const base::string16& a) {
  return base::ReplaceStringPlaceholders(
      GetLocalizedString(base_message_id),
      std::vector<base::string16>(1, a),
      NULL);
}

// Here we generate the url spec with the Microsoft res:// scheme which is
// explained here : http://support.microsoft.com/kb/220830
std::wstring GetLocalizedEulaResource() {
  wchar_t full_exe_path[MAX_PATH];
  int len = ::GetModuleFileName(NULL, full_exe_path, MAX_PATH);
  if (len == 0 || len == MAX_PATH)
    return L"";

  // The resource names are more or less the upcased language names.
  base::string16 language(GetLanguageSelector().selected_translation());
  std::replace(language.begin(), language.end(), L'-', L'_');
  language = base::ToUpperASCII(language);

  std::wstring resource(L"IDR_OEMPG_");
  resource.append(language).append(L".HTML");

  // Fall back on "en" if we don't have a resource for this language.
  if (NULL == FindResource(NULL, resource.c_str(), RT_HTML))
    resource = L"IDR_OEMPG_EN.HTML";

  // Spaces and DOS paths must be url encoded.
  std::wstring url_path =
      base::StringPrintf(L"res://%ls/#23/%ls", full_exe_path, resource.c_str());

  // The cast is safe because url_path has limited length
  // (see the definition of full_exe_path and resource).
  DCHECK(std::numeric_limits<uint32_t>::max() > (url_path.size() * 3));
  DWORD count = static_cast<DWORD>(url_path.size() * 3);
  std::unique_ptr<wchar_t[]> url_canon(new wchar_t[count]);
  HRESULT hr = ::UrlCanonicalizeW(url_path.c_str(), url_canon.get(),
                                  &count, URL_ESCAPE_UNSAFE);
  if (SUCCEEDED(hr))
    return std::wstring(url_canon.get());
  return url_path;
}

std::wstring GetCurrentTranslation() {
  return GetLanguageSelector().selected_translation();
}

int GetBaseMessageIdForMode(int base_message_id) {
// Generate the constants holding the mode-specific resource ID arrays.
#define HANDLE_MODE_STRING(id, ...)                                   \
  static constexpr int k##id##Strings[] = {__VA_ARGS__};              \
  static_assert(                                                      \
      arraysize(k##id##Strings) == install_static::NUM_INSTALL_MODES, \
      "resource " #id                                                 \
      " has the wrong number of mode-specific "                       \
      "strings.");
  DO_MODE_STRINGS
#undef HANDLE_MODE_STRING

  const int* mode_strings = nullptr;
  switch (base_message_id) {
// Generate the cases mapping each mode-specific resource ID to its array.
#define HANDLE_MODE_STRING(id, ...)    \
  case id:                             \
    mode_strings = &k##id##Strings[0]; \
    break;
    DO_MODE_STRINGS
#undef HANDLE_MODE_STRING
    default:
      // This ID has no per-mode variants.
      return base_message_id;
  }

  // Return the variant of |base_message_id| for the current mode.
  return mode_strings[install_static::InstallDetails::Get()
                          .install_mode_index()];
}

}  // namespace installer
