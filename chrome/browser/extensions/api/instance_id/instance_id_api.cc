// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/instance_id/instance_id_api.h"

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/metrics/histogram_macros.h"
#include "chrome/browser/gcm/instance_id/instance_id_profile_service.h"
#include "chrome/browser/gcm/instance_id/instance_id_profile_service_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/extensions/api/instance_id.h"
#include "components/gcm_driver/instance_id/instance_id_driver.h"
#include "extensions/common/extension.h"

namespace extensions {

namespace {

// Error messages.
const char kInvalidParameter[] = "Function was called with invalid parameters.";
const char kDisabled[] = "Instance ID is currently disabled.";
const char kAsyncOperationPending[] = "Asynchronous operation is pending.";
const char kNetworkError[] = "Network error occurred.";
const char kServerError[] = "Server error occurred.";
const char kUnknownError[] = "Unknown error occurred.";

const char* InstanceIDResultToError(instance_id::InstanceID::Result result) {
  switch (result) {
    case instance_id::InstanceID::INVALID_PARAMETER:
      return kInvalidParameter;
    case instance_id::InstanceID::DISABLED:
      return kDisabled;
    case instance_id::InstanceID::ASYNC_OPERATION_PENDING:
      return kAsyncOperationPending;
    case instance_id::InstanceID::NETWORK_ERROR:
      return kNetworkError;
    case instance_id::InstanceID::SERVER_ERROR:
      return kServerError;
    case instance_id::InstanceID::UNKNOWN_ERROR:
      return kUnknownError;
    default:
       NOTREACHED() << "Unexpected value of result cannot be converted: "
                    << result;
  }
  return "";
}

}  // namespace

InstanceIDApiFunction::InstanceIDApiFunction() {
}

InstanceIDApiFunction::~InstanceIDApiFunction() {
}

ExtensionFunction::ResponseAction InstanceIDApiFunction::Run() {
  if (Profile::FromBrowserContext(browser_context())->IsOffTheRecord()) {
    return RespondNow(Error(
        "chrome.instanceID not supported in incognito mode"));
  }

  bool isInstanceIDEnabled = IsEnabled();
  UMA_HISTOGRAM_BOOLEAN("InstanceID.Enabled", isInstanceIDEnabled);

  if (!isInstanceIDEnabled) {
    return RespondNow(Error(
        InstanceIDResultToError(instance_id::InstanceID::DISABLED)));
  }

  return DoWork();
}

bool InstanceIDApiFunction::IsEnabled() const {
  Profile* profile = Profile::FromBrowserContext(browser_context());

  return instance_id::InstanceIDProfileService::IsInstanceIDEnabled(profile);
}

instance_id::InstanceID* InstanceIDApiFunction::GetInstanceID() const {
  return instance_id::InstanceIDProfileServiceFactory::GetForProfile(
      Profile::FromBrowserContext(browser_context()))->driver()->
          GetInstanceID(extension()->id());
}

InstanceIDGetIDFunction::InstanceIDGetIDFunction() {}

InstanceIDGetIDFunction::~InstanceIDGetIDFunction() {}

ExtensionFunction::ResponseAction InstanceIDGetIDFunction::DoWork() {
  GetInstanceID()->GetID(
      base::Bind(&InstanceIDGetIDFunction::GetIDCompleted, this));
  return RespondLater();
}

void InstanceIDGetIDFunction::GetIDCompleted(const std::string& id) {
  Respond(OneArgument(base::MakeUnique<base::Value>(id)));
}

InstanceIDGetCreationTimeFunction::InstanceIDGetCreationTimeFunction() {}

InstanceIDGetCreationTimeFunction::~InstanceIDGetCreationTimeFunction() {}

ExtensionFunction::ResponseAction InstanceIDGetCreationTimeFunction::DoWork() {
  GetInstanceID()->GetCreationTime(
      base::Bind(&InstanceIDGetCreationTimeFunction::GetCreationTimeCompleted,
                 this));
  return RespondLater();
}

void InstanceIDGetCreationTimeFunction::GetCreationTimeCompleted(
    const base::Time& creation_time) {
  Respond(
      OneArgument(base::MakeUnique<base::Value>(creation_time.ToDoubleT())));
}

InstanceIDGetTokenFunction::InstanceIDGetTokenFunction() {}

InstanceIDGetTokenFunction::~InstanceIDGetTokenFunction() {}

ExtensionFunction::ResponseAction InstanceIDGetTokenFunction::DoWork() {
  std::unique_ptr<api::instance_id::GetToken::Params> params =
      api::instance_id::GetToken::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params.get());

  std::map<std::string, std::string> options;
  if (params->get_token_params.options.get())
    options = params->get_token_params.options->additional_properties;

  GetInstanceID()->GetToken(
      params->get_token_params.authorized_entity,
      params->get_token_params.scope,
      options,
      base::Bind(&InstanceIDGetTokenFunction::GetTokenCompleted, this));

  return RespondLater();
}

void InstanceIDGetTokenFunction::GetTokenCompleted(
    const std::string& token,
    instance_id::InstanceID::Result result) {
  if (result == instance_id::InstanceID::SUCCESS)
    Respond(OneArgument(base::MakeUnique<base::Value>(token)));
  else
    Respond(Error(InstanceIDResultToError(result)));
}

InstanceIDDeleteTokenFunction::InstanceIDDeleteTokenFunction() {}

InstanceIDDeleteTokenFunction::~InstanceIDDeleteTokenFunction() {}

ExtensionFunction::ResponseAction InstanceIDDeleteTokenFunction::DoWork() {
  std::unique_ptr<api::instance_id::DeleteToken::Params> params =
      api::instance_id::DeleteToken::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params.get());

  GetInstanceID()->DeleteToken(
      params->delete_token_params.authorized_entity,
      params->delete_token_params.scope,
      base::Bind(&InstanceIDDeleteTokenFunction::DeleteTokenCompleted, this));

  return RespondLater();
}

void InstanceIDDeleteTokenFunction::DeleteTokenCompleted(
    instance_id::InstanceID::Result result) {
  if (result == instance_id::InstanceID::SUCCESS)
    Respond(NoArguments());
  else
    Respond(Error(InstanceIDResultToError(result)));
}

InstanceIDDeleteIDFunction::InstanceIDDeleteIDFunction() {}

InstanceIDDeleteIDFunction::~InstanceIDDeleteIDFunction() {}

ExtensionFunction::ResponseAction InstanceIDDeleteIDFunction::DoWork() {
  GetInstanceID()->DeleteID(
      base::Bind(&InstanceIDDeleteIDFunction::DeleteIDCompleted, this));

  return RespondLater();
}

void InstanceIDDeleteIDFunction::DeleteIDCompleted(
    instance_id::InstanceID::Result result) {
  if (result == instance_id::InstanceID::SUCCESS)
    Respond(NoArguments());
  else
    Respond(Error(InstanceIDResultToError(result)));
}

}  // namespace extensions
