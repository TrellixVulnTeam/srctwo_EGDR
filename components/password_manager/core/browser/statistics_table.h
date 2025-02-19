// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_STATISTICS_TABLE_H_
#define COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_STATISTICS_TABLE_H_

#include <memory>
#include <vector>

#include "base/callback_forward.h"
#include "base/macros.h"
#include "base/time/time.h"
#include "url/gurl.h"

namespace sql {
class Connection;
}

namespace password_manager {

// The statistics containing user interactions with a site.
struct InteractionsStats {
  InteractionsStats();

  // The domain of the site.
  GURL origin_domain;

  // The value of the username.
  base::string16 username_value;

  // Number of times the user dismissed the bubble.
  int dismissal_count = 0;

  // The date when the row was updated.
  base::Time update_time;
};

bool operator==(const InteractionsStats& lhs, const InteractionsStats& rhs);

// Returns an element from |stats| with |username| or nullptr if not found.
const InteractionsStats* FindStatsByUsername(
    const std::vector<InteractionsStats>& stats,
    const base::string16& username);

// Represents the 'stats' table in the Login Database.
class StatisticsTable {
 public:
  StatisticsTable();
  ~StatisticsTable();

  // Initializes |db_|.
  void Init(sql::Connection* db);

  // Creates the statistics table if it doesn't exist.
  bool CreateTableIfNecessary();

  // Migrates this table to |version|. The current version should be less than
  // |version|. Returns false if there was migration work to do and it failed,
  // true otherwise.
  bool MigrateToVersion(int version);

  // Adds or replaces the statistics about |stats.origin_domain| and
  // |stats.username_value|.
  bool AddRow(const InteractionsStats& stats);

  // Removes the statistics for |domain|. Returns true if the SQL completed
  // successfully.
  bool RemoveRow(const GURL& domain);

  // Returns all statistics from the database.
  std::vector<InteractionsStats> GetAllRows();

  // Returns the statistics for |domain| if it exists.
  std::vector<InteractionsStats> GetRows(const GURL& domain);

  // Removes the statistics between the dates. If |origin_filter| is not null,
  // only statistics for matching origins are removed. Returns true if the SQL
  // completed successfully.
  bool RemoveStatsByOriginAndTime(
      const base::Callback<bool(const GURL&)>& origin_filter,
      base::Time delete_begin,
      base::Time delete_end);

 private:
  sql::Connection* db_;

  DISALLOW_COPY_AND_ASSIGN(StatisticsTable);
};

}  // namespace password_manager

#endif  // COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_STATISTICS_TABLE_H_
