// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_BROWSER_URL_REWRITER_IMPL_H_
#define IOS_WEB_BROWSER_URL_REWRITER_IMPL_H_

#include <vector>

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "ios/web/public/browser_url_rewriter.h"

class GURL;

namespace web {

// Concrete subclass of web::BrowserURLRewriter.  Stores added URLRewriters in a
// vector and performs URL rewrites if necessary.
class BrowserURLRewriterImpl : public BrowserURLRewriter {
 public:
  // Returns the singleton instance.
  static BrowserURLRewriterImpl* GetInstance();

  // BrowserURLRewriter implementation:
  bool RewriteURLIfNecessary(GURL* url, BrowserState* browser_state) override;
  void AddURLRewriter(URLRewriter rewriter) override;

 private:
  // This object is a singleton:
  BrowserURLRewriterImpl();
  ~BrowserURLRewriterImpl() override;
  friend struct base::DefaultSingletonTraits<BrowserURLRewriterImpl>;

  // The list of known URLRewriters.
  std::vector<URLRewriter> url_rewriters_;

  DISALLOW_COPY_AND_ASSIGN(BrowserURLRewriterImpl);
};

}  // namespace web

#endif  // IOS_WEB_BROWSER_URL_REWRITER_IMPL_H_
