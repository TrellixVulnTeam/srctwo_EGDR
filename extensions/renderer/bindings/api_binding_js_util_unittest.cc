// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/renderer/bindings/api_binding_js_util.h"

#include "base/bind.h"
#include "extensions/renderer/bindings/api_binding_test_util.h"
#include "extensions/renderer/bindings/api_bindings_system.h"
#include "extensions/renderer/bindings/api_bindings_system_unittest.h"
#include "gin/arguments.h"
#include "gin/handle.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace extensions {

namespace {

// Calls handleException on |obj|, which is presumed to be the JS binding util.
const char kHandleException[] =
    "try {\n"
    "  throw new Error('some error');\n"
    "} catch (e) {\n"
    "  obj.handleException('handled', e);\n"
    "}";

}  // namespace

class APIBindingJSUtilUnittest : public APIBindingsSystemTest {
 protected:
  APIBindingJSUtilUnittest() {}
  ~APIBindingJSUtilUnittest() override {}

  gin::Handle<APIBindingJSUtil> CreateUtil() {
    return gin::CreateHandle(
        isolate(),
        new APIBindingJSUtil(bindings_system()->type_reference_map(),
                             bindings_system()->request_handler(),
                             bindings_system()->event_handler(),
                             bindings_system()->exception_handler(),
                             base::Bind(&RunFunctionOnGlobalAndIgnoreResult)));
  }

  v8::Local<v8::Object> GetLastErrorParent(
      v8::Local<v8::Context> context,
      v8::Local<v8::Object>* secondary_parent) override {
    return context->Global();
  }

  void AddConsoleError(v8::Local<v8::Context> context,
                       const std::string& error) override {
    console_errors_.push_back(error);
  }

  std::string GetExposedError(v8::Local<v8::Context> context) {
    v8::Local<v8::Value> last_error =
        GetPropertyFromObject(context->Global(), context, "lastError");

    // Use ADD_FAILURE() to avoid messing up the return type with ASSERT.
    if (last_error.IsEmpty()) {
      ADD_FAILURE();
      return std::string();
    }
    if (!last_error->IsObject() && !last_error->IsUndefined()) {
      ADD_FAILURE();
      return std::string();
    }

    if (last_error->IsUndefined())
      return "[undefined]";
    return GetStringPropertyFromObject(last_error.As<v8::Object>(), context,
                                       "message");
  }

  APILastError* last_error() {
    return bindings_system()->request_handler()->last_error();
  }

  const std::vector<std::string>& console_errors() const {
    return console_errors_;
  }

 private:
  std::vector<std::string> console_errors_;

  DISALLOW_COPY_AND_ASSIGN(APIBindingJSUtilUnittest);
};

TEST_F(APIBindingJSUtilUnittest, TestSetLastError) {
  v8::HandleScope handle_scope(isolate());
  v8::Local<v8::Context> context = MainContext();

  gin::Handle<APIBindingJSUtil> util = CreateUtil();
  v8::Local<v8::Object> v8_util = util.ToV8().As<v8::Object>();

  EXPECT_FALSE(last_error()->HasError(context));
  EXPECT_EQ("[undefined]", GetExposedError(context));
  const char kSetLastError[] = "obj.setLastError('a last error');";
  CallFunctionOnObject(context, v8_util, kSetLastError);
  EXPECT_TRUE(last_error()->HasError(context));
  EXPECT_EQ("\"a last error\"", GetExposedError(context));

  CallFunctionOnObject(context, v8_util,
                       "obj.setLastError('a new last error')");
  EXPECT_TRUE(last_error()->HasError(context));
  EXPECT_EQ("\"a new last error\"", GetExposedError(context));
}

TEST_F(APIBindingJSUtilUnittest, TestHasLastError) {
  v8::HandleScope handle_scope(isolate());
  v8::Local<v8::Context> context = MainContext();

  gin::Handle<APIBindingJSUtil> util = CreateUtil();
  v8::Local<v8::Object> v8_util = util.ToV8().As<v8::Object>();

  EXPECT_FALSE(last_error()->HasError(context));
  EXPECT_EQ("[undefined]", GetExposedError(context));
  const char kHasLastError[] = "return obj.hasLastError();";
  v8::Local<v8::Value> has_error =
      CallFunctionOnObject(context, v8_util, kHasLastError);
  EXPECT_EQ("false", V8ToString(has_error, context));

  last_error()->SetError(context, "an error");
  EXPECT_TRUE(last_error()->HasError(context));
  EXPECT_EQ("\"an error\"", GetExposedError(context));
  has_error = CallFunctionOnObject(context, v8_util, kHasLastError);
  EXPECT_EQ("true", V8ToString(has_error, context));

  last_error()->ClearError(context, false);
  EXPECT_FALSE(last_error()->HasError(context));
  EXPECT_EQ("[undefined]", GetExposedError(context));
  has_error = CallFunctionOnObject(context, v8_util, kHasLastError);
  EXPECT_EQ("false", V8ToString(has_error, context));
}

TEST_F(APIBindingJSUtilUnittest, TestRunWithLastError) {
  v8::HandleScope handle_scope(isolate());
  v8::Local<v8::Context> context = MainContext();

  gin::Handle<APIBindingJSUtil> util = CreateUtil();
  v8::Local<v8::Object> v8_util = util.ToV8().As<v8::Object>();

  EXPECT_FALSE(last_error()->HasError(context));
  EXPECT_EQ("[undefined]", GetExposedError(context));

  const char kRunWithLastError[] =
      "obj.runCallbackWithLastError('last error', function() {\n"
      "  this.exposedLastError =\n"
      "      this.lastError ? this.lastError.message : 'undefined';\n"
      "}, [1, 'foo']);";
  CallFunctionOnObject(context, v8_util, kRunWithLastError);

  EXPECT_FALSE(last_error()->HasError(context));
  EXPECT_EQ("[undefined]", GetExposedError(context));
  EXPECT_EQ("\"last error\"",
            GetStringPropertyFromObject(context->Global(), context,
                                        "exposedLastError"));
}

TEST_F(APIBindingJSUtilUnittest, TestSendRequestWithOptions) {
  v8::HandleScope handle_scope(isolate());
  v8::Local<v8::Context> context = MainContext();

  gin::Handle<APIBindingJSUtil> util = CreateUtil();
  v8::Local<v8::Object> v8_util = util.ToV8().As<v8::Object>();

  const char kSendRequestWithNoOptions[] =
      "obj.sendRequest('alpha.functionWithCallback',\n"
      "                ['someString', function() {}], undefined, undefined);";
  CallFunctionOnObject(context, v8_util, kSendRequestWithNoOptions);
  ASSERT_TRUE(last_request());
  EXPECT_EQ("alpha.functionWithCallback", last_request()->method_name);
  EXPECT_EQ("[\"someString\"]", ValueToString(*last_request()->arguments));
  EXPECT_EQ(binding::RequestThread::UI, last_request()->thread);
  reset_last_request();

  const char kSendRequestForIOThread[] =
      "obj.sendRequest('alpha.functionWithCallback',\n"
      "                ['someOtherString', function() {}], undefined,\n"
      "                {__proto__: null, forIOThread: true});";
  CallFunctionOnObject(context, v8_util, kSendRequestForIOThread);
  ASSERT_TRUE(last_request());
  EXPECT_EQ("alpha.functionWithCallback", last_request()->method_name);
  EXPECT_EQ("[\"someOtherString\"]", ValueToString(*last_request()->arguments));
  EXPECT_EQ(binding::RequestThread::IO, last_request()->thread);
  reset_last_request();

  const char kSendRequestForUIThread[] =
      "obj.sendRequest('alpha.functionWithCallback',\n"
      "                ['someOtherString', function() {}], undefined,\n"
      "                {__proto__: null, forIOThread: false});";
  CallFunctionOnObject(context, v8_util, kSendRequestForUIThread);
  ASSERT_TRUE(last_request());
  EXPECT_EQ("alpha.functionWithCallback", last_request()->method_name);
  EXPECT_EQ("[\"someOtherString\"]", ValueToString(*last_request()->arguments));
  EXPECT_EQ(binding::RequestThread::UI, last_request()->thread);
  reset_last_request();

  const char kSendRequestWithCustomCallback[] =
      R"(obj.sendRequest(
             'alpha.functionWithCallback',
             ['stringy', function() {}], undefined,
             {
               __proto__: null,
               customCallback: function() {
                 this.callbackCalled = true;
               },
             });)";
  CallFunctionOnObject(context, v8_util, kSendRequestWithCustomCallback);
  ASSERT_TRUE(last_request());
  EXPECT_EQ("alpha.functionWithCallback", last_request()->method_name);
  EXPECT_EQ("[\"stringy\"]", ValueToString(*last_request()->arguments));
  EXPECT_EQ(binding::RequestThread::UI, last_request()->thread);
  bindings_system()->CompleteRequest(last_request()->request_id,
                                     base::ListValue(), std::string());
  EXPECT_EQ("true", GetStringPropertyFromObject(context->Global(), context,
                                                "callbackCalled"));
}

TEST_F(APIBindingJSUtilUnittest, TestCallHandleException) {
  v8::HandleScope handle_scope(isolate());
  v8::Local<v8::Context> context = MainContext();

  gin::Handle<APIBindingJSUtil> util = CreateUtil();
  v8::Local<v8::Object> v8_util = util.ToV8().As<v8::Object>();

  ASSERT_TRUE(console_errors().empty());
  CallFunctionOnObject(context, v8_util, kHandleException);
  EXPECT_THAT(console_errors(),
              testing::ElementsAre("handled: Error: some error"));

  const char kHandleTrickyException[] =
      "try {\n"
      "  throw { toString: function() { throw new Error('hahaha'); } };\n"
      "} catch (e) {\n"
      "  obj.handleException('handled again', e);\n"
      "}\n";
  CallFunctionOnObject(context, v8_util, kHandleTrickyException);
  EXPECT_THAT(
      console_errors(),
      testing::ElementsAre("handled: Error: some error",
                           "handled again: (failed to get error message)"));
}

TEST_F(APIBindingJSUtilUnittest, TestSetExceptionHandler) {
  v8::HandleScope handle_scope(isolate());
  v8::Local<v8::Context> context = MainContext();

  gin::Handle<APIBindingJSUtil> util = CreateUtil();
  v8::Local<v8::Object> v8_util = util.ToV8().As<v8::Object>();

  struct ErrorInfo {
    std::string full_message;
    std::string exception_message;
  };

  auto custom_handler = [](const v8::FunctionCallbackInfo<v8::Value>& info) {
    gin::Arguments arguments(info);
    std::string full_message;
    ASSERT_TRUE(arguments.GetNext(&full_message));
    v8::Local<v8::Object> error_object;
    ASSERT_TRUE(arguments.GetNext(&error_object));

    ASSERT_TRUE(info.Data()->IsExternal());
    ErrorInfo* error_out =
        static_cast<ErrorInfo*>(info.Data().As<v8::External>()->Value());
    error_out->full_message = full_message;
    error_out->exception_message = GetStringPropertyFromObject(
        error_object, arguments.GetHolderCreationContext(), "message");
  };

  ErrorInfo error_info;
  v8::Local<v8::Function> v8_handler =
      v8::Function::New(context, custom_handler,
                        v8::External::New(isolate(), &error_info))
          .ToLocalChecked();
  v8::Local<v8::Function> add_handler = FunctionFromString(
      context,
      "(function(util, handler) { util.setExceptionHandler(handler); })");
  v8::Local<v8::Value> args[] = {v8_util, v8_handler};
  RunFunction(add_handler, context, arraysize(args), args);

  CallFunctionOnObject(context, v8_util, kHandleException);
  // The error should not have been reported to the console since we have a
  // cusotm handler.
  EXPECT_TRUE(console_errors().empty());
  EXPECT_EQ("handled: Error: some error", error_info.full_message);
  EXPECT_EQ("\"some error\"", error_info.exception_message);
}

}  // namespace extensions
