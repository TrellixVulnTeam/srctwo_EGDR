// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/history/core/browser/url_database.h"

#include <limits>
#include <string>
#include <vector>

#include "base/i18n/case_conversion.h"
#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "components/history/core/browser/keyword_search_term.h"
#include "components/url_formatter/url_formatter.h"
#include "sql/statement.h"
#include "url/gurl.h"

namespace history {

const char URLDatabase::kURLRowFields[] = HISTORY_URL_ROW_FIELDS;
const int URLDatabase::kNumURLRowFields = 9;

URLDatabase::URLEnumeratorBase::URLEnumeratorBase()
    : initialized_(false) {
}

URLDatabase::URLEnumeratorBase::~URLEnumeratorBase() {
}

URLDatabase::URLEnumerator::URLEnumerator() {
}

bool URLDatabase::URLEnumerator::GetNextURL(URLRow* r) {
  if (statement_.Step()) {
    FillURLRow(statement_, r);
    return true;
  }
  return false;
}

URLDatabase::URLDatabase()
    : has_keyword_search_terms_(false) {
}

URLDatabase::~URLDatabase() {
}

// static
std::string URLDatabase::GURLToDatabaseURL(const GURL& gurl) {
  // TODO(brettw): do something fancy here with encoding, etc.

  // Strip username and password from URL before sending to DB.
  GURL::Replacements replacements;
  replacements.ClearUsername();
  replacements.ClearPassword();

  return (gurl.ReplaceComponents(replacements)).spec();
}

// Convenience to fill a URLRow. Must be in sync with the fields in
// kURLRowFields.
void URLDatabase::FillURLRow(sql::Statement& s, URLRow* i) {
  DCHECK(i);
  i->set_id(s.ColumnInt64(0));
  i->set_url(GURL(s.ColumnString(1)));
  i->set_title(s.ColumnString16(2));
  i->set_visit_count(s.ColumnInt(3));
  i->set_typed_count(s.ColumnInt(4));
  i->set_last_visit(base::Time::FromInternalValue(s.ColumnInt64(5)));
  i->set_hidden(s.ColumnInt(6) != 0);
}

bool URLDatabase::GetURLRow(URLID url_id, URLRow* info) {
  // TODO(brettw) We need check for empty URLs to handle the case where
  // there are old URLs in the database that are empty that got in before
  // we added any checks. We should eventually be able to remove it
  // when all inputs are using GURL (which prohibit empty input).
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "SELECT" HISTORY_URL_ROW_FIELDS "FROM urls WHERE id=?"));
  statement.BindInt64(0, url_id);

  if (statement.Step()) {
    FillURLRow(statement, info);
    return true;
  }
  return false;
}

bool URLDatabase::GetAllTypedUrls(URLRows* urls) {
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "SELECT" HISTORY_URL_ROW_FIELDS "FROM urls WHERE typed_count > 0"));

  while (statement.Step()) {
    URLRow info;
    FillURLRow(statement, &info);
    urls->push_back(info);
  }
  return true;
}

URLID URLDatabase::GetRowForURL(const GURL& url, URLRow* info) {
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "SELECT" HISTORY_URL_ROW_FIELDS "FROM urls WHERE url=?"));
  std::string url_string = GURLToDatabaseURL(url);
  statement.BindString(0, url_string);

  if (!statement.Step())
    return 0;  // no data

  if (info)
    FillURLRow(statement, info);
  return statement.ColumnInt64(0);
}

bool URLDatabase::UpdateURLRow(URLID url_id, const URLRow& info) {
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "UPDATE urls SET title=?,visit_count=?,typed_count=?,last_visit_time=?,"
        "hidden=?"
      "WHERE id=?"));
  statement.BindString16(0, info.title());
  statement.BindInt(1, info.visit_count());
  statement.BindInt(2, info.typed_count());
  statement.BindInt64(3, info.last_visit().ToInternalValue());
  statement.BindInt(4, info.hidden() ? 1 : 0);
  statement.BindInt64(5, url_id);

  return statement.Run() && GetDB().GetLastChangeCount() > 0;
}

URLID URLDatabase::AddURLInternal(const URLRow& info, bool is_temporary) {
  // This function is used to insert into two different tables, so we have to
  // do some shuffling. Unfortinately, we can't use the macro
  // HISTORY_URL_ROW_FIELDS because that specifies the table name which is
  // invalid in the insert syntax.
  #define ADDURL_COMMON_SUFFIX \
      " (url, title, visit_count, typed_count, "\
      "last_visit_time, hidden) "\
      "VALUES (?,?,?,?,?,?)"
  const char* statement_name;
  const char* statement_sql;
  if (is_temporary) {
    statement_name = "AddURLTemporary";
    statement_sql = "INSERT INTO temp_urls" ADDURL_COMMON_SUFFIX;
  } else {
    statement_name = "AddURL";
    statement_sql = "INSERT INTO urls" ADDURL_COMMON_SUFFIX;
  }
  #undef ADDURL_COMMON_SUFFIX

  sql::Statement statement(GetDB().GetCachedStatement(
      sql::StatementID(statement_name), statement_sql));
  statement.BindString(0, GURLToDatabaseURL(info.url()));
  statement.BindString16(1, info.title());
  statement.BindInt(2, info.visit_count());
  statement.BindInt(3, info.typed_count());
  statement.BindInt64(4, info.last_visit().ToInternalValue());
  statement.BindInt(5, info.hidden() ? 1 : 0);

  if (!statement.Run()) {
    VLOG(0) << "Failed to add url " << info.url().possibly_invalid_spec()
            << " to table history.urls.";
    return 0;
  }
  return GetDB().GetLastInsertRowId();
}

bool URLDatabase::InsertOrUpdateURLRowByID(const URLRow& info) {
  // SQLite does not support INSERT OR UPDATE, however, it does have INSERT OR
  // REPLACE, which is feasible to use, because of the following.
  //  * Before INSERTing, REPLACE will delete all pre-existing rows that cause
  //    constraint violations. Here, we only have a PRIMARY KEY constraint, so
  //    the only row that might get deleted is an old one with the same ID.
  //  * Another difference between the two flavors is that the latter actually
  //    deletes the old row, and thus the old values are lost in columns which
  //    are not explicitly assigned new values. This is not an issue, however,
  //    as we assign values to all columns.
  //  * When rows are deleted due to constraint violations, the delete triggers
  //    may not be invoked. As of now, we do not have any delete triggers.
  // For more details, see: http://www.sqlite.org/lang_conflict.html.
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "INSERT OR REPLACE INTO urls "
      "(id, url, title, visit_count, typed_count, last_visit_time, hidden) "
      "VALUES (?, ?, ?, ?, ?, ?, ?)"));

  statement.BindInt64(0, info.id());
  statement.BindString(1, GURLToDatabaseURL(info.url()));
  statement.BindString16(2, info.title());
  statement.BindInt(3, info.visit_count());
  statement.BindInt(4, info.typed_count());
  statement.BindInt64(5, info.last_visit().ToInternalValue());
  statement.BindInt(6, info.hidden() ? 1 : 0);

  return statement.Run();
}

bool URLDatabase::DeleteURLRow(URLID id) {
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "DELETE FROM urls WHERE id = ?"));
  statement.BindInt64(0, id);

  if (!statement.Run())
    return false;

  // And delete any keyword visits.
  return !has_keyword_search_terms_ || DeleteKeywordSearchTermForURL(id);
}

bool URLDatabase::CreateTemporaryURLTable() {
  return CreateURLTable(true);
}

bool URLDatabase::CommitTemporaryURLTable() {
  // See the comments in the header file as well as
  // HistoryBackend::DeleteAllHistory() for more information on how this works
  // and why it does what it does.

  // Swap the url table out and replace it with the temporary one.
  if (!GetDB().Execute("DROP TABLE urls")) {
    NOTREACHED() << GetDB().GetErrorMessage();
    return false;
  }
  if (!GetDB().Execute("ALTER TABLE temp_urls RENAME TO urls")) {
    NOTREACHED() << GetDB().GetErrorMessage();
    return false;
  }

  // Re-create the index over the now permanent URLs table -- this was not there
  // for the temporary table.
  CreateMainURLIndex();

  return true;
}

bool URLDatabase::InitURLEnumeratorForEverything(URLEnumerator* enumerator) {
  DCHECK(!enumerator->initialized_);
  std::string sql("SELECT ");
  sql.append(kURLRowFields);
  sql.append(" FROM urls");
  enumerator->statement_.Assign(GetDB().GetUniqueStatement(sql.c_str()));
  enumerator->initialized_ = enumerator->statement_.is_valid();
  return enumerator->statement_.is_valid();
}

bool URLDatabase::InitURLEnumeratorForSignificant(URLEnumerator* enumerator) {
  DCHECK(!enumerator->initialized_);
  std::string sql("SELECT ");
  sql.append(kURLRowFields);
  sql.append(" FROM urls WHERE last_visit_time >= ? OR visit_count >= ? OR "
             "typed_count >= ?");
  sql.append(
      " ORDER BY typed_count DESC, last_visit_time DESC, visit_count "
      "DESC");
  enumerator->statement_.Assign(GetDB().GetUniqueStatement(sql.c_str()));
  enumerator->statement_.BindInt64(
      0, AutocompleteAgeThreshold().ToInternalValue());
  enumerator->statement_.BindInt(1, kLowQualityMatchVisitLimit);
  enumerator->statement_.BindInt(2, kLowQualityMatchTypedLimit);
  enumerator->initialized_ = enumerator->statement_.is_valid();
  return enumerator->statement_.is_valid();
}

bool URLDatabase::AutocompleteForPrefix(const std::string& prefix,
                                        size_t max_results,
                                        bool typed_only,
                                        URLRows* results) {
  // NOTE: this query originally sorted by starred as the second parameter. But
  // as bookmarks is no longer part of the db we no longer include the order
  // by clause.
  results->clear();
  const char* sql;
  int line;
  if (typed_only) {
    sql = "SELECT" HISTORY_URL_ROW_FIELDS "FROM urls "
        "WHERE url >= ? AND url < ? AND hidden = 0 AND typed_count > 0 "
        "ORDER BY typed_count DESC, visit_count DESC, last_visit_time DESC "
        "LIMIT ?";
    line = __LINE__;
  } else {
    sql = "SELECT" HISTORY_URL_ROW_FIELDS "FROM urls "
        "WHERE url >= ? AND url < ? AND hidden = 0 "
        "ORDER BY typed_count DESC, visit_count DESC, last_visit_time DESC "
        "LIMIT ?";
    line = __LINE__;
  }
  sql::Statement statement(
      GetDB().GetCachedStatement(sql::StatementID(__FILE__, line), sql));

  // We will find all strings between "prefix" and this string, which is prefix
  // followed by the maximum character size. Use 8-bit strings for everything
  // so we can be sure sqlite is comparing everything in 8-bit mode. Otherwise,
  // it will have to convert strings either to UTF-8 or UTF-16 for comparison.
  std::string end_query(prefix);
  end_query.push_back(std::numeric_limits<unsigned char>::max());

  statement.BindString(0, prefix);
  statement.BindString(1, end_query);
  statement.BindInt(2, static_cast<int>(max_results));

  while (statement.Step()) {
    URLRow info;
    FillURLRow(statement, &info);
    if (info.url().is_valid())
      results->push_back(info);
  }
  return !results->empty();
}

bool URLDatabase::IsTypedHost(const std::string& host) {
  const char* schemes[] = {
    url::kHttpScheme,
    url::kHttpsScheme,
    url::kFtpScheme
  };
  URLRows dummy;
  for (size_t i = 0; i < arraysize(schemes); ++i) {
    std::string scheme_and_host(schemes[i]);
    scheme_and_host += url::kStandardSchemeSeparator + host;
    if (AutocompleteForPrefix(scheme_and_host + '/', 1, true, &dummy) ||
        AutocompleteForPrefix(scheme_and_host + ':', 1, true, &dummy))
      return true;
  }
  return false;
}

bool URLDatabase::FindShortestURLFromBase(const std::string& base,
                                          const std::string& url,
                                          int min_visits,
                                          int min_typed,
                                          bool allow_base,
                                          URLRow* info) {
  // Select URLs that start with |base| and are prefixes of |url|.  All parts
  // of this query except the substr() call can be done using the index.  We
  // could do this query with a couple of LIKE or GLOB statements as well, but
  // those wouldn't use the index, and would run into problems with "wildcard"
  // characters that appear in URLs (% for LIKE, or *, ? for GLOB).
  std::string sql("SELECT ");
  sql.append(kURLRowFields);
  sql.append(" FROM urls WHERE url ");
  sql.append(allow_base ? ">=" : ">");
  sql.append(" ? AND url < :end AND url = substr(:end, 1, length(url)) "
             "AND hidden = 0 AND visit_count >= ? AND typed_count >= ? "
             "ORDER BY url LIMIT 1");
  sql::Statement statement(GetDB().GetUniqueStatement(sql.c_str()));
  statement.BindString(0, base);
  statement.BindString(1, url);   // :end
  statement.BindInt(2, min_visits);
  statement.BindInt(3, min_typed);

  if (!statement.Step())
    return false;

  DCHECK(info);
  FillURLRow(statement, info);
  return true;
}

bool URLDatabase::GetTextMatches(const base::string16& query,
                                 URLRows* results) {
  return GetTextMatchesWithAlgorithm(
      query, query_parser::MatchingAlgorithm::DEFAULT, results);
}

bool URLDatabase::GetTextMatchesWithAlgorithm(
    const base::string16& query,
    query_parser::MatchingAlgorithm algorithm,
    URLRows* results) {
  query_parser::QueryNodeVector query_nodes;
  query_parser_.ParseQueryNodes(query, algorithm, &query_nodes);

  results->clear();
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "SELECT" HISTORY_URL_ROW_FIELDS "FROM urls WHERE hidden = 0"));

  while (statement.Step()) {
    query_parser::QueryWordVector query_words;
    base::string16 url = base::i18n::ToLower(statement.ColumnString16(1));
    query_parser_.ExtractQueryWords(url, &query_words);
    GURL gurl(url);
    if (gurl.is_valid()) {
      // Decode punycode to match IDN.
      base::string16 ascii = base::ASCIIToUTF16(gurl.host());
      base::string16 utf = url_formatter::IDNToUnicode(gurl.host());
      if (ascii != utf)
        query_parser_.ExtractQueryWords(utf, &query_words);
    }
    base::string16 title = base::i18n::ToLower(statement.ColumnString16(2));
    query_parser_.ExtractQueryWords(title, &query_words);

    if (query_parser_.DoesQueryMatch(query_words, query_nodes)) {
      URLResult info;
      FillURLRow(statement, &info);
      if (info.url().is_valid())
        results->push_back(info);
    }
  }
  return !results->empty();
}

bool URLDatabase::InitKeywordSearchTermsTable() {
  has_keyword_search_terms_ = true;
  if (!GetDB().DoesTableExist("keyword_search_terms")) {
    if (!GetDB().Execute("CREATE TABLE keyword_search_terms ("
        "keyword_id INTEGER NOT NULL,"      // ID of the TemplateURL.
        "url_id INTEGER NOT NULL,"          // ID of the url.
        "lower_term LONGVARCHAR NOT NULL,"  // The search term, in lower case.
        "term LONGVARCHAR NOT NULL)"))      // The actual search term.
      return false;
  }
  return true;
}

bool URLDatabase::CreateKeywordSearchTermsIndices() {
  // For searching.
  if (!GetDB().Execute(
          "CREATE INDEX IF NOT EXISTS keyword_search_terms_index1 ON "
          "keyword_search_terms (keyword_id, lower_term)")) {
    return false;
  }

  // For deletion.
  if (!GetDB().Execute(
          "CREATE INDEX IF NOT EXISTS keyword_search_terms_index2 ON "
          "keyword_search_terms (url_id)")) {
    return false;
  }

  // For query or deletion by term.
  if (!GetDB().Execute(
          "CREATE INDEX IF NOT EXISTS keyword_search_terms_index3 ON "
          "keyword_search_terms (term)")) {
    return false;
  }
  return true;
}

bool URLDatabase::DropKeywordSearchTermsTable() {
  // This will implicitly delete the indices over the table.
  return GetDB().Execute("DROP TABLE keyword_search_terms");
}

bool URLDatabase::SetKeywordSearchTermsForURL(URLID url_id,
                                              KeywordID keyword_id,
                                              const base::string16& term) {
  DCHECK(url_id && keyword_id && !term.empty());

  sql::Statement exist_statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "SELECT term FROM keyword_search_terms "
      "WHERE keyword_id = ? AND url_id = ?"));
  exist_statement.BindInt64(0, keyword_id);
  exist_statement.BindInt64(1, url_id);

  if (exist_statement.Step())
    return true;  // Term already exists, no need to add it.

  if (!exist_statement.Succeeded())
    return false;

  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "INSERT INTO keyword_search_terms (keyword_id, url_id, lower_term, term) "
      "VALUES (?,?,?,?)"));
  statement.BindInt64(0, keyword_id);
  statement.BindInt64(1, url_id);
  statement.BindString16(2, base::i18n::ToLower(term));
  statement.BindString16(3, term);
  return statement.Run();
}

bool URLDatabase::GetKeywordSearchTermRow(URLID url_id,
                                          KeywordSearchTermRow* row) {
  DCHECK(url_id);
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "SELECT keyword_id, term FROM keyword_search_terms WHERE url_id=?"));
  statement.BindInt64(0, url_id);

  if (!statement.Step())
    return false;

  if (row) {
    row->url_id = url_id;
    row->keyword_id = statement.ColumnInt64(0);
    row->term = statement.ColumnString16(1);
  }
  return true;
}

bool URLDatabase::GetKeywordSearchTermRows(
    const base::string16& term,
    std::vector<KeywordSearchTermRow>* rows) {
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "SELECT keyword_id, url_id FROM keyword_search_terms WHERE term=?"));
  statement.BindString16(0, term);

  if (!statement.is_valid())
    return false;

  while (statement.Step()) {
    KeywordSearchTermRow row;
    row.url_id = statement.ColumnInt64(1);
    row.keyword_id = statement.ColumnInt64(0);
    row.term = term;
    rows->push_back(row);
  }
  return true;
}

void URLDatabase::DeleteAllSearchTermsForKeyword(
    KeywordID keyword_id) {
  DCHECK(keyword_id);
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "DELETE FROM keyword_search_terms WHERE keyword_id=?"));
  statement.BindInt64(0, keyword_id);

  statement.Run();
}

void URLDatabase::GetMostRecentKeywordSearchTerms(
    KeywordID keyword_id,
    const base::string16& prefix,
    int max_count,
    std::vector<KeywordSearchTermVisit>* matches) {
  // NOTE: the keyword_id can be zero if on first run the user does a query
  // before the TemplateURLService has finished loading. As the chances of this
  // occurring are small, we ignore it.
  if (!keyword_id)
    return;

  DCHECK(!prefix.empty());
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "SELECT DISTINCT kv.term, u.visit_count, u.last_visit_time "
      "FROM keyword_search_terms kv "
      "JOIN urls u ON kv.url_id = u.id "
      "WHERE kv.keyword_id = ? AND kv.lower_term >= ? AND kv.lower_term < ? "
      "ORDER BY u.last_visit_time DESC LIMIT ?"));

  // NOTE: Keep this ToLower() call in sync with search_provider.cc.
  base::string16 lower_prefix = base::i18n::ToLower(prefix);
  // This magic gives us a prefix search.
  base::string16 next_prefix = lower_prefix;
  next_prefix.back() = next_prefix.back() + 1;
  statement.BindInt64(0, keyword_id);
  statement.BindString16(1, lower_prefix);
  statement.BindString16(2, next_prefix);
  statement.BindInt(3, max_count);

  KeywordSearchTermVisit visit;
  while (statement.Step()) {
    visit.term = statement.ColumnString16(0);
    visit.visits = statement.ColumnInt(1);
    visit.time = base::Time::FromInternalValue(statement.ColumnInt64(2));
    matches->push_back(visit);
  }
}

bool URLDatabase::DeleteKeywordSearchTerm(const base::string16& term) {
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
      "DELETE FROM keyword_search_terms WHERE term=?"));
  statement.BindString16(0, term);

  return statement.Run();
}

bool URLDatabase::DeleteKeywordSearchTermForURL(URLID url_id) {
  sql::Statement statement(GetDB().GetCachedStatement(
      SQL_FROM_HERE, "DELETE FROM keyword_search_terms WHERE url_id=?"));
  statement.BindInt64(0, url_id);
  return statement.Run();
}

bool URLDatabase::DropStarredIDFromURLs() {
  if (!GetDB().DoesColumnExist("urls", "starred_id"))
    return true;  // urls is already updated, no need to continue.

  return RecreateURLTableWithAllContents();
}

bool URLDatabase::CreateURLTable(bool is_temporary) {
  const char* name = is_temporary ? "temp_urls" : "urls";
  if (GetDB().DoesTableExist(name))
    return true;

  // Note: revise implementation for InsertOrUpdateURLRowByID() if you add any
  // new constraints to the schema.
  std::string sql;
  sql.append("CREATE TABLE ");
  sql.append(name);
  sql.append(
      "("
      // The id uses AUTOINCREMENT is for sync propose. Sync uses this |id| as
      // an unique key to identify the URLs. If here did not use AUTOINCREMENT,
      // and Sync was not working somehow, a ROWID could be deleted and re-used
      // during this period. Once Sync come back, Sync would use ROWIDs and
      // timestamps to see if there are any updates need to be synced. And sync
      // will only see the new URL, but missed the deleted URL.
      //
      // IMPORTANT NOTE: Currently new tables are created with AUTOINCREMENT
      // but the migration code is disabled. This means that you will not
      // be able to count on AUTOINCREMENT behavior without adding
      // additional migration steps.
      //
      // Along with this, an unused favicon_id column will exist for tables
      // without AUTOINCREMENT. This should be removed everywhere.
      //
      // TODO(https://crbug.com/736136) figure out how to update users to use
      // AUTOINCREMENT and remove the favicon_id column consistently.
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "url LONGVARCHAR,"
      "title LONGVARCHAR,"
      "visit_count INTEGER DEFAULT 0 NOT NULL,"
      "typed_count INTEGER DEFAULT 0 NOT NULL,"
      "last_visit_time INTEGER NOT NULL,"
      "hidden INTEGER DEFAULT 0 NOT NULL)");
  // IMPORTANT: If you change the colums, also update in_memory_database.cc
  // where the values are copied (InitFromDisk).

  return GetDB().Execute(sql.c_str());
}

bool URLDatabase::CreateMainURLIndex() {
  return GetDB().Execute(
      "CREATE INDEX IF NOT EXISTS urls_url_index ON urls (url)");
}

bool URLDatabase::RecreateURLTableWithAllContents() {
  // Create a temporary table to contain the new URLs table.
  if (!CreateTemporaryURLTable()) {
    NOTREACHED();
    return false;
  }

  // Copy the contents.
  if (!GetDB().Execute(
          "INSERT INTO temp_urls (id, url, title, visit_count, typed_count, "
          "last_visit_time, hidden) "
          "SELECT id, url, title, visit_count, typed_count, last_visit_time, "
          "hidden FROM urls")) {
    NOTREACHED() << GetDB().GetErrorMessage();
    return false;
  }

  // Rename/commit the tmp table.
  CommitTemporaryURLTable();

  return true;
}

const int kLowQualityMatchTypedLimit = 1;
const int kLowQualityMatchVisitLimit = 4;
const int kLowQualityMatchAgeLimitInDays = 3;

base::Time AutocompleteAgeThreshold() {
  return (base::Time::Now() -
          base::TimeDelta::FromDays(kLowQualityMatchAgeLimitInDays));
}

bool RowQualifiesAsSignificant(const URLRow& row,
                               const base::Time& threshold) {
  if (row.hidden())
    return false;

  const base::Time& real_threshold =
      threshold.is_null() ? AutocompleteAgeThreshold() : threshold;
  return (row.typed_count() >= kLowQualityMatchTypedLimit) ||
         (row.visit_count() >= kLowQualityMatchVisitLimit) ||
         (row.last_visit() >= real_threshold);
}

}  // namespace history
