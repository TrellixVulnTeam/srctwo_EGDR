// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/page_load_metrics/page_load_metrics_util.h"

#include <algorithm>

#include "chrome/common/page_load_metrics/page_load_timing.h"

namespace page_load_metrics {

namespace {

bool IsBackgroundAbort(const PageLoadExtraInfo& info) {
  if (!info.started_in_foreground || !info.first_background_time)
    return false;

  if (!info.page_end_time)
    return true;

  return info.first_background_time <= info.page_end_time;
}

PageAbortReason GetAbortReasonForEndReason(PageEndReason end_reason) {
  switch (end_reason) {
    case END_RELOAD:
      return ABORT_RELOAD;
    case END_FORWARD_BACK:
      return ABORT_FORWARD_BACK;
    case END_NEW_NAVIGATION:
      return ABORT_NEW_NAVIGATION;
    case END_STOP:
      return ABORT_STOP;
    case END_CLOSE:
      return ABORT_CLOSE;
    case END_OTHER:
      return ABORT_OTHER;
    default:
      return ABORT_NONE;
  }
}

// Common helper for QueryContainsComponent and QueryContainsComponentPrefix.
bool QueryContainsComponentHelper(const base::StringPiece query,
                                  const base::StringPiece component,
                                  bool component_is_prefix) {
  if (query.empty() || component.empty() ||
      component.length() > query.length()) {
    return false;
  }

  // Verify that the provided query string does not include the query or
  // fragment start character, as the logic below depends on this character not
  // being included.
  DCHECK(query[0] != '?' && query[0] != '#');

  // We shouldn't try to find matches beyond the point where there aren't enough
  // characters left in query to fully match the component.
  const size_t last_search_start = query.length() - component.length();

  // We need to search for matches in a loop, rather than stopping at the first
  // match, because we may initially match a substring that isn't a full query
  // string component. Consider, for instance, the query string 'ab=cd&b=c'. If
  // we search for component 'b=c', the first substring match will be characters
  // 1-3 (zero-based) in the query string. However, this isn't a full component
  // (the full component is ab=cd) so the match will fail. Thus, we must
  // continue our search to find the second substring match, which in the
  // example is at characters 6-8 (the end of the query string) and is a
  // successful component match.
  for (size_t start_offset = 0; start_offset <= last_search_start;
       start_offset += component.length()) {
    start_offset = query.find(component, start_offset);
    if (start_offset == std::string::npos) {
      // We searched to end of string and did not find a match.
      return false;
    }
    // Verify that the character prior to the component is valid (either we're
    // at the beginning of the query string, or are preceded by an ampersand).
    if (start_offset != 0 && query[start_offset - 1] != '&') {
      continue;
    }
    if (!component_is_prefix) {
      // Verify that the character after the component substring is valid
      // (either we're at the end of the query string, or are followed by an
      // ampersand).
      const size_t after_offset = start_offset + component.length();
      if (after_offset < query.length() && query[after_offset] != '&') {
        continue;
      }
    }
    return true;
  }
  return false;
}

}  // namespace

bool WasStartedInForegroundOptionalEventInForeground(
    const base::Optional<base::TimeDelta>& event,
    const PageLoadExtraInfo& info) {
  return info.started_in_foreground && event &&
         (!info.first_background_time ||
          event.value() <= info.first_background_time.value());
}

bool WasStartedInBackgroundOptionalEventInForeground(
    const base::Optional<base::TimeDelta>& event,
    const PageLoadExtraInfo& info) {
  return !info.started_in_foreground && event && info.first_foreground_time &&
         info.first_foreground_time.value() <= event.value() &&
         (!info.first_background_time ||
          event.value() <= info.first_background_time.value());
}

PageAbortInfo GetPageAbortInfo(const PageLoadExtraInfo& info) {
  if (IsBackgroundAbort(info)) {
    // Though most cases where a tab is backgrounded are user initiated, we
    // can't be certain that we were backgrounded due to a user action. For
    // example, on Android, the screen times out after a period of inactivity,
    // resulting in a non-user-initiated backgrounding.
    return {ABORT_BACKGROUND, UserInitiatedInfo::NotUserInitiated(),
            info.first_background_time.value()};
  }

  PageAbortReason abort_reason =
      GetAbortReasonForEndReason(info.page_end_reason);
  if (abort_reason == ABORT_NONE)
    return PageAbortInfo();

  return {abort_reason, info.page_end_user_initiated_info,
          info.page_end_time.value()};
}

base::Optional<base::TimeDelta> GetInitialForegroundDuration(
    const PageLoadExtraInfo& info,
    base::TimeTicks app_background_time) {
  if (!info.started_in_foreground)
    return base::Optional<base::TimeDelta>();

  base::Optional<base::TimeDelta> time_on_page =
      OptionalMin(info.first_background_time, info.page_end_time);

  // If we don't have a time_on_page value yet, and we have an app background
  // time, use the app background time as our end time. This addresses cases
  // where the Chrome app is backgrounded before the page load is complete, on
  // platforms where Chrome may be killed once it goes into the background
  // (Android). In these cases, we use the app background time as the 'end
  // time'.
  if (!time_on_page && !app_background_time.is_null()) {
    time_on_page = app_background_time - info.navigation_start;
  }
  return time_on_page;
}

bool DidObserveLoadingBehaviorInAnyFrame(
    const page_load_metrics::PageLoadExtraInfo& info,
    blink::WebLoadingBehaviorFlag behavior) {
  const int all_frame_loading_behavior_flags =
      info.main_frame_metadata.behavior_flags |
      info.subframe_metadata.behavior_flags;

  return (all_frame_loading_behavior_flags & behavior) != 0;
}

bool IsGoogleSearchHostname(const GURL& url) {
  base::Optional<std::string> result =
      page_load_metrics::GetGoogleHostnamePrefix(url);
  return result && result.value() == "www";
}

bool IsGoogleSearchResultUrl(const GURL& url) {
  // NOTE: we do not require 'q=' in the query, as AJAXy search may instead
  // store the query in the URL fragment.
  if (!IsGoogleSearchHostname(url)) {
    return false;
  }

  if (!QueryContainsComponentPrefix(url.query_piece(), "q=") &&
      !QueryContainsComponentPrefix(url.ref_piece(), "q=")) {
    return false;
  }

  const base::StringPiece path = url.path_piece();
  return path == "/search" || path == "/webhp" || path == "/custom" ||
         path == "/";
}

bool IsGoogleSearchRedirectorUrl(const GURL& url) {
  if (!IsGoogleSearchHostname(url))
    return false;

  // The primary search redirector.  Google search result redirects are
  // differentiated from other general google redirects by 'source=web' in the
  // query string.
  if (url.path_piece() == "/url" && url.has_query() &&
      QueryContainsComponent(url.query_piece(), "source=web")) {
    return true;
  }

  // Intent-based navigations from search are redirected through a second
  // redirector, which receives its redirect URL in the fragment/hash/ref
  // portion of the URL (the portion after '#'). We don't check for the presence
  // of certain params in the ref since this redirector is only used for
  // redirects from search.
  return url.path_piece() == "/searchurl/r.html" && url.has_ref();
}

bool QueryContainsComponent(const base::StringPiece query,
                            const base::StringPiece component) {
  return QueryContainsComponentHelper(query, component, false);
}

bool QueryContainsComponentPrefix(const base::StringPiece query,
                                  const base::StringPiece component) {
  return QueryContainsComponentHelper(query, component, true);
}

}  // namespace page_load_metrics
