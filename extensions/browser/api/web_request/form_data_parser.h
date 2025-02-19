// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_API_WEB_REQUEST_FORM_DATA_PARSER_H_
#define EXTENSIONS_BROWSER_API_WEB_REQUEST_FORM_DATA_PARSER_H_

#include <memory>
#include <string>

#include "base/macros.h"
// Cannot forward declare StringPiece because it is a typedef.
#include "base/strings/string_piece.h"

namespace net {
class URLRequest;
}

namespace extensions {

// Interface for the form data parsers.
class FormDataParser {
 public:
  // Result encapsulates name-value pairs returned by GetNextNameValue.
  class Result {
   public:
    Result();
    ~Result();

    const std::string& name() const { return name_; }
    const std::string& value() const { return value_; }

    void set_name(base::StringPiece str) { str.CopyToString(&name_); }
    void set_value(base::StringPiece str) { str.CopyToString(&value_); }

   private:
    std::string name_;
    std::string value_;

    DISALLOW_COPY_AND_ASSIGN(Result);
  };

  virtual ~FormDataParser();

  // Creates a correct parser instance based on the |request|. Returns NULL
  // on failure.
  static std::unique_ptr<FormDataParser> Create(const net::URLRequest& request);

  // Creates a correct parser instance based on |content_type_header|, the
  // "Content-Type" request header value. If |content_type_header| is NULL, it
  // defaults to "application/x-www-form-urlencoded". Returns NULL on failure.
  static std::unique_ptr<FormDataParser> CreateFromContentTypeHeader(
      const std::string* content_type_header);

  // Returns true if there was some data, it was well formed and all was read.
  virtual bool AllDataReadOK() = 0;

  // Gets the next name-value pair from the source data and stores it in
  // |result|. Returns true if a pair was found. Callers must have previously
  // succesfully called the SetSource method.
  virtual bool GetNextNameValue(Result* result) = 0;

  // Sets the |source| of the data to be parsed. One form data parser is only
  // expected to be associated with one source, so generally, SetSource should
  // be only called once. However, for technical reasons, the source might only
  // be available in chunks for multipart encoded forms, in which case it is OK
  // to call SetSource multiple times to add all chunks of a single source. The
  // ownership of |source| is left with the caller and the source should live
  // until |this| dies or |this->SetSource()| is called again, whichever comes
  // sooner. Returns true on success.
  virtual bool SetSource(base::StringPiece source) = 0;

 protected:
  FormDataParser();

 private:
  DISALLOW_COPY_AND_ASSIGN(FormDataParser);
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_WEB_REQUEST_FORM_DATA_PARSER_H_
