// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef URL_IPC_URL_PARAM_TRAITS_H_
#define URL_IPC_URL_PARAM_TRAITS_H_

#include "ipc/ipc_message_utils.h"
#include "url/gurl.h"
#include "url/ipc/url_ipc_export.h"

namespace IPC {

template <>
struct URL_IPC_EXPORT ParamTraits<GURL> {
  typedef GURL param_type;
  static void Write(base::Pickle* m, const param_type& p);
  static bool Read(const base::Pickle* m,
                   base::PickleIterator* iter,
                   param_type* p);
  static void Log(const param_type& p, std::string* l);
};

}  // namespace IPC

#endif  // URL_IPC_URL_PARAM_TRAITS_H_
