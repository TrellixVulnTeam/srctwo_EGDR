// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/service_process_util.h"

#import <Foundation/Foundation.h>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/mac/mac_util.h"
#include "base/mac/scoped_nsobject.h"
#include "base/message_loop/message_loop.h"
#include "base/process/launch.h"
#include "base/run_loop.h"
#include "base/strings/sys_string_conversions.h"
#include "base/test/test_timeouts.h"
#include "base/threading/thread.h"
#include "chrome/common/mac/launchd.h"
#include "chrome/common/mac/mock_launchd.h"
#include "testing/gtest/include/gtest/gtest.h"

// Once Chrome no longer supports OSX 10.7, everything within this
// preprocessor block can be removed.
#if !defined(MAC_OS_X_VERSION_10_8) || \
    MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_8
@interface NSFileManager (Chrome)
- (BOOL)trashItemAtURL:(NSURL*)url
      resultingItemURL:(NSURL* __nullable* __nullable)outResultingURL
                 error:(NSError**)error;
@end
#endif

class ServiceProcessStateFileManipulationTest : public ::testing::Test {
 public:
  void TrashFunc(const base::FilePath& src) {
    NSURL* url = [NSURL fileURLWithPath:base::SysUTF8ToNSString(src.value())];
    ASSERT_TRUE(url);
    NSURL* resultingItemURL = nil;
    BOOL success =
        [[NSFileManager defaultManager] trashItemAtURL:url
                                      resultingItemURL:&resultingItemURL
                                                 error:nil];
    ASSERT_TRUE(success);
    trashed_url_.reset([resultingItemURL retain]);
  }

 protected:
  ServiceProcessStateFileManipulationTest()
      : io_thread_("ServiceProcessStateFileManipulationTest_IO") {}

  void SetUp() override {
    base::Thread::Options options;
    options.message_loop_type = base::MessageLoop::TYPE_IO;
    ASSERT_TRUE(io_thread_.StartWithOptions(options));
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
    ASSERT_TRUE(MockLaunchd::MakeABundle(GetTempDirPath(), "Test",
                                         &bundle_path_, &executable_path_));
    mock_launchd_.reset(
        new MockLaunchd(executable_path_, &loop_, false, false));
    scoped_launchd_instance_.reset(
        new Launchd::ScopedInstance(mock_launchd_.get()));
    ASSERT_TRUE(service_process_state_.Initialize());
    ASSERT_TRUE(service_process_state_.SignalReady(
        io_thread_.task_runner().get(), base::Closure()));
    loop_.task_runner()->PostDelayedTask(FROM_HERE,
                                         run_loop_.QuitWhenIdleClosure(),
                                         TestTimeouts::action_max_timeout());
  }

  const MockLaunchd* mock_launchd() const { return mock_launchd_.get(); }
  const base::FilePath& executable_path() const { return executable_path_; }
  const base::FilePath& bundle_path() const { return bundle_path_; }
  const base::FilePath& GetTempDirPath() const { return temp_dir_.GetPath(); }

  base::SingleThreadTaskRunner* GetIOTaskRunner() {
    return io_thread_.task_runner().get();
  }
  void Run() { run_loop_.Run(); }

  base::scoped_nsobject<NSURL> trashed_url_;

 private:
  base::ScopedTempDir temp_dir_;
  base::MessageLoopForUI loop_;
  base::RunLoop run_loop_;
  base::Thread io_thread_;
  base::FilePath executable_path_, bundle_path_;
  std::unique_ptr<MockLaunchd> mock_launchd_;
  std::unique_ptr<Launchd::ScopedInstance> scoped_launchd_instance_;
  ServiceProcessState service_process_state_;
};

void DeleteFunc(const base::FilePath& file) {
  EXPECT_TRUE(base::DeleteFile(file, true));
}

void MoveFunc(const base::FilePath& from, const base::FilePath& to) {
  EXPECT_TRUE(base::Move(from, to));
}

void ChangeAttr(const base::FilePath& from, int mode) {
  EXPECT_EQ(chmod(from.value().c_str(), mode), 0);
}

class ScopedAttributesRestorer {
 public:
  ScopedAttributesRestorer(const base::FilePath& path, int mode)
      : path_(path), mode_(mode) {}
  ~ScopedAttributesRestorer() { ChangeAttr(path_, mode_); }

 private:
  base::FilePath path_;
  int mode_;
};

TEST_F(ServiceProcessStateFileManipulationTest, VerifyLaunchD) {
  // There have been problems where launchd has gotten into a bad state, usually
  // because something had deleted all the files in /tmp. launchd depends on
  // a Unix Domain Socket that it creates at /tmp/launchd*/sock.
  // The symptom of this problem is that the service process connect fails
  // on Mac and "launch_msg(): Socket is not connected" appears.
  // This test is designed to make sure that launchd is working.
  // http://crbug/75518
  // Note: This particular problem no longer affects launchd in 10.10+, since
  // there is no user owned launchd process and sockets are no longer made at
  // /tmp/launchd*/sock. This test is still useful as a sanity check to make
  // sure that launchd appears to be working.

  base::CommandLine cl(base::FilePath("/bin/launchctl"));
  cl.AppendArg("limit");

  std::string output;
  int exit_code = -1;
  ASSERT_TRUE(base::GetAppOutputWithExitCode(cl, &output, &exit_code) &&
              exit_code == 0)
      << " exit_code:" << exit_code << " " << output;
}

TEST_F(ServiceProcessStateFileManipulationTest, DeleteFile) {
  GetIOTaskRunner()->PostTask(FROM_HERE,
                              base::Bind(&DeleteFunc, executable_path()));
  Run();
  ASSERT_TRUE(mock_launchd()->remove_called());
  ASSERT_TRUE(mock_launchd()->delete_called());
}

TEST_F(ServiceProcessStateFileManipulationTest, DeleteBundle) {
  GetIOTaskRunner()->PostTask(FROM_HERE,
                              base::Bind(&DeleteFunc, bundle_path()));
  Run();
  ASSERT_TRUE(mock_launchd()->remove_called());
  ASSERT_TRUE(mock_launchd()->delete_called());
}

TEST_F(ServiceProcessStateFileManipulationTest, MoveBundle) {
  base::FilePath new_loc = GetTempDirPath().AppendASCII("MoveBundle");
  GetIOTaskRunner()->PostTask(FROM_HERE,
                              base::Bind(&MoveFunc, bundle_path(), new_loc));
  Run();
  ASSERT_TRUE(mock_launchd()->restart_called());
  ASSERT_TRUE(mock_launchd()->write_called());
}

TEST_F(ServiceProcessStateFileManipulationTest, MoveFile) {
  base::FilePath new_loc = GetTempDirPath().AppendASCII("MoveFile");
  GetIOTaskRunner()->PostTask(
      FROM_HERE, base::Bind(&MoveFunc, executable_path(), new_loc));
  Run();
  ASSERT_TRUE(mock_launchd()->remove_called());
  ASSERT_TRUE(mock_launchd()->delete_called());
}

TEST_F(ServiceProcessStateFileManipulationTest, TrashBundle) {
  GetIOTaskRunner()->PostTask(
      FROM_HERE, base::Bind(&ServiceProcessStateFileManipulationTest::TrashFunc,
                            base::Unretained(this), bundle_path()));
  Run();
  ASSERT_TRUE(mock_launchd()->remove_called());
  ASSERT_TRUE(mock_launchd()->delete_called());
  std::string path(base::SysNSStringToUTF8([trashed_url_ path]));
  base::FilePath file_path(path);
  ASSERT_TRUE(base::DeleteFile(file_path, true));
}

TEST_F(ServiceProcessStateFileManipulationTest, ChangeAttr) {
  ScopedAttributesRestorer restorer(bundle_path(), 0777);
  GetIOTaskRunner()->PostTask(FROM_HERE,
                              base::Bind(&ChangeAttr, bundle_path(), 0222));
  Run();
  ASSERT_TRUE(mock_launchd()->remove_called());
  ASSERT_TRUE(mock_launchd()->delete_called());
}
