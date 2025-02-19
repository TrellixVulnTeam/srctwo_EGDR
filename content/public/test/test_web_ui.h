// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_TEST_TEST_WEB_UI_H_
#define CONTENT_PUBLIC_TEST_TEST_WEB_UI_H_

#include <memory>
#include <vector>

#include "base/values.h"
#include "content/public/browser/web_ui.h"

namespace content {

// Test instance of WebUI that tracks the data passed to
// CallJavascriptFunctionUnsafe().
class TestWebUI : public WebUI {
 public:
  TestWebUI();
  ~TestWebUI() override;

  void ClearTrackedCalls();
  void set_web_contents(WebContents* web_contents) {
    web_contents_ = web_contents;
  }

  // WebUI overrides.
  WebContents* GetWebContents() const override;
  WebUIController* GetController() const override;
  void SetController(WebUIController* controller) override {}
  float GetDeviceScaleFactor() const override;
  const base::string16& GetOverriddenTitle() const override;
  void OverrideTitle(const base::string16& title) override {}
  int GetBindings() const override;
  void SetBindings(int bindings) override {}
  bool HasRenderFrame() override;
  void AddMessageHandler(std::unique_ptr<WebUIMessageHandler> handler) override;
  void RegisterMessageCallback(const std::string& message,
                               const MessageCallback& callback) override {}
  void ProcessWebUIMessage(const GURL& source_url,
                           const std::string& message,
                           const base::ListValue& args) override {}
  bool CanCallJavascript() override;
  void CallJavascriptFunctionUnsafe(const std::string& function_name) override;
  void CallJavascriptFunctionUnsafe(const std::string& function_name,
                                    const base::Value& arg1) override;
  void CallJavascriptFunctionUnsafe(const std::string& function_name,
                                    const base::Value& arg1,
                                    const base::Value& arg2) override;
  void CallJavascriptFunctionUnsafe(const std::string& function_name,
                                    const base::Value& arg1,
                                    const base::Value& arg2,
                                    const base::Value& arg3) override;
  void CallJavascriptFunctionUnsafe(const std::string& function_name,
                                    const base::Value& arg1,
                                    const base::Value& arg2,
                                    const base::Value& arg3,
                                    const base::Value& arg4) override;
  void CallJavascriptFunctionUnsafe(
      const std::string& function_name,
      const std::vector<const base::Value*>& args) override;
  std::vector<std::unique_ptr<WebUIMessageHandler>>* GetHandlersForTesting()
      override;

  class CallData {
   public:
    explicit CallData(const std::string& function_name);
    ~CallData();

    void TakeAsArg1(std::unique_ptr<base::Value> arg);
    void TakeAsArg2(std::unique_ptr<base::Value> arg);
    void TakeAsArg3(std::unique_ptr<base::Value> arg);
    void TakeAsArg4(std::unique_ptr<base::Value> arg);

    const std::string& function_name() const { return function_name_; }
    const base::Value* arg1() const { return arg1_.get(); }
    const base::Value* arg2() const { return arg2_.get(); }
    const base::Value* arg3() const { return arg3_.get(); }
    const base::Value* arg4() const { return arg4_.get(); }

   private:
    std::string function_name_;
    std::unique_ptr<base::Value> arg1_;
    std::unique_ptr<base::Value> arg2_;
    std::unique_ptr<base::Value> arg3_;
    std::unique_ptr<base::Value> arg4_;
  };

  const std::vector<std::unique_ptr<CallData>>& call_data() const {
    return call_data_;
  }

 private:
  std::vector<std::unique_ptr<CallData>> call_data_;
  std::vector<std::unique_ptr<WebUIMessageHandler>> handlers_;
  base::string16 temp_string_;
  WebContents* web_contents_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_TEST_TEST_WEB_UI_H_
