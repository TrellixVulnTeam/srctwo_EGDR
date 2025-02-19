// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/api_test_utils.h"

#include <memory>
#include <utility>

#include "base/callback_helpers.h"
#include "base/json/json_reader.h"
#include "base/memory/ptr_util.h"
#include "base/values.h"
#include "components/crx_file/id_util.h"
#include "content/public/browser/browser_context.h"
#include "content/public/test/test_utils.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_dispatcher.h"
#include "extensions/common/extension_builder.h"
#include "testing/gtest/include/gtest/gtest.h"

using extensions::ExtensionFunctionDispatcher;

namespace {

std::unique_ptr<base::Value> ParseJSON(const std::string& data) {
  return base::JSONReader::Read(data);
}

std::unique_ptr<base::ListValue> ParseList(const std::string& data) {
  return base::ListValue::From(ParseJSON(data));
}

}  // namespace

namespace extensions {

namespace api_test_utils {

SendResponseHelper::SendResponseHelper(UIThreadExtensionFunction* function) {
  function->set_has_callback(true);
  function->set_response_callback(
      base::Bind(&SendResponseHelper::OnResponse, base::Unretained(this)));
}

SendResponseHelper::~SendResponseHelper() {}

bool SendResponseHelper::GetResponse() {
  EXPECT_TRUE(has_response());
  return *response_;
}

void SendResponseHelper::OnResponse(ExtensionFunction::ResponseType response,
                                    const base::ListValue& results,
                                    const std::string& error,
                                    functions::HistogramValue histogram_value) {
  ASSERT_NE(ExtensionFunction::BAD_MESSAGE, response);
  response_.reset(new bool(response == ExtensionFunction::SUCCEEDED));
  run_loop_.Quit();
}

void SendResponseHelper::WaitForResponse() {
  run_loop_.Run();
}

std::unique_ptr<base::DictionaryValue> ParseDictionary(
    const std::string& data) {
  return base::DictionaryValue::From(ParseJSON(data));
}

bool GetBoolean(const base::DictionaryValue* val, const std::string& key) {
  bool result = false;
  if (!val->GetBoolean(key, &result))
    ADD_FAILURE() << key << " does not exist or is not a boolean.";
  return result;
}

int GetInteger(const base::DictionaryValue* val, const std::string& key) {
  int result = 0;
  if (!val->GetInteger(key, &result))
    ADD_FAILURE() << key << " does not exist or is not an integer.";
  return result;
}

std::string GetString(const base::DictionaryValue* val,
                      const std::string& key) {
  std::string result;
  if (!val->GetString(key, &result))
    ADD_FAILURE() << key << " does not exist or is not a string.";
  return result;
}

scoped_refptr<Extension> CreateExtension(
    Manifest::Location location,
    base::DictionaryValue* test_extension_value,
    const std::string& id_input) {
  std::string error;
  const base::FilePath test_extension_path;
  std::string id;
  if (!id_input.empty())
    id = crx_file::id_util::GenerateId(id_input);
  scoped_refptr<Extension> extension(
      Extension::Create(test_extension_path, location, *test_extension_value,
                        Extension::NO_FLAGS, id, &error));
  EXPECT_TRUE(error.empty()) << "Could not parse test extension " << error;
  return extension;
}

scoped_refptr<Extension> CreateExtension(
    base::DictionaryValue* test_extension_value) {
  return CreateExtension(Manifest::INTERNAL, test_extension_value,
                         std::string());
}

scoped_refptr<Extension> CreateEmptyExtensionWithLocation(
    Manifest::Location location) {
  std::unique_ptr<base::DictionaryValue> test_extension_value =
      ParseDictionary("{\"name\": \"Test\", \"version\": \"1.0\"}");
  return CreateExtension(location, test_extension_value.get(), std::string());
}

std::unique_ptr<base::Value> RunFunctionWithDelegateAndReturnSingleResult(
    scoped_refptr<UIThreadExtensionFunction> function,
    const std::string& args,
    content::BrowserContext* context,
    std::unique_ptr<extensions::ExtensionFunctionDispatcher> dispatcher) {
  return RunFunctionWithDelegateAndReturnSingleResult(
      function, args, context, std::move(dispatcher), NONE);
}

std::unique_ptr<base::Value> RunFunctionWithDelegateAndReturnSingleResult(
    scoped_refptr<UIThreadExtensionFunction> function,
    const std::string& args,
    content::BrowserContext* context,
    std::unique_ptr<extensions::ExtensionFunctionDispatcher> dispatcher,
    RunFunctionFlags flags) {
  std::unique_ptr<base::ListValue> parsed_args = ParseList(args);
  EXPECT_TRUE(parsed_args.get())
      << "Could not parse extension function arguments: " << args;

  return RunFunctionWithDelegateAndReturnSingleResult(
      function, std::move(parsed_args), context, std::move(dispatcher), flags);
}

std::unique_ptr<base::Value> RunFunctionWithDelegateAndReturnSingleResult(
    scoped_refptr<UIThreadExtensionFunction> function,
    std::unique_ptr<base::ListValue> args,
    content::BrowserContext* context,
    std::unique_ptr<extensions::ExtensionFunctionDispatcher> dispatcher,
    RunFunctionFlags flags) {
  RunFunction(function.get(), std::move(args), context, std::move(dispatcher),
              flags);
  EXPECT_TRUE(function->GetError().empty()) << "Unexpected error: "
                                            << function->GetError();
  const base::Value* single_result = NULL;
  if (function->GetResultList() != NULL &&
      function->GetResultList()->Get(0, &single_result)) {
    return single_result->CreateDeepCopy();
  }
  return NULL;
}

std::unique_ptr<base::Value> RunFunctionAndReturnSingleResult(
    UIThreadExtensionFunction* function,
    const std::string& args,
    content::BrowserContext* context) {
  return RunFunctionAndReturnSingleResult(function, args, context, NONE);
}

std::unique_ptr<base::Value> RunFunctionAndReturnSingleResult(
    UIThreadExtensionFunction* function,
    const std::string& args,
    content::BrowserContext* context,
    RunFunctionFlags flags) {
  std::unique_ptr<ExtensionFunctionDispatcher> dispatcher(
      new ExtensionFunctionDispatcher(context));

  return RunFunctionWithDelegateAndReturnSingleResult(
      function, args, context, std::move(dispatcher), flags);
}

std::string RunFunctionAndReturnError(UIThreadExtensionFunction* function,
                                      const std::string& args,
                                      content::BrowserContext* context) {
  return RunFunctionAndReturnError(function, args, context, NONE);
}

std::string RunFunctionAndReturnError(UIThreadExtensionFunction* function,
                                      const std::string& args,
                                      content::BrowserContext* context,
                                      RunFunctionFlags flags) {
  std::unique_ptr<ExtensionFunctionDispatcher> dispatcher(
      new ExtensionFunctionDispatcher(context));
  scoped_refptr<ExtensionFunction> function_owner(function);
  // Without a callback the function will not generate a result.
  RunFunction(function, args, context, std::move(dispatcher), flags);
  // When sending a response, the function will set an empty list value if there
  // is no specified result.
  const base::ListValue* results = function->GetResultList();
  CHECK(results);
  EXPECT_TRUE(results->empty()) << "Did not expect a result";
  CHECK(function->response_type());
  EXPECT_EQ(ExtensionFunction::FAILED, *function->response_type());
  return function->GetError();
}

bool RunFunction(UIThreadExtensionFunction* function,
                 const std::string& args,
                 content::BrowserContext* context) {
  std::unique_ptr<ExtensionFunctionDispatcher> dispatcher(
      new ExtensionFunctionDispatcher(context));
  return RunFunction(function, args, context, std::move(dispatcher), NONE);
}

bool RunFunction(
    UIThreadExtensionFunction* function,
    const std::string& args,
    content::BrowserContext* context,
    std::unique_ptr<extensions::ExtensionFunctionDispatcher> dispatcher,
    RunFunctionFlags flags) {
  std::unique_ptr<base::ListValue> parsed_args = ParseList(args);
  EXPECT_TRUE(parsed_args.get())
      << "Could not parse extension function arguments: " << args;
  return RunFunction(function, std::move(parsed_args), context,
                     std::move(dispatcher), flags);
}

bool RunFunction(
    UIThreadExtensionFunction* function,
    std::unique_ptr<base::ListValue> args,
    content::BrowserContext* context,
    std::unique_ptr<extensions::ExtensionFunctionDispatcher> dispatcher,
    RunFunctionFlags flags) {
  SendResponseHelper response_helper(function);
  function->SetArgs(args.get());

  CHECK(dispatcher);
  function->set_dispatcher(dispatcher->AsWeakPtr());

  function->set_browser_context(context);
  function->set_include_incognito(flags & INCLUDE_INCOGNITO);
  function->RunWithValidation()->Execute();
  response_helper.WaitForResponse();

  EXPECT_TRUE(response_helper.has_response());
  return response_helper.GetResponse();
}

}  // namespace api_test_utils
}  // namespace extensions
