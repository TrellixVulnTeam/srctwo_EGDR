/* Copyright (c) 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file. */

#include "nacl_io/kernel_intercept.h"
#include "nacl_io/kernel_wrap.h"

#if defined(__native_client__) && defined(__GLIBC__)
// GLIBC-only entry point.
// TODO(sbc): remove once this bug gets fixed:
// https://code.google.com/p/nativeclient/issues/detail?id=3709
int isatty(int fd) {
  return ki_isatty(fd);
}
#endif
