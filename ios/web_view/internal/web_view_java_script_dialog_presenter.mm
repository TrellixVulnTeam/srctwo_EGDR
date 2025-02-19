// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/web_view/internal/web_view_java_script_dialog_presenter.h"

#import "ios/web_view/public/cwv_ui_delegate.h"
#import "net/base/mac/url_conversions.h"

namespace ios_web_view {

WebViewJavaScriptDialogPresenter::WebViewJavaScriptDialogPresenter(
    CWVWebView* web_view,
    id<CWVUIDelegate> ui_delegate)
    : ui_delegate_(ui_delegate), web_view_(web_view) {}

WebViewJavaScriptDialogPresenter::~WebViewJavaScriptDialogPresenter() = default;

void WebViewJavaScriptDialogPresenter::RunJavaScriptDialog(
    web::WebState* web_state,
    const GURL& origin_url,
    web::JavaScriptDialogType dialog_type,
    NSString* message_text,
    NSString* default_prompt_text,
    const web::DialogClosedCallback& callback) {
  switch (dialog_type) {
    case web::JAVASCRIPT_DIALOG_TYPE_ALERT:
      HandleJavaScriptAlert(origin_url, message_text, callback);
      break;
    case web::JAVASCRIPT_DIALOG_TYPE_CONFIRM:
      HandleJavaScriptConfirmDialog(origin_url, message_text, callback);
      break;
    case web::JAVASCRIPT_DIALOG_TYPE_PROMPT:
      HandleJavaScriptTextPrompt(origin_url, message_text, default_prompt_text,
                                 callback);
      break;
  }
}

void WebViewJavaScriptDialogPresenter::HandleJavaScriptAlert(
    const GURL& origin_url,
    NSString* message_text,
    const web::DialogClosedCallback& callback) {
  if (![ui_delegate_ respondsToSelector:@selector
                     (webView:runJavaScriptAlertPanelWithMessage:pageURL
                                :completionHandler:)]) {
    callback.Run(NO, nil);
    return;
  }
  web::DialogClosedCallback scoped_callback = callback;
  [ui_delegate_ webView:web_view_
      runJavaScriptAlertPanelWithMessage:message_text
                                 pageURL:net::NSURLWithGURL(origin_url)
                       completionHandler:^{
                         if (!scoped_callback.is_null()) {
                           scoped_callback.Run(YES, nil);
                         }
                       }];
}

void WebViewJavaScriptDialogPresenter::HandleJavaScriptConfirmDialog(
    const GURL& origin_url,
    NSString* message_text,
    const web::DialogClosedCallback& callback) {
  if (![ui_delegate_ respondsToSelector:@selector
                     (webView:runJavaScriptConfirmPanelWithMessage:pageURL
                                :completionHandler:)]) {
    callback.Run(NO, nil);
    return;
  }
  web::DialogClosedCallback scoped_callback = callback;
  [ui_delegate_ webView:web_view_
      runJavaScriptConfirmPanelWithMessage:message_text
                                   pageURL:net::NSURLWithGURL(origin_url)
                         completionHandler:^(BOOL is_confirmed) {
                           if (!scoped_callback.is_null()) {
                             scoped_callback.Run(is_confirmed, nil);
                           }
                         }];
}

void WebViewJavaScriptDialogPresenter::HandleJavaScriptTextPrompt(
    const GURL& origin_url,
    NSString* message_text,
    NSString* default_prompt_text,
    const web::DialogClosedCallback& callback) {
  if (![ui_delegate_ respondsToSelector:@selector
                     (webView:runJavaScriptTextInputPanelWithPrompt:defaultText
                                :pageURL:completionHandler:)]) {
    callback.Run(NO, nil);
    return;
  }
  web::DialogClosedCallback scoped_callback = callback;
  [ui_delegate_ webView:web_view_
      runJavaScriptTextInputPanelWithPrompt:message_text
                                defaultText:default_prompt_text
                                    pageURL:net::NSURLWithGURL(origin_url)
                          completionHandler:^(NSString* text_input) {
                            if (!scoped_callback.is_null()) {
                              if (text_input == nil) {
                                scoped_callback.Run(NO, nil);
                              } else {
                                scoped_callback.Run(YES, text_input);
                              }
                            }
                          }];
}

void WebViewJavaScriptDialogPresenter::CancelDialogs(web::WebState* web_state) {
}

void WebViewJavaScriptDialogPresenter::SetUIDelegate(
    id<CWVUIDelegate> ui_delegate) {
  ui_delegate_.reset(ui_delegate);
}

}  // namespace ios_web_view
