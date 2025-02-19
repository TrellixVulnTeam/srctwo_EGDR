// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebServiceWorkerClientType_h
#define WebServiceWorkerClientType_h

namespace blink {

enum WebServiceWorkerClientType {
  kWebServiceWorkerClientTypeWindow,
  kWebServiceWorkerClientTypeWorker,
  kWebServiceWorkerClientTypeSharedWorker,
  kWebServiceWorkerClientTypeAll,
  kWebServiceWorkerClientTypeLast = kWebServiceWorkerClientTypeAll
};

}  // namespace blink

#endif  // WebServiceWorkerClientType_h
