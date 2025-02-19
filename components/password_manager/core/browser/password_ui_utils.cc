// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/password_manager/core/browser/password_ui_utils.h"

#include <algorithm>
#include <string>
#include <vector>

#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "components/autofill/core/common/password_form.h"
#include "components/password_manager/core/browser/android_affiliation/affiliation_utils.h"
#include "components/url_formatter/elide_url.h"

namespace password_manager {

namespace {

// The URL prefixes that are removed from shown origin.
const char* const kRemovedPrefixes[] = {"m.", "mobile.", "www."};

constexpr char kPlayStoreAppPrefix[] =
    "https://play.google.com/store/apps/details?id=";

}  // namespace

std::string SplitByDotAndReverse(base::StringPiece host) {
  std::vector<base::StringPiece> parts = base::SplitStringPiece(
      host, ".", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
  std::reverse(parts.begin(), parts.end());
  return base::JoinString(parts, ".");
}

std::pair<std::string, GURL> GetShownOriginAndLinkUrl(
    const autofill::PasswordForm& password_form) {
  std::string shown_origin;
  GURL link_url;

  FacetURI facet_uri =
      FacetURI::FromPotentiallyInvalidSpec(password_form.signon_realm);
  if (facet_uri.IsValidAndroidFacetURI()) {
    shown_origin = password_form.app_display_name.empty()
                       ? SplitByDotAndReverse(facet_uri.android_package_name())
                       : password_form.app_display_name;
    link_url = GURL(kPlayStoreAppPrefix + facet_uri.android_package_name());
  } else {
    shown_origin = GetShownOrigin(password_form.origin);
    link_url = password_form.origin;
  }

  return {std::move(shown_origin), std::move(link_url)};
}

std::string GetShownOrigin(const GURL& origin) {
  std::string original =
      base::UTF16ToUTF8(url_formatter::FormatUrlForSecurityDisplay(
          origin, url_formatter::SchemeDisplay::OMIT_HTTP_AND_HTTPS));
  base::StringPiece result = original;
  for (base::StringPiece prefix : kRemovedPrefixes) {
    if (base::StartsWith(result, prefix,
                         base::CompareCase::INSENSITIVE_ASCII)) {
      result.remove_prefix(prefix.length());
      break;  // Remove only one prefix (e.g. www.mobile.de).
    }
  }

  return result.find('.') != base::StringPiece::npos ? result.as_string()
                                                     : original;
}

}  // namespace password_manager
