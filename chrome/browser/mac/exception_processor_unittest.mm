// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/mac/exception_processor.h"

#import <Cocoa/Cocoa.h>
#include <stddef.h>
#include <sys/wait.h>

#include "base/mac/os_crash_dumps.h"
#include "base/metrics/histogram_macros.h"
#include "base/metrics/histogram_samples.h"
#include "base/metrics/statistics_recorder.h"
#include "testing/gtest/include/gtest/gtest.h"

using base::HistogramBase;
using base::HistogramSamples;
using base::StatisticsRecorder;

namespace chrome {

// Generate an NSException with the given name.
NSException* ExceptionNamed(NSString* name) {
  return [NSException exceptionWithName:name
                                 reason:@"No reason given"
                               userInfo:nil];
}

// Helper to keep binning expectations readible.
size_t BinForExceptionNamed(NSString* name) {
  return BinForException(ExceptionNamed(name));
}

TEST(ExceptionProcessorTest, ExceptionBinning) {
  // These exceptions must be in this order.
  EXPECT_EQ(BinForExceptionNamed(NSGenericException), 0U);
  EXPECT_EQ(BinForExceptionNamed(NSRangeException), 1U);
  EXPECT_EQ(BinForExceptionNamed(NSInvalidArgumentException), 2U);
  EXPECT_EQ(BinForExceptionNamed(NSMallocException), 3U);

  // Random other exceptions map to |kUnknownNSException|.
  EXPECT_EQ(BinForExceptionNamed(@"CustomName"), kUnknownNSException);
  EXPECT_EQ(BinForExceptionNamed(@"Custom Name"), kUnknownNSException);
  EXPECT_EQ(BinForExceptionNamed(@""), kUnknownNSException);
  EXPECT_EQ(BinForException(nil), kUnknownNSException);
}

TEST(ExceptionProcessorTest, RecordException) {
  // Start up a histogram recorder.
  // TODO(rtenneti): Leaks StatisticsRecorder and will update suppressions.
  base::StatisticsRecorder::Initialize();

  StatisticsRecorder::Histograms histograms;
  StatisticsRecorder::GetSnapshot("OSX.NSException", &histograms);
  EXPECT_EQ(0U, histograms.size());

  // Record some known exceptions.
  RecordExceptionWithUma(ExceptionNamed(NSGenericException));
  RecordExceptionWithUma(ExceptionNamed(NSGenericException));
  RecordExceptionWithUma(ExceptionNamed(NSGenericException));
  RecordExceptionWithUma(ExceptionNamed(NSGenericException));
  RecordExceptionWithUma(ExceptionNamed(NSRangeException));
  RecordExceptionWithUma(ExceptionNamed(NSInvalidArgumentException));
  RecordExceptionWithUma(ExceptionNamed(NSInvalidArgumentException));
  RecordExceptionWithUma(ExceptionNamed(NSInvalidArgumentException));
  RecordExceptionWithUma(ExceptionNamed(NSMallocException));
  RecordExceptionWithUma(ExceptionNamed(NSMallocException));

  // Record some unknown exceptions.
  RecordExceptionWithUma(ExceptionNamed(@"CustomName"));
  RecordExceptionWithUma(ExceptionNamed(@"Custom Name"));
  RecordExceptionWithUma(ExceptionNamed(@""));
  RecordExceptionWithUma(nil);

  // We should have exactly the right number of exceptions.
  StatisticsRecorder::GetSnapshot("OSX.NSException", &histograms);
  EXPECT_EQ(1U, histograms.size());
  EXPECT_EQ(HistogramBase::kUmaTargetedHistogramFlag, histograms[0]->flags());

  std::unique_ptr<HistogramSamples> samples(histograms[0]->SnapshotSamples());
  EXPECT_EQ(4, samples->GetCount(0));
  EXPECT_EQ(1, samples->GetCount(1));
  EXPECT_EQ(3, samples->GetCount(2));
  EXPECT_EQ(2, samples->GetCount(3));

  // The unknown exceptions should end up in the overflow bucket.
  EXPECT_TRUE(histograms[0]->HasConstructionArguments(1,
                                                      kUnknownNSException,
                                                      kUnknownNSException + 1));
  EXPECT_EQ(4, samples->GetCount(kUnknownNSException));
}

void RaiseExceptionInRunLoop() {
  CFRunLoopRef run_loop = CFRunLoopGetCurrent();

  CFRunLoopPerformBlock(run_loop, kCFRunLoopCommonModes, ^{
    [NSException raise:@"ThrowExceptionInRunLoop" format:@""];
  });
  CFRunLoopPerformBlock(run_loop, kCFRunLoopCommonModes, ^{
    CFRunLoopStop(run_loop);
  });
  CFRunLoopRun();
}

void ThrowExceptionInRunLoop() {
  base::mac::DisableOSCrashDumps();
  chrome::InstallObjcExceptionPreprocessor();

  RaiseExceptionInRunLoop();

  fprintf(stderr, "TEST FAILED\n");
  exit(1);
}

// Tests that when the preprocessor is installed, exceptions thrown from
// a runloop callout are made fatal, so that the stack trace is useful.
TEST(ExceptionProcessorTest, ThrowExceptionInRunLoop) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_DEATH(ThrowExceptionInRunLoop(),
               ".*FATAL:exception_processor\\.mm.*"
               "Terminating from Objective-C exception:.*");
}

void ThrowAndCatchExceptionInRunLoop() {
  base::mac::DisableOSCrashDumps();
  chrome::InstallObjcExceptionPreprocessor();

  CFRunLoopRef run_loop = CFRunLoopGetCurrent();
  CFRunLoopPerformBlock(run_loop, kCFRunLoopCommonModes, ^{
    @try {
      [NSException raise:@"ObjcExceptionPreprocessCaught" format:@""];
    } @catch (id exception) {
    }
  });

  CFRunLoopPerformBlock(run_loop, kCFRunLoopCommonModes, ^{
    CFRunLoopStop(run_loop);
  });

  CFRunLoopRun();

  fprintf(stderr, "TEST PASS\n");
  exit(0);
}

// Tests that exceptions can still be caught when the preprocessor is enabled.
TEST(ExceptionProcessorTest, ThrowAndCatchExceptionInRunLoop) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_EXIT(ThrowAndCatchExceptionInRunLoop(),
              [](int exit_code) -> bool {
                return WEXITSTATUS(exit_code) == 0;
              },
              ".*TEST PASS.*");
}

void ThrowExceptionFromSelector() {
  base::mac::DisableOSCrashDumps();
  chrome::InstallObjcExceptionPreprocessor();

  NSException* exception = [NSException exceptionWithName:@"ThrowFromSelector"
                                                   reason:@""
                                                 userInfo:nil];

  [exception performSelector:@selector(raise) withObject:nil afterDelay:0.1];

  [[NSRunLoop currentRunLoop] runUntilDate:
      [NSDate dateWithTimeIntervalSinceNow:10]];

  fprintf(stderr, "TEST FAILED\n");
  exit(1);
}

TEST(ExceptionProcessorTest, ThrowExceptionFromSelector) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_DEATH(ThrowExceptionFromSelector(),
               ".*FATAL:exception_processor\\.mm.*"
               "Terminating from Objective-C exception:.*");
}

void ThrowInNotificationObserver() {
  base::mac::DisableOSCrashDumps();
  chrome::InstallObjcExceptionPreprocessor();

  NSNotification* notification =
      [NSNotification notificationWithName:@"TestExceptionInObserver"
                                    object:nil];

  NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
  [center addObserverForName:[notification name]
                      object:nil
                       queue:nil
                  usingBlock:^(NSNotification*) {
                    [NSException raise:@"ThrowInNotificationObserver"
                                format:@""];
                  }];

  [center performSelector:@selector(postNotification:)
               withObject:notification
               afterDelay:0];

  [[NSRunLoop currentRunLoop] runUntilDate:
      [NSDate dateWithTimeIntervalSinceNow:10]];

  fprintf(stderr, "TEST FAILED\n");
  exit(1);
}

TEST(ExceptionProcessorTest, ThrowInNotificationObserver) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_DEATH(ThrowInNotificationObserver(),
               ".*FATAL:exception_processor\\.mm.*"
               "Terminating from Objective-C exception:.*");
}

void ThrowExceptionInRunLoopWithoutProcessor() {
  base::mac::DisableOSCrashDumps();
  chrome::UninstallObjcExceptionPreprocessor();

  @try {
    RaiseExceptionInRunLoop();
  } @catch (id exception) {
    fprintf(stderr, "TEST PASS\n");
    exit(0);
  }

  fprintf(stderr, "TEST FAILED\n");
  exit(1);
}

// Tests basic exception handling when the preprocessor is disabled.
TEST(ExceptionProcessorTest, ThrowExceptionInRunLoopWithoutProcessor) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_EXIT(ThrowExceptionInRunLoopWithoutProcessor(),
              [](int exit_code) -> bool {
                return WEXITSTATUS(exit_code) == 0;
              },
              ".*TEST PASS.*");
}

}  // namespace chrome
