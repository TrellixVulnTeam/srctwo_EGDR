// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/platform/WebURLLoaderTestDelegate.h"

#include "public/platform/WebURLError.h"
#include "public/platform/WebURLLoader.h"
#include "public/platform/WebURLLoaderClient.h"
#include "public/platform/WebURLRequest.h"

namespace blink {

WebURLLoaderTestDelegate::WebURLLoaderTestDelegate() {}

WebURLLoaderTestDelegate::~WebURLLoaderTestDelegate() {}

void WebURLLoaderTestDelegate::DidReceiveResponse(
    WebURLLoaderClient* original_client,
    const WebURLResponse& response) {
  original_client->DidReceiveResponse(response);
}

void WebURLLoaderTestDelegate::DidReceiveData(
    WebURLLoaderClient* original_client,
    const char* data,
    int data_length) {
  original_client->DidReceiveData(data, data_length);
}

void WebURLLoaderTestDelegate::DidFail(WebURLLoaderClient* original_client,
                                       const WebURLError& error,
                                       int64_t total_encoded_data_length,
                                       int64_t total_encoded_body_length,
                                       int64_t total_decoded_body_length) {
  original_client->DidFail(error, total_encoded_data_length,
                           total_encoded_body_length,
                           total_decoded_body_length);
}

void WebURLLoaderTestDelegate::DidFinishLoading(
    WebURLLoaderClient* original_client,
    double finish_time,
    int64_t total_encoded_data_length,
    int64_t total_encoded_body_length,
    int64_t total_decoded_body_length) {
  original_client->DidFinishLoading(finish_time, total_encoded_data_length,
                                    total_encoded_body_length,
                                    total_decoded_body_length);
}

}  // namespace blink
