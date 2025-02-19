// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_CUSTOM_HOME_PAGES_TABLE_MODEL_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_CUSTOM_HOME_PAGES_TABLE_MODEL_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/task/cancelable_task_tracker.h"
#include "components/history/core/browser/history_types.h"
#include "ui/base/models/table_model.h"

class Browser;
class GURL;
class Profile;

namespace content {
class WebContents;
}

namespace history {
class URLRow;
}

namespace ui {
class TableModelObserver;
}

// CustomHomePagesTableModel is the model for the TableView showing the list
// of pages the user wants opened on startup.

class CustomHomePagesTableModel : public ui::TableModel {
 public:
  explicit CustomHomePagesTableModel(Profile* profile);
  ~CustomHomePagesTableModel() override;

  // Sets the set of urls that this model contains.
  void SetURLs(const std::vector<GURL>& urls);

  // Collect all entries indexed by |index_list|, and moves them to be right
  // before the element addressed by |insert_before|. Used by Drag&Drop.
  // Expects |index_list| to be ordered ascending.
  void MoveURLs(int insert_before, const std::vector<int>& index_list);

  // Adds an entry at the specified index.
  void Add(int index, const GURL& url);

  // Removes the entry at the specified index.
  void Remove(int index);

  // Clears any entries and fills the list with pages currently opened in the
  // browser. |ignore_contents| is omitted from the open pages.
  void SetToCurrentlyOpenPages(const content::WebContents* ignore_contents);

  // Returns the set of urls this model contains.
  std::vector<GURL> GetURLs();

  // TableModel overrides:
  int RowCount() override;
  base::string16 GetText(int row, int column_id) override;
  base::string16 GetTooltip(int row) override;
  void SetObserver(ui::TableModelObserver* observer) override;

 private:
  // Each item in the model is represented as an Entry. Entry stores the URL
  // and title of the page.
  struct Entry;

  // Returns false if pages from |browser| should not be considered.
  bool ShouldIncludeBrowser(Browser* browser);

  // Loads the title for the specified entry.
  void LoadTitle(Entry* entry);

  // Loads all the titles, notifies the observer of the change once all loads
  // are complete.
  void LoadAllTitles();

  // Callback from history service. Updates the title of the Entry whose
  // |url| matches |entry_url| and notifies the observer of the change if
  // |observable| is true.
  void OnGotTitle(const GURL& entry_url,
                  bool observable,
                  bool found_url,
                  const history::URLRow& row,
                  const history::VisitVector& visits);

  // Like OnGotTitle, except that num_outstanding_title_lookups_ is decremented
  // and if the count reaches zero the observer is notifed.
  void OnGotOneOfManyTitles(const GURL& entry_url,
                            bool found_url,
                            const history::URLRow& row,
                            const history::VisitVector& visits);

  // Adds an entry at the specified index, but doesn't load the title or tell
  // the observer.
  void AddWithoutNotification(int index, const GURL& url);

  // Removes the entry at the specified index, but doesn't tell the observer.
  void RemoveWithoutNotification(int index);

  // Returns the URL for a particular row, formatted for display to the user.
  base::string16 FormattedURL(int row) const;

  // Set of entries we're showing.
  std::vector<Entry> entries_;

  // Profile used to load titles.
  Profile* profile_;

  ui::TableModelObserver* observer_;

  // Used in loading titles.
  base::CancelableTaskTracker task_tracker_;

  // Used to keep track of when it's time to update the observer when loading
  // multiple titles.
  int num_outstanding_title_lookups_;

  DISALLOW_COPY_AND_ASSIGN(CustomHomePagesTableModel);
};

#endif  // CHROME_BROWSER_UI_WEBUI_SETTINGS_CUSTOM_HOME_PAGES_TABLE_MODEL_H_
