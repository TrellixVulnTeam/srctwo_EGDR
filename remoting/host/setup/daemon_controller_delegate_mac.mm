// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/setup/daemon_controller_delegate_mac.h"

#include <launch.h>
#include <sys/types.h>

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/files/scoped_file.h"
#include "base/logging.h"
#include "base/mac/authorization_util.h"
#include "base/mac/foundation_util.h"
#include "base/mac/launchd.h"
#include "base/mac/mac_logging.h"
#include "base/mac/scoped_authorizationref.h"
#include "base/mac/scoped_launch_data.h"
#include "base/memory/ptr_util.h"
#include "base/posix/eintr_wrapper.h"
#include "base/values.h"
#include "remoting/base/string_resources.h"
#include "remoting/host/host_config.h"
#include "remoting/host/mac/constants_mac.h"
#include "remoting/host/resources.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/l10n/l10n_util_mac.h"

namespace remoting {

namespace {

// Simple RAII class to ensure that waitpid() gets called on a child process.
// Neither std::unique_ptr nor base::ScopedGeneric are well suited, because the
// caller wants to examine the returned status of waitpid() from the scoper's
// deleter function.
class ScopedWaitpid {
 public:
  // -1 is treated as an invalid PID and waitpit() will not be called in this
  // case. Note that -1 is the value returned from
  // base::mac::ExecuteWithPrivilegesAndGetPID() when the child PID could not be
  // determined.
  ScopedWaitpid(pid_t pid) : pid_(pid) {}
  ~ScopedWaitpid() { MaybeWait(); }

  // Executes the waitpid() and resets the scoper. After this, the caller may
  // examine error() and exit_status().
  void Reset() { MaybeWait(); }

  bool error() { return error_; }
  int exit_status() { return exit_status_; }

 private:
  pid_t pid_ = -1;

  // Set if waitpid() failed (returned a value not equal to |pid_|).
  bool error_ = false;

  // The exit status, if waitpid() succeeded.
  int exit_status_ = 0;

  void MaybeWait() {
    if (pid_ != -1) {
      pid_t wait_result = HANDLE_EINTR(waitpid(pid_, &exit_status_, 0));
      if (wait_result != pid_) {
        PLOG(ERROR) << "waitpid failed";
        error_ = true;
      }
      pid_ = -1;
    }
  }
};

// Runs the helper script as root with the given command-line argument.
// If |input_data| is non-empty, it will be piped to the script via standard
// input. Returns true if successful.
bool RunHelperAsRoot(const std::string& command,
                     const std::string& input_data) {
  NSString* prompt = l10n_util::GetNSStringFWithFixup(
      IDS_HOST_AUTHENTICATION_PROMPT,
      l10n_util::GetStringUTF16(IDS_PRODUCT_NAME));
  base::mac::ScopedAuthorizationRef authorization(
      base::mac::AuthorizationCreateToRunAsRoot(base::mac::NSToCFCast(prompt)));
  if (!authorization.get()) {
    LOG(ERROR) << "Failed to obtain authorizationRef";
    return false;
  }

  // TODO(lambroslambrou): Replace the deprecated ExecuteWithPrivileges
  // call with a launchd-based helper tool, which is more secure.
  // http://crbug.com/120903
  const char* arguments[] = {command.c_str(), nullptr};
  FILE* pipe = nullptr;
  pid_t pid;
  OSStatus status = base::mac::ExecuteWithPrivilegesAndGetPID(
      authorization.get(), remoting::kHostHelperScriptPath,
      kAuthorizationFlagDefaults, arguments, &pipe, &pid);
  if (status != errAuthorizationSuccess) {
    LOG(ERROR) << "AuthorizationExecuteWithPrivileges: "
               << logging::DescriptionFromOSStatus(status)
               << static_cast<int>(status);
    return false;
  }

  // It is safer to order the scopers this way round, to ensure that the pipe is
  // closed before calling waitpid(). In the case of sending data to the child,
  // the child reads until EOF on its stdin, so calling waitpid() first would
  // result in deadlock in this situation.
  ScopedWaitpid scoped_pid(pid);
  base::ScopedFILE scoped_pipe(pipe);

  if (pid == -1) {
    LOG(ERROR) << "Failed to get child PID";
    return false;
  }
  if (!pipe) {
    LOG(ERROR) << "Unexpected nullptr pipe";
    return false;
  }

  if (!input_data.empty()) {
    size_t bytes_written =
        fwrite(input_data.data(), sizeof(char), input_data.size(), pipe);
    // According to the fwrite manpage, a partial count is returned only if a
    // write error has occurred.
    if (bytes_written != input_data.size()) {
      LOG(ERROR) << "Failed to write data to child process";
      return false;
    }

    // Flush any buffers here to avoid doing it in fclose(), because the
    // ScopedFILE does not allow checking for errors from fclose().
    if (fflush(pipe) != 0) {
      PLOG(ERROR) << "Failed to flush data to child process";
      return false;
    }
  }

  // Close the pipe (to send EOF) and wait for the child process to run.
  scoped_pipe.reset();
  scoped_pid.Reset();

  if (scoped_pid.error()) {
    PLOG(ERROR) << "waitpid failed";
    return false;
  }
  const int exit_status = scoped_pid.exit_status();
  if (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == 0) {
    return true;
  }

  LOG(ERROR) << remoting::kHostHelperScriptPath << " failed with exit status "
             << exit_status;
  return false;
}

void ElevateAndSetConfig(const base::DictionaryValue& config,
                         const DaemonController::CompletionCallback& done) {
  // Find out if the host service is running.
  pid_t job_pid = base::mac::PIDForJob(remoting::kServiceName);
  bool service_running = (job_pid > 0);

  const char* command = service_running ? "--save-config" : "--enable";
  std::string input_data = HostConfigToJson(config);
  if (!RunHelperAsRoot(command, input_data)) {
    LOG(ERROR) << "Failed to run the helper tool.";
    done.Run(DaemonController::RESULT_FAILED);
    return;
  }

  if (!service_running) {
    base::mac::ScopedLaunchData response(
        base::mac::MessageForJob(remoting::kServiceName, LAUNCH_KEY_STARTJOB));
    if (!response.is_valid()) {
      LOG(ERROR) << "Failed to send STARTJOB to launchd";
      done.Run(DaemonController::RESULT_FAILED);
      return;
    }
  }
  done.Run(DaemonController::RESULT_OK);
}

void ElevateAndStopHost(const DaemonController::CompletionCallback& done) {
  if (!RunHelperAsRoot("--disable", std::string())) {
    LOG(ERROR) << "Failed to run the helper tool.";
    done.Run(DaemonController::RESULT_FAILED);
    return;
  }

  // Stop the launchd job.  This cannot easily be done by the helper tool,
  // since the launchd job runs in the current user's context.
  base::mac::ScopedLaunchData response(
      base::mac::MessageForJob(remoting::kServiceName, LAUNCH_KEY_STOPJOB));
  if (!response.is_valid()) {
    LOG(ERROR) << "Failed to send STOPJOB to launchd";
    done.Run(DaemonController::RESULT_FAILED);
    return;
  }
  done.Run(DaemonController::RESULT_OK);
}

}  // namespace

DaemonControllerDelegateMac::DaemonControllerDelegateMac() {
  LoadResources(std::string());
}

DaemonControllerDelegateMac::~DaemonControllerDelegateMac() {
  UnloadResources();
}

DaemonController::State DaemonControllerDelegateMac::GetState() {
  pid_t job_pid = base::mac::PIDForJob(kServiceName);
  if (job_pid < 0) {
    return DaemonController::STATE_UNKNOWN;
  } else if (job_pid == 0) {
    // Service is stopped, or a start attempt failed.
    return DaemonController::STATE_STOPPED;
  } else {
    return DaemonController::STATE_STARTED;
  }
}

std::unique_ptr<base::DictionaryValue>
DaemonControllerDelegateMac::GetConfig() {
  base::FilePath config_path(kHostConfigFilePath);
  std::unique_ptr<base::DictionaryValue> host_config(
      HostConfigFromJsonFile(config_path));
  if (!host_config)
    return nullptr;

  std::unique_ptr<base::DictionaryValue> config(new base::DictionaryValue);
  std::string value;
  if (host_config->GetString(kHostIdConfigPath, &value))
    config->SetString(kHostIdConfigPath, value);
  if (host_config->GetString(kXmppLoginConfigPath, &value))
    config->SetString(kXmppLoginConfigPath, value);
  return config;
}

void DaemonControllerDelegateMac::SetConfigAndStart(
    std::unique_ptr<base::DictionaryValue> config,
    bool consent,
    const DaemonController::CompletionCallback& done) {
  config->SetBoolean(kUsageStatsConsentConfigPath, consent);
  ElevateAndSetConfig(*config, done);
}

void DaemonControllerDelegateMac::UpdateConfig(
    std::unique_ptr<base::DictionaryValue> config,
    const DaemonController::CompletionCallback& done) {
  base::FilePath config_file_path(kHostConfigFilePath);
  std::unique_ptr<base::DictionaryValue> host_config(
      HostConfigFromJsonFile(config_file_path));
  if (!host_config) {
    done.Run(DaemonController::RESULT_FAILED);
    return;
  }

  host_config->MergeDictionary(config.get());
  ElevateAndSetConfig(*host_config, done);
}

void DaemonControllerDelegateMac::Stop(
    const DaemonController::CompletionCallback& done) {
  ElevateAndStopHost(done);
}

DaemonController::UsageStatsConsent
DaemonControllerDelegateMac::GetUsageStatsConsent() {
  DaemonController::UsageStatsConsent consent;
  consent.supported = true;
  consent.allowed = true;
  // set_by_policy is not yet supported.
  consent.set_by_policy = false;

  base::FilePath config_file_path(kHostConfigFilePath);
  std::unique_ptr<base::DictionaryValue> host_config(
      HostConfigFromJsonFile(config_file_path));
  if (host_config) {
    host_config->GetBoolean(kUsageStatsConsentConfigPath, &consent.allowed);
  }

  return consent;
}

scoped_refptr<DaemonController> DaemonController::Create() {
  return new DaemonController(
      base::WrapUnique(new DaemonControllerDelegateMac()));
}

}  // namespace remoting
