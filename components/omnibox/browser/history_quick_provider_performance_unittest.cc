// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/omnibox/browser/history_quick_provider.h"

#include <memory>
#include <random>
#include <string>

#include "base/macros.h"
#include "base/run_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/scoped_task_environment.h"
#include "components/history/core/browser/history_backend.h"
#include "components/history/core/browser/history_database.h"
#include "components/history/core/browser/history_service.h"
#include "components/history/core/test/history_service_test_util.h"
#include "components/omnibox/browser/fake_autocomplete_provider_client.h"
#include "components/omnibox/browser/history_test_util.h"
#include "components/omnibox/browser/in_memory_url_index_test_util.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/perf/perf_test.h"

namespace history {

namespace {

// Not threadsafe.
std::string GenerateFakeHashedString(size_t sym_count) {
  static constexpr char kSyms[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,/=+?#";
  CR_DEFINE_STATIC_LOCAL(std::mt19937, engine, ());
  std::uniform_int_distribution<size_t> index_distribution(
      0, arraysize(kSyms) - 2 /* trailing \0 */);

  std::string res;
  res.reserve(sym_count);

  std::generate_n(std::back_inserter(res), sym_count, [&index_distribution] {
    return kSyms[index_distribution(engine)];
  });

  return res;
}

URLRow GeneratePopularURLRow() {
  static constexpr char kPopularUrl[] =
      "http://long.popular_url_with.many_variations/";

  constexpr size_t kFakeHashLength = 10;
  std::string fake_hash = GenerateFakeHashedString(kFakeHashLength);

  URLRow row{GURL(kPopularUrl + fake_hash)};
  EXPECT_TRUE(row.url().is_valid());
  row.set_title(base::UTF8ToUTF16("Page " + fake_hash));
  row.set_visit_count(1);
  row.set_typed_count(1);
  row.set_last_visit(base::Time::Now() - base::TimeDelta::FromDays(1));
  return row;
}

using StringPieces = std::vector<base::StringPiece>;

StringPieces AllPrefixes(const std::string& str) {
  std::vector<base::StringPiece> res;
  res.reserve(str.size());
  for (auto char_it = str.begin(); char_it != str.end(); ++char_it)
    res.push_back({str.begin(), char_it});
  return res;
}

}  // namespace

class HQPPerfTestOnePopularURL : public testing::Test {
 protected:
  HQPPerfTestOnePopularURL() = default;

  void SetUp() override;
  void TearDown() override;

  // Populates history with variations of the same URL.
  void PrepareData();

  // Runs HQP on a banch of consecutive pieces of an input string and times
  // them. Resulting timings printed in groups.
  template <typename PieceIt>
  void RunAllTests(PieceIt first, PieceIt last);

  // Encapsulates calls to performance infrastructure.
  void PrintMeasurements(const std::string& trace_name,
                         const std::vector<base::TimeDelta>& measurements);

  history::HistoryBackend* history_backend() {
    return client_->GetHistoryService()->history_backend_.get();
  }

 private:
  base::TimeDelta RunTest(const base::string16& text);

  base::test::ScopedTaskEnvironment scoped_task_environment_;
  std::unique_ptr<FakeAutocompleteProviderClient> client_;

  scoped_refptr<HistoryQuickProvider> provider_;

  DISALLOW_COPY_AND_ASSIGN(HQPPerfTestOnePopularURL);
};

void HQPPerfTestOnePopularURL::SetUp() {
  if (base::ThreadTicks::IsSupported())
    base::ThreadTicks::WaitUntilInitialized();
  client_.reset(new FakeAutocompleteProviderClient());
  ASSERT_TRUE(client_->GetHistoryService());
  ASSERT_NO_FATAL_FAILURE(PrepareData());
}

void HQPPerfTestOnePopularURL::TearDown() {
  provider_ = nullptr;
  // The InMemoryURLIndex must be explicitly shut down or it will DCHECK() in
  // its destructor.
  client_->GetInMemoryURLIndex()->Shutdown();
  client_->set_in_memory_url_index(nullptr);
  // History index rebuild task is created from main thread during SetUp,
  // performed on DB thread and must be deleted on main thread.
  // Run main loop to process delete task, to prevent leaks.
  base::RunLoop().RunUntilIdle();
}

void HQPPerfTestOnePopularURL::PrepareData() {
  // Adding fake urls to db must be done before RebuildFromHistory(). This will
  // ensure that the index is properly populated with data from the database.
  constexpr size_t kSimilarUrlCount = 10000;
  for (size_t i = 0; i < kSimilarUrlCount; ++i)
    AddFakeURLToHistoryDB(history_backend()->db(), GeneratePopularURLRow());

  InMemoryURLIndex* url_index = client_->GetInMemoryURLIndex();
  url_index->RebuildFromHistory(
      client_->GetHistoryService()->history_backend_->db());
  BlockUntilInMemoryURLIndexIsRefreshed(url_index);

  // History index refresh creates rebuilt tasks to run on history thread.
  // Block here to make sure that all of them are complete.
  history::BlockUntilHistoryProcessesPendingRequests(
      client_->GetHistoryService());

  provider_ = new HistoryQuickProvider(client_.get());
}

void HQPPerfTestOnePopularURL::PrintMeasurements(
    const std::string& trace_name,
    const std::vector<base::TimeDelta>& measurements) {
  auto* test_info = ::testing::UnitTest::GetInstance()->current_test_info();

  std::string durations;
  for (const auto& measurement : measurements)
    durations += std::to_string(measurement.InMillisecondsRoundedUp()) + ',';

  perf_test::PrintResultList(test_info->test_case_name(), test_info->name(),
                             trace_name, durations, "ms", true);
}

base::TimeDelta HQPPerfTestOnePopularURL::RunTest(const base::string16& text) {
  base::RunLoop().RunUntilIdle();
  AutocompleteInput input(text, base::string16::npos, std::string(), GURL(),
                          base::string16(),
                          metrics::OmniboxEventProto::INVALID_SPEC, false,
                          false, true, true, false, TestSchemeClassifier());

  if (base::ThreadTicks::IsSupported()) {
    base::ThreadTicks start = base::ThreadTicks::Now();
    provider_->Start(input, false);
    return base::ThreadTicks::Now() - start;
  }

  base::Time start = base::Time::Now();
  provider_->Start(input, false);
  return base::Time::Now() - start;
}

template <typename PieceIt>
void HQPPerfTestOnePopularURL::RunAllTests(PieceIt first, PieceIt last) {
  constexpr size_t kTestGroupSize = 5;
  std::vector<base::TimeDelta> measurements;
  measurements.reserve(kTestGroupSize);

  for (PieceIt group_start = first; group_start != last;) {
    PieceIt group_end = std::min(group_start + kTestGroupSize, last);

    std::transform(group_start, group_end, std::back_inserter(measurements),
                   [this](const base::StringPiece& prefix) {
                     return RunTest(base::UTF8ToUTF16(prefix));
                   });

    PrintMeasurements(std::to_string(group_start->size()) + '-' +
                          std::to_string((group_end - 1)->size()),
                      measurements);

    measurements.clear();
    group_start = group_end;
  }
}

TEST_F(HQPPerfTestOnePopularURL, Typing) {
  std::string test_url = GeneratePopularURLRow().url().spec();
  StringPieces prefixes = AllPrefixes(test_url);
  RunAllTests(prefixes.begin(), prefixes.end());
}

TEST_F(HQPPerfTestOnePopularURL, Backspacing) {
  std::string test_url = GeneratePopularURLRow().url().spec();
  StringPieces prefixes = AllPrefixes(test_url);
  RunAllTests(prefixes.rbegin(), prefixes.rend());
}

}  // namespace history
