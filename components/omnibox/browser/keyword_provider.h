// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file contains the keyword autocomplete provider. The keyword provider
// is responsible for remembering/suggesting user "search keyword queries"
// (e.g.  "imdb Godzilla") and then fixing them up into valid URLs.  An
// instance of it gets created and managed by the autocomplete controller.
// KeywordProvider uses a TemplateURLService to find the set of keywords.

#ifndef COMPONENTS_OMNIBOX_BROWSER_KEYWORD_PROVIDER_H_
#define COMPONENTS_OMNIBOX_BROWSER_KEYWORD_PROVIDER_H_

#include <stddef.h>

#include <memory>
#include <string>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "components/metrics/proto/omnibox_input_type.pb.h"
#include "components/omnibox/browser/autocomplete_input.h"
#include "components/omnibox/browser/autocomplete_provider.h"
#include "components/omnibox/browser/keyword_extensions_delegate.h"

class AutocompleteProviderClient;
class AutocompleteProviderListener;
class KeywordExtensionsDelegate;
class TemplateURL;
class TemplateURLService;

// Autocomplete provider for keyword input.
//
// After construction, the autocomplete controller repeatedly calls Start()
// with some user input, each time expecting to receive a small set of the best
// matches (either synchronously or asynchronously).
//
// To construct these matches, the provider treats user input as a series of
// whitespace-delimited tokens and tries to match the first token as the prefix
// of a known "keyword".  A keyword is some string that maps to a search query
// URL; the rest of the user's input is taken as the input to the query.  For
// example, the keyword "bug" might map to the URL "http://b/issue?id=%s", so
// input like "bug 123" would become "http://b/issue?id=123".
//
// Because we do prefix matching, user input could match more than one keyword
// at once.  (Example: the input "f jazz" matches all keywords starting with
// "f".)  We return the best matches, up to three.
//
// The resulting matches are shown with content specified by the keyword
// (usually "Search [name] for %s"), description "(Keyword: [keyword])", and
// action "[keyword] %s".  If the user has typed a (possibly partial) keyword
// but no search terms, the suggested result is shown greyed out, with
// "<enter term(s)>" as the substituted input, and does nothing when selected.
class KeywordProvider : public AutocompleteProvider {
 public:
  KeywordProvider(AutocompleteProviderClient* client,
                  AutocompleteProviderListener* listener);

  // Extracts the next whitespace-delimited token from input and returns it.
  // Sets |remaining_input| to everything after the first token (skipping over
  // the first intervening whitespace).
  // If |trim_leading_whitespace| is true then leading whitespace in
  // |*remaining_input| will be trimmed.
  static base::string16 SplitKeywordFromInput(const base::string16& input,
                                              bool trim_leading_whitespace,
                                              base::string16* remaining_input);

  // Returns the replacement string from the user input. The replacement
  // string is the portion of the input that does not contain the keyword.
  // For example, the replacement string for "b blah" is blah.
  // If |trim_leading_whitespace| is true then leading whitespace in
  // replacement string will be trimmed.
  static base::string16 SplitReplacementStringFromInput(
      const base::string16& input,
      bool trim_leading_whitespace);

  // Returns the matching substituting keyword for |input|, or NULL if there
  // is no keyword for the specified input.  If the matching keyword was found,
  // updates |input|'s text and cursor position.
  static const TemplateURL* GetSubstitutingTemplateURLForInput(
      TemplateURLService* model,
      AutocompleteInput* input);

  // If |text| corresponds (in the sense of
  // TemplateURLModel::CleanUserInputKeyword()) to an enabled, substituting
  // keyword, returns that keyword; returns the empty string otherwise.
  base::string16 GetKeywordForText(const base::string16& text) const;

  // Creates a fully marked-up AutocompleteMatch for a specific keyword.
  AutocompleteMatch CreateVerbatimMatch(const base::string16& text,
                                        const base::string16& keyword,
                                        const AutocompleteInput& input);

  // AutocompleteProvider:
  void Start(const AutocompleteInput& input, bool minimal_changes) override;
  void Stop(bool clear_cached_results,
            bool due_to_user_inactivity) override;

 private:
  friend class KeywordExtensionsDelegateImpl;

  ~KeywordProvider() override;

  // Extracts the keyword from |input| into |keyword|. Any remaining characters
  // after the keyword are placed in |remaining_input|. Returns true if |input|
  // is valid and has a keyword. This makes use of SplitKeywordFromInput to
  // extract the keyword and remaining string, and uses
  // TemplateURLService::CleanUserInputKeyword to remove unnecessary characters.
  // In general use this instead of SplitKeywordFromInput.
  // Leading whitespace in |*remaining_input| will be trimmed.
  static bool ExtractKeywordFromInput(const AutocompleteInput& input,
                                      base::string16* keyword,
                                      base::string16* remaining_input);

  // Determines the relevance for some input, given its type, whether the user
  // typed the complete keyword (or close to it), and whether the user is in
  // "prefer keyword matches" mode, and whether the keyword supports
  // replacement. If |allow_exact_keyword_match| is false, the relevance for
  // complete keywords that support replacements is degraded.
  static int CalculateRelevance(metrics::OmniboxInputType type,
                                bool complete,
                                bool sufficiently_complete,
                                bool support_replacement,
                                bool prefer_keyword,
                                bool allow_exact_keyword_match);

  // Creates a fully marked-up AutocompleteMatch from the user's input.
  // If |relevance| is negative, calculate a relevance based on heuristics.
  AutocompleteMatch CreateAutocompleteMatch(
      const TemplateURL* template_url,
      const size_t meaningful_keyword_length,
      const AutocompleteInput& input,
      size_t prefix_length,
      const base::string16& remaining_input,
      bool allowed_to_be_default_match,
      int relevance);

  // Fills in the "destination_url" and "contents" fields of |match| with the
  // provided user input and keyword data.
  void FillInURLAndContents(const base::string16& remaining_input,
                            const TemplateURL* element,
                            AutocompleteMatch* match) const;

  TemplateURLService* GetTemplateURLService() const;

  AutocompleteProviderListener* listener_;

  // Model for the keywords.
  TemplateURLService* model_;

  // Delegate to handle the extensions-only logic for KeywordProvider.
  // NULL when extensions are not enabled. May be NULL for tests.
  std::unique_ptr<KeywordExtensionsDelegate> extensions_delegate_;

  DISALLOW_COPY_AND_ASSIGN(KeywordProvider);
};

#endif  // COMPONENTS_OMNIBOX_BROWSER_KEYWORD_PROVIDER_H_
