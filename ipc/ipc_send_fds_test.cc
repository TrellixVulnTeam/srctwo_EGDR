// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"

#if defined(OS_POSIX)
#if defined(OS_MACOSX)
extern "C" {
#include <sandbox.h>
};
#endif
#include <fcntl.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>
#include <queue>

#include "base/callback.h"
#include "base/file_descriptor_posix.h"
#include "base/location.h"
#include "base/pickle.h"
#include "base/posix/eintr_wrapper.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "base/threading/thread_task_runner_handle.h"
#include "ipc/ipc_message_attachment_set.h"
#include "ipc/ipc_message_utils.h"
#include "ipc/ipc_test_base.h"

#if defined(OS_POSIX)
#include "base/macros.h"
#endif

#if defined(OS_MACOSX)
#include "sandbox/mac/seatbelt.h"
#endif

namespace {

const unsigned kNumFDsToSend = 7;  // per message
const unsigned kNumMessages = 20;
const char* kDevZeroPath = "/dev/zero";

#if defined(OS_POSIX)
static_assert(kNumFDsToSend ==
                  IPC::MessageAttachmentSet::kMaxDescriptorsPerMessage,
              "The number of FDs to send must be kMaxDescriptorsPerMessage.");
#endif

class MyChannelDescriptorListenerBase : public IPC::Listener {
 public:
  bool OnMessageReceived(const IPC::Message& message) override {
    base::PickleIterator iter(message);
    base::FileDescriptor descriptor;
    while (IPC::ParamTraits<base::FileDescriptor>::Read(
               &message, &iter, &descriptor)) {
      HandleFD(descriptor.fd);
    }
    return true;
  }

 protected:
  virtual void HandleFD(int fd) = 0;
};

class MyChannelDescriptorListener : public MyChannelDescriptorListenerBase {
 public:
  explicit MyChannelDescriptorListener(ino_t expected_inode_num)
      : MyChannelDescriptorListenerBase(),
        expected_inode_num_(expected_inode_num),
        num_fds_received_(0) {
  }

  unsigned num_fds_received() const {
    return num_fds_received_;
  }

  void OnChannelError() override {
    base::RunLoop::QuitCurrentWhenIdleDeprecated();
  }

 protected:
  void HandleFD(int fd) override {
    ASSERT_GE(fd, 0);
    // Check that we can read from the FD.
    char buf;
    ssize_t amt_read = read(fd, &buf, 1);
    ASSERT_EQ(amt_read, 1);
    ASSERT_EQ(buf, 0);  // /dev/zero always reads 0 bytes.

    struct stat st;
    ASSERT_EQ(fstat(fd, &st), 0);

    ASSERT_EQ(close(fd), 0);

    // Compare inode numbers to check that the file sent over the wire is
    // actually the one expected.
    ASSERT_EQ(expected_inode_num_, st.st_ino);

    ++num_fds_received_;
    if (num_fds_received_ == kNumFDsToSend * kNumMessages)
      base::RunLoop::QuitCurrentWhenIdleDeprecated();
  }

 private:
  ino_t expected_inode_num_;
  unsigned num_fds_received_;
};

class IPCSendFdsTest : public IPCChannelMojoTestBase {
 protected:
  void RunServer() {
    // Set up IPC channel and start client.
    MyChannelDescriptorListener listener(-1);
    CreateChannel(&listener);
    ASSERT_TRUE(ConnectChannel());

    for (unsigned i = 0; i < kNumMessages; ++i) {
      IPC::Message* message =
          new IPC::Message(0, 3, IPC::Message::PRIORITY_NORMAL);
      for (unsigned j = 0; j < kNumFDsToSend; ++j) {
        const int fd = open(kDevZeroPath, O_RDONLY);
        ASSERT_GE(fd, 0);
        base::FileDescriptor descriptor(fd, true);
        IPC::ParamTraits<base::FileDescriptor>::Write(message, descriptor);
      }
      ASSERT_TRUE(sender()->Send(message));
    }

    // Run message loop.
    base::RunLoop().Run();

    // Close the channel so the client's OnChannelError() gets fired.
    channel()->Close();

    EXPECT_TRUE(WaitForClientShutdown());
    DestroyChannel();
  }
};

TEST_F(IPCSendFdsTest, DescriptorTest) {
  Init("SendFdsClient");
  RunServer();
}

class SendFdsTestClientFixture : public IpcChannelMojoTestClient {
 protected:
  void SendFdsClientCommon(const std::string& test_client_name,
                           ino_t expected_inode_num) {
    MyChannelDescriptorListener listener(expected_inode_num);

    // Set up IPC channel.
    Connect(&listener);

    // Run message loop.
    base::RunLoop().Run();

    // Verify that the message loop was exited due to getting the correct number
    // of descriptors, and not because of the channel closing unexpectedly.
    EXPECT_EQ(kNumFDsToSend * kNumMessages, listener.num_fds_received());

    Close();
  }
};

DEFINE_IPC_CHANNEL_MOJO_TEST_CLIENT_WITH_CUSTOM_FIXTURE(
    SendFdsClient,
    SendFdsTestClientFixture) {
  struct stat st;
  int fd = open(kDevZeroPath, O_RDONLY);
  fstat(fd, &st);
  EXPECT_GE(IGNORE_EINTR(close(fd)), 0);
  SendFdsClientCommon("SendFdsClient", st.st_ino);
}

#if defined(OS_MACOSX)
// Test that FDs are correctly sent to a sandboxed process.
// TODO(port): Make this test cross-platform.
TEST_F(IPCSendFdsTest, DescriptorTestSandboxed) {
  Init("SendFdsSandboxedClient");
  RunServer();
}

DEFINE_IPC_CHANNEL_MOJO_TEST_CLIENT_WITH_CUSTOM_FIXTURE(
    SendFdsSandboxedClient,
    SendFdsTestClientFixture) {
  struct stat st;
  const int fd = open(kDevZeroPath, O_RDONLY);
  fstat(fd, &st);
  ASSERT_LE(0, IGNORE_EINTR(close(fd)));

  // Enable the sandbox.
  char* error_buff = NULL;
  int error = sandbox::Seatbelt::Init(
      sandbox::Seatbelt::kProfilePureComputation, SANDBOX_NAMED, &error_buff);
  ASSERT_EQ(0, error);
  ASSERT_FALSE(error_buff);

  sandbox::Seatbelt::FreeError(error_buff);

  // Make sure sandbox is really enabled.
  ASSERT_EQ(-1, open(kDevZeroPath, O_RDONLY))
      << "Sandbox wasn't properly enabled";

  // See if we can receive a file descriptor.
  SendFdsClientCommon("SendFdsSandboxedClient", st.st_ino);
}
#endif  // defined(OS_MACOSX)

}  // namespace

#endif  // defined(OS_POSIX)
