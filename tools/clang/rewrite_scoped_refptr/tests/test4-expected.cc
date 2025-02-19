// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scoped_refptr.h"

struct Foo {
  int dummy;
};

void ExpectsRawPtr(Foo* foo) {
  Foo* temp = foo;
}

void CallExpectsRawPtrWithScopedRefptr() {
  scoped_refptr<Foo> ok(new Foo);
  ExpectsRawPtr(ok.get());
}

void CallExpectsRawPtrWithRawPtr() {
  ExpectsRawPtr(new Foo);
}
