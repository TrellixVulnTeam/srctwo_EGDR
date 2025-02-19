// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_SSL_SSL_PLATFORM_KEY_ANDROID_H_
#define NET_SSL_SSL_PLATFORM_KEY_ANDROID_H_

#include <jni.h>

#include "base/android/scoped_java_ref.h"
#include "base/memory/ref_counted.h"
#include "net/base/net_export.h"

namespace net {

class SSLPrivateKey;
class X509Certificate;

// Returns a new SSLPrivateKey for |cert| which uses |key| for signing
// operations or nullptr on error. |key| must be a java.security.PrivateKey
// object.
NET_EXPORT scoped_refptr<SSLPrivateKey> WrapJavaPrivateKey(
    const X509Certificate* cert,
    const base::android::JavaRef<jobject>& key);

}  // namespace net

#endif  // NET_SSL_SSL_PLATFORM_KEY_ANDROID_H_
