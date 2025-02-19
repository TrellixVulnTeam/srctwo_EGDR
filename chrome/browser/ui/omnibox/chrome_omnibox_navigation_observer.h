// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_OMNIBOX_CHROME_OMNIBOX_NAVIGATION_OBSERVER_H_
#define CHROME_BROWSER_UI_OMNIBOX_CHROME_OMNIBOX_NAVIGATION_OBSERVER_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "components/omnibox/browser/autocomplete_match.h"
#include "components/omnibox/browser/omnibox_navigation_observer.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/reload_type.h"
#include "content/public/browser/web_contents_observer.h"
#include "net/url_request/url_fetcher_delegate.h"

class Profile;
class ShortcutsBackend;

namespace net {
class URLFetcher;
}

// Monitors omnibox navigations in order to trigger behaviors that depend on
// successful navigations.
//
// Currently two such behaviors exist:
// (1) For single-word queries where we can't tell if the entry was a search or
//     an intranet hostname, the omnibox opens as a search by default, but this
//     class attempts to open as a URL via an HTTP HEAD request.  If successful,
//     displays an infobar once the search result has also loaded.  See
//     AlternateNavInfoBarDelegate.
// (2) Omnibox navigations that complete successfully are added to the
//     Shortcuts backend.
//
// Please see the class comment on the base class for important information
// about the memory management of this object.
class ChromeOmniboxNavigationObserver : public OmniboxNavigationObserver,
                                        public content::NotificationObserver,
                                        public content::WebContentsObserver,
                                        public net::URLFetcherDelegate {
 public:
  enum LoadState {
    LOAD_NOT_SEEN,
    LOAD_PENDING,
    LOAD_COMMITTED,
  };

  ChromeOmniboxNavigationObserver(Profile* profile,
                                  const base::string16& text,
                                  const AutocompleteMatch& match,
                                  const AutocompleteMatch& alternate_nav_match);
  ~ChromeOmniboxNavigationObserver() override;

  LoadState load_state() const { return load_state_; }

  // Called directly by ChromeOmniboxClient when an extension-related navigation
  // occurs.  Such navigations don't trigger an immediate NAV_ENTRY_PENDING and
  // must be handled separately.
  void OnSuccessfulNavigation();

 private:
  enum FetchState {
    FETCH_NOT_COMPLETE,
    FETCH_SUCCEEDED,
    FETCH_FAILED,
  };

  // OmniboxNavigationObserver:
  bool HasSeenPendingLoad() const override;

  // content::NotificationObserver:
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  // content::WebContentsObserver:
  void DidFinishNavigation(
      content::NavigationHandle* navigation_handle) override;
  void NavigationEntryCommitted(
      const content::LoadCommittedDetails& load_details) override;
  void WebContentsDestroyed() override;

  // net::URLFetcherDelegate:
  void OnURLFetchComplete(const net::URLFetcher* source) override;

  // Once the load has committed and any URL fetch has completed, this displays
  // the alternate nav infobar if necessary, and deletes |this|.
  void OnAllLoadingFinished();

  const base::string16 text_;
  const AutocompleteMatch match_;
  const AutocompleteMatch alternate_nav_match_;
  scoped_refptr<ShortcutsBackend> shortcuts_backend_;  // NULL in incognito.
  std::unique_ptr<net::URLFetcher> fetcher_;
  LoadState load_state_;
  FetchState fetch_state_;

  content::NotificationRegistrar registrar_;

  DISALLOW_COPY_AND_ASSIGN(ChromeOmniboxNavigationObserver);
};

#endif  // CHROME_BROWSER_UI_OMNIBOX_CHROME_OMNIBOX_NAVIGATION_OBSERVER_H_
