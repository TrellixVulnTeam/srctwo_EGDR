// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SelfKeepAlive_h
#define SelfKeepAlive_h

#include "platform/heap/Persistent.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/Assertions.h"

namespace blink {

// SelfKeepAlive<Object> is the idiom to use for objects that have to keep
// themselves temporarily alive and cannot rely on there being some
// external reference in that interval:
//
//  class Opener {
//  public:
//     ...
//     void open()
//     {
//         // Retain a self-reference while in an open()ed state:
//         m_keepAlive = this;
//         ....
//     }
//
//     void close()
//     {
//         // Clear self-reference that ensured we were kept alive while opened.
//         m_keepAlive.clear();
//         ....
//     }
//
//  private:
//     ...
//     SelfKeepAlive m_keepAlive;
//  };
//
// The responsibility to call clear() in a timely fashion resides with the
// implementation of the object.
//
//
template <typename Self>
class SelfKeepAlive final {
  DISALLOW_NEW();

 public:
  SelfKeepAlive() {}

  explicit SelfKeepAlive(Self* self) { Assign(self); }

  SelfKeepAlive& operator=(Self* self) {
    Assign(self);
    return *this;
  }

  void Clear() { keep_alive_.Clear(); }

  explicit operator bool() const { return keep_alive_; }

 private:
  void Assign(Self* self) {
    DCHECK(!keep_alive_ || keep_alive_.Get() == self);
    keep_alive_ = self;
  }

  GC_PLUGIN_IGNORE("420515")
  Persistent<Self> keep_alive_;
};

}  // namespace blink

#endif  // SelfKeepAlive_h
