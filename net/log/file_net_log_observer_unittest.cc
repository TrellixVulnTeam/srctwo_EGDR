// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/log/file_net_log_observer.h"

#include <memory>
#include <string>
#include <vector>

#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_file.h"
#include "base/files/scoped_temp_dir.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/task_scheduler/task_scheduler.h"
#include "base/threading/thread.h"
#include "base/values.h"
#include "net/base/test_completion_callback.h"
#include "net/log/net_log_entry.h"
#include "net/log/net_log_event_type.h"
#include "net/log/net_log_parameters_callback.h"
#include "net/log/net_log_source.h"
#include "net/log/net_log_source_type.h"
#include "net/log/net_log_util.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_test_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace net {

namespace {

// Indicates the number of event files used in test cases.
const int kTotalNumFiles = 10;

// Used to set the total file size maximum in test cases where the file size
// doesn't matter.
const int kLargeFileSize = 100000000;

// Used to set the size of events to be sent to the observer in test cases
// where event size doesn't matter.
const size_t kDummyEventSize = 150;

// Adds |num_entries| to |logger|. The "inverse" of this is VerifyEventsInLog().
void AddEntries(FileNetLogObserver* logger,
                int num_entries,
                size_t entry_size) {
  // Get base size of event.
  const int kDummyId = 0;
  std::string message = "";
  NetLogParametersCallback callback =
      NetLog::StringCallback("message", &message);
  NetLogSource source(NetLogSourceType::HTTP2_SESSION, kDummyId);
  NetLogEntryData base_entry_data(NetLogEventType::PAC_JAVASCRIPT_ERROR, source,
                                  NetLogEventPhase::BEGIN,
                                  base::TimeTicks::Now(), &callback);
  NetLogEntry base_entry(&base_entry_data,
                         NetLogCaptureMode::IncludeSocketBytes());
  std::unique_ptr<base::Value> value(base_entry.ToValue());
  std::string json;
  base::JSONWriter::Write(*value, &json);
  size_t base_entry_size = json.size();

  // The maximum value of base::TimeTicks::Now() will be the maximum value of
  // int64_t, and if the maximum number of digits are included, the
  // |base_entry_size| could be up to 101 characters. Check that the event
  // format does not include additional padding.
  DCHECK_LE(base_entry_size, 101u);

  // |entry_size| should be at least as big as the largest possible base
  // entry.
  EXPECT_GE(entry_size, 101u);

  // |entry_size| cannot be smaller than the minimum event size.
  EXPECT_GE(entry_size, base_entry_size);

  for (int i = 0; i < num_entries; i++) {
    source = NetLogSource(NetLogSourceType::HTTP2_SESSION, i);
    std::string id = std::to_string(i);

    // String size accounts for the number of digits in id so that all events
    // are the same size.
    message = std::string(entry_size - base_entry_size - id.size() + 1, 'x');
    callback = NetLog::StringCallback("message", &message);
    NetLogEntryData entry_data(NetLogEventType::PAC_JAVASCRIPT_ERROR, source,
                               NetLogEventPhase::BEGIN, base::TimeTicks::Now(),
                               &callback);
    NetLogEntry entry(&entry_data, NetLogCaptureMode::IncludeSocketBytes());
    logger->OnAddEntry(entry);
  }
}

// ParsedNetLog holds the parsed contents of a NetLog file (constants, events,
// and polled data).
struct ParsedNetLog {
  ::testing::AssertionResult InitFromFileContents(const std::string& input);
  const base::DictionaryValue* GetEvent(size_t i) const;

  // Initializes the ParsedNetLog by parsing a JSON file.
  // Owner for the Value tree.
  std::unique_ptr<base::Value> container;

  // A dictionary for the entire netlog.
  const base::DictionaryValue* root = nullptr;

  // The constants dictionary.
  const base::DictionaryValue* constants = nullptr;

  // The events list.
  const base::ListValue* events = nullptr;

  // The optional polled data (may be nullptr).
  const base::DictionaryValue* polled_data = nullptr;
};

::testing::AssertionResult ParsedNetLog::InitFromFileContents(
    const std::string& input) {
  if (input.empty()) {
    return ::testing::AssertionFailure() << "input is empty";
  }

  base::JSONReader reader;
  container = reader.ReadToValue(input);
  if (!container) {
    return ::testing::AssertionFailure() << reader.GetErrorMessage();
  }

  if (!container->GetAsDictionary(&root)) {
    return ::testing::AssertionFailure() << "Not a dictionary";
  }

  if (!root->GetList("events", &events)) {
    return ::testing::AssertionFailure() << "No events list";
  }

  if (!root->GetDictionary("constants", &constants)) {
    return ::testing::AssertionFailure() << "No constants dictionary";
  }

  // Polled data is optional (ignore success).
  root->GetDictionary("polledData", &polled_data);

  return ::testing::AssertionSuccess();
}

// Returns the event at index |i|, or nullptr if there is none.
const base::DictionaryValue* ParsedNetLog::GetEvent(size_t i) const {
  if (!events || i >= events->GetSize())
    return nullptr;

  const base::Value* value;
  if (!events->Get(i, &value))
    return nullptr;

  const base::DictionaryValue* dict;
  if (!value->GetAsDictionary(&dict))
    return nullptr;

  return dict;
}

// Creates a ParsedNetLog by reading a NetLog from a file. Returns nullptr on
// failure.
std::unique_ptr<ParsedNetLog> ReadNetLogFromDisk(
    const base::FilePath& log_path) {
  std::string input;
  if (!base::ReadFileToString(log_path, &input)) {
    ADD_FAILURE() << "Failed reading file: " << log_path.value();
    return nullptr;
  }

  std::unique_ptr<ParsedNetLog> result = std::make_unique<ParsedNetLog>();

  ::testing::AssertionResult init_result = result->InitFromFileContents(input);
  EXPECT_TRUE(init_result);
  if (!init_result)
    return nullptr;

  return result;
}

// Checks that |log| contains events as emitted by AddEntries() above.
// |num_events_emitted| corresponds to |num_entries| of AddEntries(). Whereas
// |num_events_saved| is the expected number of events that have actually been
// written to the log (post-truncation).
void VerifyEventsInLog(const ParsedNetLog* log,
                       size_t num_events_emitted,
                       size_t num_events_saved) {
  ASSERT_TRUE(log);
  ASSERT_LE(num_events_saved, num_events_emitted);
  ASSERT_EQ(num_events_saved, log->events->GetSize());

  // The last |num_events_saved| should all be sequential, with the last one
  // being numbered |num_events_emitted - 1|.
  for (size_t i = 0; i < num_events_saved; ++i) {
    const base::DictionaryValue* event = log->GetEvent(i);
    ASSERT_TRUE(event);

    size_t expected_source_id = num_events_emitted - num_events_saved + i;

    const base::Value* id_value = nullptr;
    ASSERT_TRUE(event->Get("source.id", &id_value));
    int actual_id;
    ASSERT_TRUE(id_value->GetAsInteger(&actual_id));
    ASSERT_EQ(static_cast<int>(expected_source_id), actual_id);
  }
}

// Helper that checks whether |dict| has a string property at |key| having
// |value|.
void ExpectDictionaryContainsProperty(const base::DictionaryValue* dict,
                                      const std::string& key,
                                      const std::string& value) {
  ASSERT_TRUE(dict);
  std::string actual_value;
  ASSERT_TRUE(dict->GetString(key, &actual_value));
  ASSERT_EQ(value, actual_value);
}

// Used for tests that are common to both bounded and unbounded modes of the
// the FileNetLogObserver. The param is true if bounded mode is used.
class FileNetLogObserverTest : public ::testing::TestWithParam<bool> {
 public:
  void SetUp() override {
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
    log_path_ = temp_dir_.GetPath().AppendASCII("net-log.json");
  }

  bool IsBounded() const { return GetParam(); }

  void CreateAndStartObserving(std::unique_ptr<base::Value> constants) {
    if (IsBounded()) {
      logger_ = FileNetLogObserver::CreateBoundedForTests(
          log_path_, kLargeFileSize, kTotalNumFiles, std::move(constants));
    } else {
      logger_ =
          FileNetLogObserver::CreateUnbounded(log_path_, std::move(constants));
    }

    logger_->StartObserving(&net_log_, NetLogCaptureMode::Default());
  }

  bool LogFileExists() {
    // The log files are written by a sequenced task runner. Drain all the
    // scheduled tasks to ensure that the file writing ones have run before
    // checking if they exist.
    base::TaskScheduler::GetInstance()->FlushForTesting();
    return base::PathExists(log_path_);
  }

 protected:
  NetLog net_log_;
  std::unique_ptr<FileNetLogObserver> logger_;
  base::ScopedTempDir temp_dir_;
  base::FilePath log_path_;
};

// Used for tests that are exclusive to the bounded mode of FileNetLogObserver.
class FileNetLogObserverBoundedTest : public ::testing::Test {
 public:
  void SetUp() override {
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
    log_path_ = temp_dir_.GetPath().AppendASCII("net-log.json");
  }

  void CreateAndStartObserving(std::unique_ptr<base::Value> constants,
                               int total_file_size,
                               int num_files) {
    logger_ = FileNetLogObserver::CreateBoundedForTests(
        log_path_, total_file_size, num_files, std::move(constants));
    logger_->StartObserving(&net_log_, NetLogCaptureMode::Default());
  }

  // Returns the path for an internally directory created for bounded logs (this
  // needs to be kept in sync with the implementation).
  base::FilePath GetInprogressDirectory() const {
    return log_path_.AddExtension(FILE_PATH_LITERAL(".inprogress"));
  }

  base::FilePath GetEventFilePath(int index) const {
    return GetInprogressDirectory().AppendASCII(
        "event_file_" + std::to_string(index) + ".json");
  }

  base::FilePath GetEndNetlogPath() const {
    return GetInprogressDirectory().AppendASCII("end_netlog.json");
  }

  base::FilePath GetConstantsPath() const {
    return GetInprogressDirectory().AppendASCII("constants.json");
  }


 protected:
  NetLog net_log_;
  std::unique_ptr<FileNetLogObserver> logger_;
  base::FilePath log_path_;

 private:
  base::ScopedTempDir temp_dir_;
};

// Instantiates each FileNetLogObserverTest to use bounded and unbounded modes.
INSTANTIATE_TEST_CASE_P(,
                        FileNetLogObserverTest,
                        ::testing::Values(true, false));

// Tests deleting a FileNetLogObserver without first calling StopObserving().
TEST_P(FileNetLogObserverTest, ObserverDestroyedWithoutStopObserving) {
  CreateAndStartObserving(nullptr);

  // Send dummy event
  AddEntries(logger_.get(), 1, kDummyEventSize);

  // The log files should have been started.
  ASSERT_TRUE(LogFileExists());

  logger_.reset();

  // When the logger is re-set without having called StopObserving(), the
  // partially written log files are deleted.
  ASSERT_FALSE(LogFileExists());
}

// Tests calling StopObserving() with a null closure.
TEST_P(FileNetLogObserverTest, StopObservingNullClosure) {
  CreateAndStartObserving(nullptr);

  // Send dummy event
  AddEntries(logger_.get(), 1, kDummyEventSize);

  // The log files should have been started.
  ASSERT_TRUE(LogFileExists());

  logger_->StopObserving(nullptr, base::OnceClosure());

  logger_.reset();

  // Since the logger was explicitly stopped, its files should still exist.
  ASSERT_TRUE(LogFileExists());
}

// Tests creating a FileNetLogObserver using an invalid (can't be written to)
// path.
TEST_P(FileNetLogObserverTest, InitLogWithInvalidPath) {
  // Use a path to a non-existent directory.
  log_path_ = temp_dir_.GetPath().AppendASCII("bogus").AppendASCII("path");

  CreateAndStartObserving(nullptr);

  // Send dummy event
  AddEntries(logger_.get(), 1, kDummyEventSize);

  // No log files should have been written, as the log writer will not create
  // missing directories.
  ASSERT_FALSE(LogFileExists());

  logger_->StopObserving(nullptr, base::OnceClosure());

  logger_.reset();

  // There should still be no files.
  ASSERT_FALSE(LogFileExists());
}

TEST_P(FileNetLogObserverTest, GeneratesValidJSONWithNoEvents) {
  TestClosure closure;

  CreateAndStartObserving(nullptr);

  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);
  ASSERT_EQ(0u, log->events->GetSize());
}

TEST_P(FileNetLogObserverTest, GeneratesValidJSONWithOneEvent) {
  TestClosure closure;

  CreateAndStartObserving(nullptr);

  // Send dummy event.
  AddEntries(logger_.get(), 1, kDummyEventSize);

  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);
  ASSERT_EQ(1u, log->events->GetSize());
}

TEST_P(FileNetLogObserverTest, CustomConstants) {
  TestClosure closure;

  const char kConstantKey[] = "magic";
  const char kConstantString[] = "poney";
  std::unique_ptr<base::DictionaryValue> constants(new base::DictionaryValue());
  constants->SetString(kConstantKey, kConstantString);

  CreateAndStartObserving(std::move(constants));

  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);

  // Check that custom constant was correctly printed.
  ExpectDictionaryContainsProperty(log->constants, kConstantKey,
                                   kConstantString);
}

TEST_P(FileNetLogObserverTest, GeneratesValidJSONWithPolledData) {
  TestClosure closure;

  CreateAndStartObserving(nullptr);

  // Create dummy polled data
  const char kDummyPolledDataPath[] = "dummy_path";
  const char kDummyPolledDataString[] = "dummy_info";
  std::unique_ptr<base::DictionaryValue> dummy_polled_data =
      std::make_unique<base::DictionaryValue>();
  dummy_polled_data->SetString(kDummyPolledDataPath, kDummyPolledDataString);

  logger_->StopObserving(std::move(dummy_polled_data), closure.closure());

  closure.WaitForResult();

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);
  ASSERT_EQ(0u, log->events->GetSize());

  // Make sure additional information is present and validate it.
  ASSERT_TRUE(log->polled_data);
  ExpectDictionaryContainsProperty(log->polled_data, kDummyPolledDataPath,
                                   kDummyPolledDataString);
}

// Adds events concurrently from several different threads. The exact order of
// events seen by this test is non-deterministic.
TEST_P(FileNetLogObserverTest, AddEventsFromMultipleThreads) {
  const size_t kNumThreads = 10;
  std::vector<std::unique_ptr<base::Thread>> threads(kNumThreads);
  // Start all the threads. Waiting for them to start is to hopefuly improve
  // the odds of hitting interesting races once events start being added.
  for (size_t i = 0; i < threads.size(); ++i) {
    threads[i] = std::make_unique<base::Thread>(
        base::StringPrintf("WorkerThread%i", static_cast<int>(i)));
    threads[i]->Start();
    threads[i]->WaitUntilThreadStarted();
  }

  CreateAndStartObserving(nullptr);

  const size_t kNumEventsAddedPerThread = 200;

  // Add events in parallel from all the threads.
  for (size_t i = 0; i < kNumThreads; ++i) {
    threads[i]->task_runner()->PostTask(
        FROM_HERE, base::Bind(&AddEntries, base::Unretained(logger_.get()),
                              kNumEventsAddedPerThread, kDummyEventSize));
  }

  // Join all the threads.
  threads.clear();

  // Stop observing.
  TestClosure closure;
  logger_->StopObserving(nullptr, closure.closure());
  closure.WaitForResult();

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);
  // Check that the expected number of events were written to disk.
  EXPECT_EQ(kNumEventsAddedPerThread * kNumThreads, log->events->GetSize());
}

// Sends enough events to the observer to completely fill one file, but not
// write any events to an additional file. Checks the file bounds.
TEST_F(FileNetLogObserverBoundedTest, EqualToOneFile) {
  // The total size of the events is equal to the size of one file.
  // |kNumEvents| * |kEventSize| = |kTotalFileSize| / |kTotalNumEvents|
  const int kTotalFileSize = 5000;
  const int kNumEvents = 2;
  const int kEventSize = 250;
  TestClosure closure;

  CreateAndStartObserving(nullptr, kTotalFileSize, kTotalNumFiles);

  AddEntries(logger_.get(), kNumEvents, kEventSize);
  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);
  VerifyEventsInLog(log.get(), kNumEvents, kNumEvents);
}

// Sends enough events to fill one file, and partially fill a second file.
// Checks the file bounds and writing to a new file.
TEST_F(FileNetLogObserverBoundedTest, OneEventOverOneFile) {
  // The total size of the events is greater than the size of one file, and
  // less than the size of two files. The total size of all events except one
  // is equal to the size of one file, so the last event will be the only event
  // in the second file.
  // (|kNumEvents| - 1) * kEventSize = |kTotalFileSize| / |kTotalNumEvents|
  const int kTotalFileSize = 6000;
  const int kNumEvents = 4;
  const int kEventSize = 200;
  TestClosure closure;

  CreateAndStartObserving(nullptr, kTotalFileSize, kTotalNumFiles);

  AddEntries(logger_.get(), kNumEvents, kEventSize);

  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);
  VerifyEventsInLog(log.get(), kNumEvents, kNumEvents);
}

// Sends enough events to the observer to completely fill two files.
TEST_F(FileNetLogObserverBoundedTest, EqualToTwoFiles) {
  // The total size of the events is equal to the total size of two files.
  // |kNumEvents| * |kEventSize| = 2 * |kTotalFileSize| / |kTotalNumEvents|
  const int kTotalFileSize = 6000;
  const int kNumEvents = 6;
  const int kEventSize = 200;
  TestClosure closure;

  CreateAndStartObserving(nullptr, kTotalFileSize, kTotalNumFiles);

  AddEntries(logger_.get(), kNumEvents, kEventSize);

  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);
  VerifyEventsInLog(log.get(), kNumEvents, kNumEvents);
}

// Sends exactly enough events to the observer to completely fill all files,
// so that all events fit into the event files and no files need to be
// overwritten.
TEST_F(FileNetLogObserverBoundedTest, FillAllFilesNoOverwriting) {
  // The total size of events is equal to the total size of all files.
  // |kEventSize| * |kNumEvents| = |kTotalFileSize|
  const int kTotalFileSize = 10000;
  const int kEventSize = 200;
  const int kFileSize = kTotalFileSize / kTotalNumFiles;
  const int kNumEvents = kTotalNumFiles * ((kFileSize - 1) / kEventSize + 1);
  TestClosure closure;

  CreateAndStartObserving(nullptr, kTotalFileSize, kTotalNumFiles);

  AddEntries(logger_.get(), kNumEvents, kEventSize);

  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);
  VerifyEventsInLog(log.get(), kNumEvents, kNumEvents);
}

// Sends more events to the observer than will fill the WriteQueue, forcing the
// queue to drop an event. Checks that the queue drops the oldest event.
TEST_F(FileNetLogObserverBoundedTest, DropOldEventsFromWriteQueue) {
  // The total size of events is greater than the WriteQueue's memory limit, so
  // the oldest event must be dropped from the queue and not written to any
  // file.
  // |kNumEvents| * |kEventSize| > |kTotalFileSize| * 2
  const int kTotalFileSize = 1000;
  const int kNumEvents = 11;
  const int kEventSize = 200;
  const int kFileSize = kTotalFileSize / kTotalNumFiles;
  TestClosure closure;

  CreateAndStartObserving(nullptr, kTotalFileSize, kTotalNumFiles);

  AddEntries(logger_.get(), kNumEvents, kEventSize);

  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);
  VerifyEventsInLog(
      log.get(), kNumEvents,
      static_cast<size_t>(kTotalNumFiles * ((kFileSize - 1) / kEventSize + 1)));
}

// Sends twice as many events as will fill all files to the observer, so that
// all of the event files will be filled twice, and every file will be
// overwritten.
TEST_F(FileNetLogObserverBoundedTest, OverwriteAllFiles) {
  // The total size of the events is much greater than twice the number of
  // events that can fit in the event files, to make sure that the extra events
  // are written to a file, not just dropped from the queue.
  // |kNumEvents| * |kEventSize| >= 2 * |kTotalFileSize|
  const int kTotalFileSize = 6000;
  const int kNumEvents = 60;
  const int kEventSize = 200;
  const int kFileSize = kTotalFileSize / kTotalNumFiles;
  TestClosure closure;

  CreateAndStartObserving(nullptr, kTotalFileSize, kTotalNumFiles);

  AddEntries(logger_.get(), kNumEvents, kEventSize);

  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // Check that the minimum number of events that should fit in event files
  // have been written to all files.
  int events_per_file = (kFileSize - 1) / kEventSize + 1;
  int events_in_last_file = (kNumEvents - 1) % events_per_file + 1;

  // Indicates the total number of events that should be written to all files.
  int num_events_in_files =
      (kTotalNumFiles - 1) * events_per_file + events_in_last_file;

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);
  VerifyEventsInLog(log.get(), kNumEvents,
                    static_cast<size_t>(num_events_in_files));
}

// Sends enough events to the observer to fill all event files, plus overwrite
// some files, without overwriting all of them. Checks that the FileWriter
// overwrites the file with the oldest events.
TEST_F(FileNetLogObserverBoundedTest, PartiallyOverwriteFiles) {
  // The number of events sent to the observer is greater than the number of
  // events that can fit into the event files, but the events can fit in less
  // than twice the number of event files, so not every file will need to be
  // overwritten.
  // |kTotalFileSize| < |kNumEvents| * |kEventSize|
  // |kNumEvents| * |kEventSize| <= (2 * |kTotalNumFiles| - 1) * |kFileSize|
  const int kTotalFileSize = 6000;
  const int kNumEvents = 50;
  const int kEventSize = 200;
  const int kFileSize = kTotalFileSize / kTotalNumFiles;
  TestClosure closure;

  CreateAndStartObserving(nullptr, kTotalFileSize, kTotalNumFiles);

  AddEntries(logger_.get(), kNumEvents, kEventSize);

  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // Check that the minimum number of events that should fit in event files
  // have been written to a file.
  int events_per_file = (kFileSize - 1) / kEventSize + 1;
  int events_in_last_file = kNumEvents % events_per_file;
  if (!events_in_last_file)
    events_in_last_file = events_per_file;
  int num_events_in_files =
      (kTotalNumFiles - 1) * events_per_file + events_in_last_file;

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);
  VerifyEventsInLog(log.get(), kNumEvents,
                    static_cast<size_t>(num_events_in_files));
}

// Start logging in bounded mode. Create directories in places where the logger
// expects to create files, in order to cause that file creation to fail.
//
//   constants.json      -- succeess
//   event_file_0.json   -- fails to open
//   end_netlog.json     -- fails to open
TEST_F(FileNetLogObserverBoundedTest, SomeFilesFailToOpen) {
  // The total size of events is equal to the total size of all files.
  // |kEventSize| * |kNumEvents| = |kTotalFileSize|
  const int kTotalFileSize = 10000;
  const int kEventSize = 200;
  const int kFileSize = kTotalFileSize / kTotalNumFiles;
  const int kNumEvents = kTotalNumFiles * ((kFileSize - 1) / kEventSize + 1);
  TestClosure closure;

  // Create directories as a means to block files from being created by logger.
  EXPECT_TRUE(base::CreateDirectory(GetEventFilePath(0)));
  EXPECT_TRUE(base::CreateDirectory(GetEndNetlogPath()));

  CreateAndStartObserving(nullptr, kTotalFileSize, kTotalNumFiles);

  AddEntries(logger_.get(), kNumEvents, kEventSize);

  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // The written log is invalid (and hence can't be parsed). It is just the
  // constants.
  std::string log_contents;
  ASSERT_TRUE(base::ReadFileToString(log_path_, &log_contents));
  // TODO(eroman): Verify the partially written log file?

  // Even though FileNetLogObserver didn't create the directory itself, it will
  // unconditionally delete it. The name should be uncommon enough for this be
  // to reasonable.
  EXPECT_FALSE(base::PathExists(GetInprogressDirectory()));
}

// Start logging in bounded mode. Create a file at the path where the logger
// expects to create its inprogress directory to store event files. This will
// cause logging to completely break. open it.
TEST_F(FileNetLogObserverBoundedTest, InprogressDirectoryBlocked) {
  // The total size of events is equal to the total size of all files.
  // |kEventSize| * |kNumEvents| = |kTotalFileSize|
  const int kTotalFileSize = 10000;
  const int kEventSize = 200;
  const int kFileSize = kTotalFileSize / kTotalNumFiles;
  const int kNumEvents = kTotalNumFiles * ((kFileSize - 1) / kEventSize + 1);
  TestClosure closure;

  // By creating a file where a directory should be, it will not be possible to
  // write any event files.
  EXPECT_TRUE(base::WriteFile(GetInprogressDirectory(), "x", 1));

  CreateAndStartObserving(nullptr, kTotalFileSize, kTotalNumFiles);

  AddEntries(logger_.get(), kNumEvents, kEventSize);

  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // There will be a log file at the final output, however it will be empty
  // since nothing was written to the .inprogress directory.
  std::string log_contents;
  ASSERT_TRUE(base::ReadFileToString(log_path_, &log_contents));
  EXPECT_EQ("", log_contents);

  // FileNetLogObserver unconditionally deletes the inprogress path (even though
  // it didn't actually create this file and it was a file instead of a
  // directory).
  // TODO(eroman): Should it only delete if it is a file?
  EXPECT_FALSE(base::PathExists(GetInprogressDirectory()));
}

// Start logging in bounded mode. Create a file with the same name as the 0th
// events file. This will prevent any events from being written.
TEST_F(FileNetLogObserverBoundedTest, BlockEventsFile0) {
  // The total size of events is equal to the total size of all files.
  // |kEventSize| * |kNumEvents| = |kTotalFileSize|
  const int kTotalFileSize = 10000;
  const int kEventSize = 200;
  const int kFileSize = kTotalFileSize / kTotalNumFiles;
  const int kNumEvents = kTotalNumFiles * ((kFileSize - 1) / kEventSize + 1);
  TestClosure closure;

  // Block the 0th events file.
  EXPECT_TRUE(base::CreateDirectory(GetEventFilePath(0)));

  CreateAndStartObserving(nullptr, kTotalFileSize, kTotalNumFiles);

  AddEntries(logger_.get(), kNumEvents, kEventSize);

  logger_->StopObserving(nullptr, closure.closure());

  closure.WaitForResult();

  // Verify the written log.
  std::unique_ptr<ParsedNetLog> log = ReadNetLogFromDisk(log_path_);
  ASSERT_TRUE(log);
  ASSERT_EQ(0u, log->events->GetSize());
}

void AddEntriesViaNetLog(NetLog* net_log, int num_entries) {
  for (int i = 0; i < num_entries; i++) {
    net_log->AddGlobalEntry(NetLogEventType::PAC_JAVASCRIPT_ERROR);
  }
}

TEST_P(FileNetLogObserverTest, AddEventsFromMultipleThreadsWithStopObserving) {
  const size_t kNumThreads = 10;
  std::vector<std::unique_ptr<base::Thread>> threads(kNumThreads);
  // Start all the threads. Waiting for them to start is to hopefully improve
  // the odds of hitting interesting races once events start being added.
  for (size_t i = 0; i < threads.size(); ++i) {
    threads[i] = std::make_unique<base::Thread>(
        base::StringPrintf("WorkerThread%i", static_cast<int>(i)));
    threads[i]->Start();
    threads[i]->WaitUntilThreadStarted();
  }

  CreateAndStartObserving(nullptr);

  const size_t kNumEventsAddedPerThread = 200;

  // Add events in parallel from all the threads.
  for (size_t i = 0; i < kNumThreads; ++i) {
    threads[i]->task_runner()->PostTask(
        FROM_HERE, base::Bind(&AddEntriesViaNetLog, base::Unretained(&net_log_),
                              kNumEventsAddedPerThread));
  }

  // Stop observing.
  TestClosure closure;
  logger_->StopObserving(nullptr, closure.closure());
  closure.WaitForResult();

  // Join all the threads.
  threads.clear();

  ASSERT_TRUE(LogFileExists());
}

TEST_P(FileNetLogObserverTest,
       AddEventsFromMultipleThreadsWithoutStopObserving) {
  const size_t kNumThreads = 10;
  std::vector<std::unique_ptr<base::Thread>> threads(kNumThreads);
  // Start all the threads. Waiting for them to start is to hopefully improve
  // the odds of hitting interesting races once events start being added.
  for (size_t i = 0; i < threads.size(); ++i) {
    threads[i] = std::make_unique<base::Thread>(
        base::StringPrintf("WorkerThread%i", static_cast<int>(i)));
    threads[i]->Start();
    threads[i]->WaitUntilThreadStarted();
  }

  CreateAndStartObserving(nullptr);

  const size_t kNumEventsAddedPerThread = 200;

  // Add events in parallel from all the threads.
  for (size_t i = 0; i < kNumThreads; ++i) {
    threads[i]->task_runner()->PostTask(
        FROM_HERE, base::Bind(&AddEntriesViaNetLog, base::Unretained(&net_log_),
                              kNumEventsAddedPerThread));
  }

  // Destroy logger.
  logger_.reset();

  // Join all the threads.
  threads.clear();

  // The log file doesn't exist since StopObserving() was not called.
  ASSERT_FALSE(LogFileExists());
}

}  // namespace

}  // namespace net
