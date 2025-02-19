// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/web_view/test/web_view_test.h"

#import <ChromeWebView/ChromeWebView.h>
#import <Foundation/Foundation.h>

#include "base/base64.h"
#import "base/memory/ptr_util.h"
#include "base/strings/stringprintf.h"
#import "ios/web_view/test/web_view_test_util.h"
#include "net/base/url_util.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "net/test/embedded_test_server/http_request.h"
#include "net/test/embedded_test_server/http_response.h"
#include "testing/gtest/include/gtest/gtest.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {

// Test server path which renders a basic html page.
const char kPageHtmlPath[] = "/PageHtml?";
// URL parameter for html body. Value must be base64 encoded.
const char kPageHtmlBodyParamName[] = "body";
// URL parameter for page title. Value must be base64 encoded.
const char kPageHtmlTitleParamName[] = "title";

// Generates an html response.
std::unique_ptr<net::test_server::HttpResponse> CreatePageHTMLResponse(
    const std::string& title,
    const std::string& body) {
  std::string html = base::StringPrintf(
      "<html><head><title>%s</title></head><body>%s</body></html>",
      title.c_str(), body.c_str());

  auto http_response = base::MakeUnique<net::test_server::BasicHttpResponse>();
  http_response->set_content(html);
  return std::move(http_response);
}

// Returns true if |string| starts with |prefix|. String comparison is case
// insensitive.
bool StartsWith(std::string string, std::string prefix) {
  return base::StartsWith(string, prefix, base::CompareCase::SENSITIVE);
}

// Encodes the |string| for use as the value of a url parameter.
std::string EncodeQueryParamValue(std::string string) {
  std::string encoded_string;
  base::Base64Encode(string, &encoded_string);
  return encoded_string;
}

// Decodes the |encoded_string|. Undoes the encoding performed by
// |EncodeQueryParamValue|.
std::string DecodeQueryParamValue(std::string encoded_string) {
  std::string decoded_string;
  base::Base64Decode(encoded_string, &decoded_string);
  return decoded_string;
}

// Maps test server requests to responses.
std::unique_ptr<net::test_server::HttpResponse> TestRequestHandler(
    const net::test_server::HttpRequest& request) {
  if (StartsWith(request.relative_url, kPageHtmlPath)) {
    std::string title;
    std::string body;

    GURL request_url = request.GetURL();

    std::string encoded_title;
    bool title_found = net::GetValueForKeyInQuery(
        request_url, kPageHtmlTitleParamName, &encoded_title);
    if (title_found) {
      title = DecodeQueryParamValue(encoded_title);
    }

    std::string encoded_body;
    bool body_found = net::GetValueForKeyInQuery(
        request_url, kPageHtmlBodyParamName, &encoded_body);
    if (body_found) {
      body = DecodeQueryParamValue(encoded_body);
    }

    return CreatePageHTMLResponse(title, body);
  }
  return nullptr;
}

}  // namespace

namespace ios_web_view {

WebViewTest::WebViewTest()
    : web_view_(test::CreateWebView()),
      test_server_(base::MakeUnique<net::EmbeddedTestServer>(
          net::test_server::EmbeddedTestServer::TYPE_HTTP)) {
  test_server_->RegisterRequestHandler(base::Bind(&TestRequestHandler));
}

WebViewTest::~WebViewTest() = default;

void WebViewTest::SetUp() {
  PlatformTest::SetUp();
  ASSERT_TRUE(test_server_->Start());
}

GURL WebViewTest::GetUrlForPageWithTitle(const std::string& title) {
  return GetUrlForPageWithTitleAndBody(title, std::string());
}

GURL WebViewTest::GetUrlForPageWithHtmlBody(const std::string& html) {
  return GetUrlForPageWithTitleAndBody(std::string(), html);
}

GURL WebViewTest::GetUrlForPageWithTitleAndBody(const std::string& title,
                                                const std::string& body) {
  GURL url = test_server_->GetURL(kPageHtmlPath);

  // Encode |title| and |body| in url query in order to build the server
  // response later in TestRequestHandler.
  std::string encoded_title = EncodeQueryParamValue(title);
  url = net::AppendQueryParameter(url, kPageHtmlTitleParamName, encoded_title);
  std::string encoded_body = EncodeQueryParamValue(body);
  url = net::AppendQueryParameter(url, kPageHtmlBodyParamName, encoded_body);

  return url;
}

}  // namespace ios_web_view
