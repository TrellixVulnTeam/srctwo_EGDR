// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OMNIBOX_BROWSER_OMNIBOX_LOG_H_
#define COMPONENTS_OMNIBOX_BROWSER_OMNIBOX_LOG_H_

#include <stddef.h>

#include "base/strings/string16.h"
#include "base/time/time.h"
#include "components/metrics/proto/omnibox_event.pb.h"
#include "components/metrics/proto/omnibox_input_type.pb.h"
#include "components/omnibox/browser/autocomplete_provider.h"
#include "components/sessions/core/session_id.h"

class AutocompleteResult;

// The data to log (via the metrics service) when the user selects an item from
// the omnibox popup.
struct OmniboxLog {
  OmniboxLog(const base::string16& text,
             bool just_deleted_text,
             metrics::OmniboxInputType input_type,
             bool is_popup_open,
             size_t selected_index,
             bool is_paste_and_go,
             SessionID::id_type tab_id,
             metrics::OmniboxEventProto::PageClassification
                 current_page_classification,
             base::TimeDelta elapsed_time_since_user_first_modified_omnibox,
             size_t completed_length,
             base::TimeDelta elapsed_time_since_last_change_to_default_match,
             const AutocompleteResult& result);
  ~OmniboxLog();

  // The user's input text in the omnibox.
  base::string16 text;

  // Whether the user deleted text immediately before selecting an omnibox
  // suggestion.  This is usually the result of pressing backspace or delete.
  bool just_deleted_text;

  // The detected type of the user's input.
  metrics::OmniboxInputType input_type;

  // True if the popup is open.
  bool is_popup_open;

  // The index of the item selected in the dropdown list.  Set to 0 if the
  // dropdown is closed (and therefore there is only one implicit suggestion).
  size_t selected_index;

  // True if this is a paste-and-search or paste-and-go omnibox interaction.
  // (The codebase refers to both these types as paste-and-go.)
  bool is_paste_and_go;

  // ID of the tab the selected autocomplete suggestion was opened in.
  // Set to -1 if we haven't yet determined the destination tab.
  SessionID::id_type tab_id;

  // The type of page (e.g., new tab page, regular web page) that the
  // user was viewing before going somewhere with the omnibox.
  metrics::OmniboxEventProto::PageClassification current_page_classification;

  // The amount of time since the user first began modifying the text
  // in the omnibox.  If at some point after modifying the text, the
  // user reverts the modifications (thus seeing the current web
  // page's URL again), then writes in the omnibox again, this time
  // delta should be computed starting from the second series of
  // modifications.  If we somehow skipped the logic to record
  // the time the user began typing (this should only happen in
  // unit tests), this elapsed time is set to -1 milliseconds.
  base::TimeDelta elapsed_time_since_user_first_modified_omnibox;

  // The number of extra characters the user would have to manually type
  // if they were not given the opportunity to select this match.  Only
  // set for matches that are allowed to be the default match (i.e., are
  // inlineable).  Set to base::string16::npos if the match is not allowed
  // to be the default match.
  size_t completed_length;

  // The amount of time since the last time the default (i.e., inline)
  // match changed.  This will certainly be less than
  // elapsed_time_since_user_first_modified_omnibox.  Measuring this
  // may be inappropriate in some cases (e.g., if editing is not in
  // progress).  In such cases, it's set to -1 milliseconds.
  base::TimeDelta elapsed_time_since_last_change_to_default_match;

  // Result set.
  const AutocompleteResult& result;

  // Diagnostic information from providers.  See
  // AutocompleteController::AddProvidersInfo() and
  // AutocompleteProvider::AddProviderInfo() above.
  ProvidersInfo providers_info;
};

#endif  // COMPONENTS_OMNIBOX_BROWSER_OMNIBOX_LOG_H_
