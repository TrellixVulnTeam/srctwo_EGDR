// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/client/client_telemetry_logger.h"

#include <deque>
#include <string>

#include "base/memory/ptr_util.h"
#include "remoting/protocol/connection_to_host.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

// Returns true if |actual| has all fields that |expected| has and the values
// also match. |actual| can have fields that |expected| doesn't have but not the
// other way around.
static bool Contains(const remoting::ChromotingEvent& actual,
                     const remoting::ChromotingEvent& expected) {
  auto actual_dict = actual.CopyDictionaryValue();
  auto expected_dict = expected.CopyDictionaryValue();

  base::DictionaryValue::Iterator expected_it(*expected_dict);

  while (!expected_it.IsAtEnd()) {
    const std::string& key = expected_it.key();
    const base::Value* out_value = nullptr;

    if (!actual_dict->Get(key, &out_value) ||
        !expected_it.value().Equals(out_value)) {
      return false;
    }
    expected_it.Advance();
  }
  return true;
}

}  // namespace

namespace remoting {

// The fake log writer will pass the test IFF:
// 1. The caller logs an entry when the writer expects an entry to be logged.
// 2. The caller logs an entry with all key-value pairs found in the expected
//    entry.
// 3. There are no more expected log entries when the writer destructs.
class FakeLogWriter : public ChromotingEventLogWriter {
 public:
  ~FakeLogWriter() override {
    EXPECT_TRUE(expected_events_.empty()) << "Sent less logs than expected.";
  }

  // Add the event to |expected_events_|. Log() will only succeed IFF the actual
  // entry has all fields that the expected entry (first entry in
  // |expected_events|) has and the values also match. The actual entry can have
  // more fields in addition to those in the expected entry.
  void AddExpectedEvent(const ChromotingEvent& entry);

  // ChromotingEventLogWriter overrides.
  void Log(const ChromotingEvent& entry) override;
  void SetAuthToken(const std::string& auth_token) override;
  void SetAuthClosure(const base::Closure& closure) override;

  const std::string& auth_token() const { return auth_token_; }
  const base::Closure& auth_closure() const { return auth_closure_; }

 private:
  std::deque<ChromotingEvent> expected_events_;
  std::string auth_token_;
  base::Closure auth_closure_;
};

void FakeLogWriter::AddExpectedEvent(const ChromotingEvent& entry) {
  expected_events_.push_back(entry);
}

void FakeLogWriter::Log(const ChromotingEvent& entry) {
  ASSERT_FALSE(expected_events_.empty())
      << "Trying to send more logs than expected";
  ASSERT_TRUE(Contains(entry, expected_events_.front()))
      << "Unexpected log being sent.";
  expected_events_.pop_front();
}

void FakeLogWriter::SetAuthToken(const std::string& auth_token) {
  auth_token_ = auth_token;
}

void FakeLogWriter::SetAuthClosure(const base::Closure& closure) {
  auth_closure_ = closure;
}

class ClientTelemetryLoggerTest : public testing::Test {
 public:
  // testing::Test override.
  void SetUp() override;

 protected:
  std::unique_ptr<FakeLogWriter> log_writer_;
  std::unique_ptr<ClientTelemetryLogger> logger_;
};

void ClientTelemetryLoggerTest::SetUp() {
  log_writer_.reset(new FakeLogWriter());
  logger_.reset(new ClientTelemetryLogger(log_writer_.get(),
                                          ChromotingEvent::Mode::ME2ME));
}

TEST_F(ClientTelemetryLoggerTest, LogSessionStateChange) {
  ChromotingEvent event(ChromotingEvent::Type::SESSION_STATE);
  event.SetEnum("session_state", ChromotingEvent::SessionState::CONNECTED);
  event.SetEnum("connection_error", ChromotingEvent::ConnectionError::NONE);
  log_writer_->AddExpectedEvent(event);
  logger_->LogSessionStateChange(ChromotingEvent::SessionState::CONNECTED,
                                 ChromotingEvent::ConnectionError::NONE);

  event.SetEnum("session_state",
                ChromotingEvent::SessionState::CONNECTION_FAILED);
  event.SetEnum("connection_error",
                ChromotingEvent::ConnectionError::HOST_OFFLINE);
  log_writer_->AddExpectedEvent(event);
  logger_->LogSessionStateChange(
      ChromotingEvent::SessionState::CONNECTION_FAILED,
      ChromotingEvent::ConnectionError::HOST_OFFLINE);
}

TEST_F(ClientTelemetryLoggerTest, LogStatistics) {
  protocol::PerformanceTracker perf_tracker;
  log_writer_->AddExpectedEvent(
      ChromotingEvent(ChromotingEvent::Type::CONNECTION_STATISTICS));
  logger_->LogStatistics(&perf_tracker);
}

TEST_F(ClientTelemetryLoggerTest, SessionIdGeneration) {
  ChromotingEvent any_event;
  log_writer_->AddExpectedEvent(any_event);
  log_writer_->AddExpectedEvent(any_event);
  log_writer_->AddExpectedEvent(any_event);
  logger_->LogSessionStateChange(ChromotingEvent::SessionState::CONNECTED,
                                 ChromotingEvent::ConnectionError::NONE);
  std::string last_id = logger_->session_id();

  logger_->LogSessionStateChange(ChromotingEvent::SessionState::CLOSED,
                                 ChromotingEvent::ConnectionError::NONE);
  EXPECT_TRUE(logger_->session_id().empty());

  logger_->LogSessionStateChange(ChromotingEvent::SessionState::CONNECTED,
                                 ChromotingEvent::ConnectionError::NONE);
  EXPECT_NE(last_id, logger_->session_id());
}

TEST_F(ClientTelemetryLoggerTest, SessionIdExpiration) {
  log_writer_->AddExpectedEvent(
      ChromotingEvent(ChromotingEvent::Type::SESSION_STATE));
  log_writer_->AddExpectedEvent(
      ChromotingEvent(ChromotingEvent::Type::SESSION_ID_OLD));
  log_writer_->AddExpectedEvent(
      ChromotingEvent(ChromotingEvent::Type::SESSION_ID_NEW));
  log_writer_->AddExpectedEvent(
      ChromotingEvent(ChromotingEvent::Type::CONNECTION_STATISTICS));
  logger_->LogSessionStateChange(ChromotingEvent::SessionState::CONNECTED,
                                 ChromotingEvent::ConnectionError::NONE);
  std::string last_id = logger_->session_id();

  // kMaxSessionIdAgeDays = 1. Fake the generation time to be 2 days ago and
  // force it to expire.
  logger_->SetSessionIdGenerationTimeForTest(base::TimeTicks::Now() -
                                             base::TimeDelta::FromDays(2));
  protocol::PerformanceTracker perf_tracker;
  logger_->LogStatistics(&perf_tracker);
  EXPECT_NE(last_id, logger_->session_id());
}

}  // namespace remoting
