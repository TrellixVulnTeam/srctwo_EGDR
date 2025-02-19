// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_PAGE_STATE_H_
#define CONTENT_PUBLIC_COMMON_PAGE_STATE_H_

#include <string>
#include <vector>

#include "content/common/content_export.h"

class GURL;

namespace base {
class FilePath;
}

namespace content {

// The PageState class represents the information needed by the rendering
// engine to reconstruct a web page (and its tree of frames), including for
// example the URLs of the documents and the values of any form fields.  This
// information is used when navigating back & forward through session history.
//
// The format of the encoded data is not exposed by the content API.
class CONTENT_EXPORT PageState {
 public:
  static PageState CreateFromEncodedData(const std::string& data);
  static PageState CreateFromURL(const GURL& url);

  static PageState CreateForTesting(
      const GURL& url,
      bool body_contains_password_data,
      const char* optional_body_data,
      const base::FilePath* optional_body_file_path);

  // Creates an encoded page state from the |url|, |item_sequence_mnumber| and
  // |document_sequence_number| parameters.
  static PageState CreateForTestingWithSequenceNumbers(
      const GURL& url,
      int64_t item_sequence_number,
      int64_t document_sequence_number);

  PageState();

  bool IsValid() const;
  bool Equals(const PageState& page_state) const;
  const std::string& ToEncodedData() const;

  std::vector<base::FilePath> GetReferencedFiles() const;
  PageState RemovePasswordData() const;
  PageState RemoveScrollOffset() const;
  PageState RemoveReferrer() const;

 private:
  PageState(const std::string& data);

  std::string data_;
};

// Support DCHECK_EQ(a, b), etc.
inline bool operator==(const PageState& a, const PageState& b) {
  return a.Equals(b);
}
inline bool operator!=(const PageState& a, const PageState& b) {
  return !(a == b);
}

}  // namespace content

#endif  // CONTENT_PUBLIC_COMMON_PAGE_STATE_H_
