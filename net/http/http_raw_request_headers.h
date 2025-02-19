// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_HTTP_HTTP_RAW_REQUEST_HEADERS_H_
#define NET_HTTP_HTTP_RAW_REQUEST_HEADERS_H_

#include <string>
#include <utility>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "net/base/net_export.h"

namespace net {

// This contains actual headers sent to the remote party, as passed to
// RequestHeadersCallback associated with URLRequest.
// The headers come in actual wire order and include those provided by
// BeforeSendHeaders hooks and headers added or modified by the net stack,
// as well as SPDY & QUIC internal headers (':method' etc).
// In case of non-multiplexed HTTP, request_line also provides the first
// line of the HTTP request (i.e. "METHOD <url> VERSION\r\n").

class NET_EXPORT HttpRawRequestHeaders {
 public:
  using HeaderPair = std::pair<std::string, std::string>;
  using HeaderVector = std::vector<HeaderPair>;

  HttpRawRequestHeaders();
  HttpRawRequestHeaders(HttpRawRequestHeaders&&);
  HttpRawRequestHeaders& operator=(HttpRawRequestHeaders&&);
  ~HttpRawRequestHeaders();

  void Assign(HttpRawRequestHeaders other) { *this = std::move(other); }

  void Add(base::StringPiece key, base::StringPiece value);
  void set_request_line(base::StringPiece line) {
    request_line_ = line.as_string();
  }

  const HeaderVector& headers() const { return headers_; }
  const std::string& request_line() const { return request_line_; }
  bool FindHeaderForTest(base::StringPiece key, std::string* value) const;

 private:
  HeaderVector headers_;
  std::string request_line_;

  DISALLOW_COPY_AND_ASSIGN(HttpRawRequestHeaders);
};

// A callback of this type can be passed to
// URLRequest::SetRequestHeadersCallback to obtain HttpRawRequestHeaders just
// before these hit the socket.
using RequestHeadersCallback =
    base::RepeatingCallback<void(HttpRawRequestHeaders)>;

}  // namespace net

#endif  // NET_HTTP_HTTP_RAW_REQUEST_HEADERS_H_
