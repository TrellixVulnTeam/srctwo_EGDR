// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_NET_HTTP_CACHE_HELPER_H_
#define IOS_NET_HTTP_CACHE_HELPER_H_

#include "base/callback_forward.h"
#include "base/memory/ref_counted.h"
#include "net/base/completion_callback.h"

namespace base {
class TaskRunner;
}

namespace net {
class URLRequestContextGetter;

// Clears the HTTP cache and calls |closure| back.
void ClearHttpCache(const scoped_refptr<net::URLRequestContextGetter>& getter,
                    const scoped_refptr<base::TaskRunner>& network_task_runner,
                    const base::Time& delete_begin,
                    const base::Time& delete_end,
                    const net::CompletionCallback& callback);

}  // namespace net

#endif  // IOS_NET_HTTP_CACHE_HELPER_H_
