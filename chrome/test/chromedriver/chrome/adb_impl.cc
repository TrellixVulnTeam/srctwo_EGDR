// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/chrome/adb_impl.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/json/string_escape.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_tokenizer.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/synchronization/waitable_event.h"
#include "base/time/time.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "chrome/test/chromedriver/net/adb_client_socket.h"

namespace {

// This class is bound in the callback to AdbQuery and isn't freed until the
// callback is run, even if the function that creates the buffer times out.
class ResponseBuffer : public base::RefCountedThreadSafe<ResponseBuffer> {
 public:
  ResponseBuffer()
      : ready_(base::WaitableEvent::ResetPolicy::MANUAL,
               base::WaitableEvent::InitialState::NOT_SIGNALED) {}

  void OnResponse(int result, const std::string& response) {
    response_ = response;
    result_ = result;
    ready_.Signal();
  }

  Status GetResponse(
      std::string* response, const base::TimeDelta& timeout) {
    base::TimeTicks deadline = base::TimeTicks::Now() + timeout;
    while (!ready_.IsSignaled()) {
      base::TimeDelta delta = deadline - base::TimeTicks::Now();
      if (delta <= base::TimeDelta())
        return Status(kTimeout, base::StringPrintf(
            "Adb command timed out after %d seconds",
            static_cast<int>(timeout.InSeconds())));
      ready_.TimedWait(timeout);
    }
    if (result_ < 0)
      return Status(kUnknownError,
          "Failed to run adb command, is the adb server running?");
    *response = response_;
    return Status(kOk);
  }

 private:
  friend class base::RefCountedThreadSafe<ResponseBuffer>;
  ~ResponseBuffer() {}

  std::string response_;
  int result_;
  base::WaitableEvent ready_;
};

void ExecuteCommandOnIOThread(
    const std::string& command, scoped_refptr<ResponseBuffer> response_buffer,
    int port) {
  CHECK(base::MessageLoopForIO::IsCurrent());
  AdbClientSocket::AdbQuery(port, command,
      base::Bind(&ResponseBuffer::OnResponse, response_buffer));
}

void SendFileOnIOThread(const std::string& device_serial,
                        const std::string& filename,
                        const std::string& content,
                        scoped_refptr<ResponseBuffer> response_buffer,
                        int port) {
  CHECK(base::MessageLoopForIO::IsCurrent());
  AdbClientSocket::SendFile(
      port, device_serial, filename, content,
      base::Bind(&ResponseBuffer::OnResponse, response_buffer));
}

}  // namespace

AdbImpl::AdbImpl(
    const scoped_refptr<base::SingleThreadTaskRunner>& io_task_runner,
    int port)
    : io_task_runner_(io_task_runner), port_(port) {
  CHECK(io_task_runner_.get());
}

AdbImpl::~AdbImpl() {}

Status AdbImpl::GetDevices(std::vector<std::string>* devices) {
  std::string response;
  Status status = ExecuteCommand("host:devices", &response);
  if (!status.IsOk())
    return status;
  base::StringTokenizer lines(response, "\n");
  while (lines.GetNext()) {
    std::vector<std::string> fields = base::SplitString(
        lines.token_piece(), base::kWhitespaceASCII,
        base::KEEP_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
    if (fields.size() == 2 && fields[1] == "device") {
      devices->push_back(fields[0]);
    }
  }
  return Status(kOk);
}

Status AdbImpl::ForwardPort(
    const std::string& device_serial, int local_port,
    const std::string& remote_abstract) {
  std::string response;
  Status status = ExecuteHostCommand(
      device_serial,
      "forward:tcp:" + base::IntToString(local_port) + ";localabstract:" +
          remote_abstract,
      &response);
  if (!status.IsOk())
    return status;
  if (response == "OKAY")
    return Status(kOk);
  return Status(kUnknownError, "Failed to forward ports to device " +
                device_serial + ": " + response);
}

Status AdbImpl::SetCommandLineFile(const std::string& device_serial,
                                   const std::string& command_line_file,
                                   const std::string& exec_name,
                                   const std::string& args) {
  std::string response;
  std::string command(exec_name + " " + args + "\n");
  scoped_refptr<ResponseBuffer> response_buffer = new ResponseBuffer;
  VLOG(1) << "Sending command line file: " << command_line_file;
  io_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&SendFileOnIOThread, device_serial, command_line_file,
                     command, response_buffer, port_));
  Status status =
      response_buffer->GetResponse(&response, base::TimeDelta::FromSeconds(30));
  return status;
}

Status AdbImpl::CheckAppInstalled(
    const std::string& device_serial, const std::string& package) {
  std::string response;
  std::string command = "pm path " + package;
  Status status = ExecuteHostShellCommand(device_serial, command, &response);
  if (!status.IsOk())
    return status;
  if (response.find("package") == std::string::npos)
    return Status(kUnknownError, package + " is not installed on device " +
                  device_serial);
  return Status(kOk);
}

Status AdbImpl::ClearAppData(
    const std::string& device_serial, const std::string& package) {
  std::string response;
  std::string command = "pm clear " + package;
  Status status = ExecuteHostShellCommand(device_serial, command, &response);
  if (!status.IsOk())
    return status;
  if (response.find("Success") == std::string::npos)
    return Status(kUnknownError, "Failed to clear data for " + package +
                  " on device " + device_serial + ": " + response);
  return Status(kOk);
}

Status AdbImpl::SetDebugApp(
    const std::string& device_serial, const std::string& package) {
  std::string response;
  return ExecuteHostShellCommand(
      device_serial, "am set-debug-app --persistent " + package, &response);
}

Status AdbImpl::Launch(
    const std::string& device_serial, const std::string& package,
    const std::string& activity) {
  std::string response;
  Status status = ExecuteHostShellCommand(
      device_serial,
      "am start -W -n " + package + "/" + activity + " -d data:,",
      &response);
  if (!status.IsOk())
    return status;
  if (response.find("Complete") == std::string::npos)
    return Status(kUnknownError,
                  "Failed to start " + package + " on device " + device_serial +
                  ": " + response);
  return Status(kOk);
}

Status AdbImpl::ForceStop(
    const std::string& device_serial, const std::string& package) {
  std::string response;
  return ExecuteHostShellCommand(
      device_serial, "am force-stop " + package, &response);
}

Status AdbImpl::GetPidByName(const std::string& device_serial,
                             const std::string& process_name,
                             int* pid) {
  std::string response;
  Status status = ExecuteHostShellCommand(device_serial, "ps", &response);
  if (!status.IsOk())
    return status;

  for (const base::StringPiece& line : base::SplitString(
           response, "\n", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY)) {
    std::vector<base::StringPiece> tokens = base::SplitStringPiece(
        line, base::kWhitespaceASCII,
        base::KEEP_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
    if (tokens.size() != 8 && tokens.size() != 9)
      continue;
    // The ps command on Android M+ does not always output a value for WCHAN,
    // so the process name might appear in the 8th or 9th column. Use the
    // right-most column for process name.
    if (tokens[tokens.size() - 1] == process_name) {
      if (base::StringToInt(tokens[1], pid)) {
        return Status(kOk);
      } else {
        break;
      }
    }
  }

  return Status(kUnknownError,
                "Failed to get PID for the following process: " + process_name);
}

Status AdbImpl::ExecuteCommand(
    const std::string& command, std::string* response) {
  scoped_refptr<ResponseBuffer> response_buffer = new ResponseBuffer;
  VLOG(1) << "Sending adb command: " << command;
  io_task_runner_->PostTask(FROM_HERE,
                            base::BindOnce(&ExecuteCommandOnIOThread, command,
                                           response_buffer, port_));
  Status status = response_buffer->GetResponse(
      response, base::TimeDelta::FromSeconds(30));
  if (status.IsOk()) {
    VLOG(1) << "Received adb response: " << *response;
  }
  return status;
}

Status AdbImpl::ExecuteHostCommand(
    const std::string& device_serial,
    const std::string& host_command, std::string* response) {
  return ExecuteCommand(
      "host-serial:" + device_serial + ":" + host_command, response);
}

Status AdbImpl::ExecuteHostShellCommand(
    const std::string& device_serial,
    const std::string& shell_command,
    std::string* response) {
  return ExecuteCommand(
      "host:transport:" + device_serial + "|shell:" + shell_command,
      response);
}

