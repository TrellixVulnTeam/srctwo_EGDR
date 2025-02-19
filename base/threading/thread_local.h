// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// WARNING: Thread local storage is a bit tricky to get right. Please make sure
// that this is really the proper solution for what you're trying to achieve.
// Don't prematurely optimize, most likely you can just use a Lock.
//
// These classes implement a wrapper around ThreadLocalStorage::Slot. On
// construction, they will allocate a TLS slot, and free the TLS slot on
// destruction. No memory management (creation or destruction) is handled. This
// means for uses of ThreadLocalPointer, you must correctly manage the memory
// yourself, these classes will not destroy the pointer for you. There are no
// at-thread-exit actions taken by these classes.
//
// ThreadLocalPointer<Type> wraps a Type*. It performs no creation or
// destruction, so memory management must be handled elsewhere. The first call
// to Get() on a thread will return NULL. You can update the pointer with a call
// to Set().
//
// ThreadLocalBoolean wraps a bool. It will default to false if it has never
// been set otherwise with Set().
//
// Thread Safety: An instance of ThreadLocalStorage is completely thread safe
// once it has been created. If you want to dynamically create an instance, you
// must of course properly deal with safety and race conditions. This means a
// function-level static initializer is generally inappropiate.
//
// In Android, the system TLS is limited.
//
// Example usage:
//   // My class is logically attached to a single thread. We cache a pointer
//   // on the thread it was created on, so we can implement current().
//   MyClass::MyClass() {
//     DCHECK(Singleton<ThreadLocalPointer<MyClass> >::get()->Get() == NULL);
//     Singleton<ThreadLocalPointer<MyClass> >::get()->Set(this);
//   }
//
//   MyClass::~MyClass() {
//     DCHECK(Singleton<ThreadLocalPointer<MyClass> >::get()->Get() != NULL);
//     Singleton<ThreadLocalPointer<MyClass> >::get()->Set(NULL);
//   }
//
//   // Return the current MyClass associated with the calling thread, can be
//   // NULL if there isn't a MyClass associated.
//   MyClass* MyClass::current() {
//     return Singleton<ThreadLocalPointer<MyClass> >::get()->Get();
//   }

#ifndef BASE_THREADING_THREAD_LOCAL_H_
#define BASE_THREADING_THREAD_LOCAL_H_

#include "base/macros.h"
#include "base/threading/thread_local_storage.h"

namespace base {

template <typename Type>
class ThreadLocalPointer {
 public:
  ThreadLocalPointer() = default;
  ~ThreadLocalPointer() = default;

  Type* Get() {
    return static_cast<Type*>(slot_.Get());
  }

  void Set(Type* ptr) {
    slot_.Set(const_cast<void*>(static_cast<const void*>(ptr)));
  }

 private:
  ThreadLocalStorage::Slot slot_;

  DISALLOW_COPY_AND_ASSIGN(ThreadLocalPointer<Type>);
};

class ThreadLocalBoolean {
 public:
  ThreadLocalBoolean() = default;
  ~ThreadLocalBoolean() = default;

  bool Get() {
    return tlp_.Get() != nullptr;
  }

  void Set(bool val) {
    tlp_.Set(val ? this : nullptr);
  }

 private:
  ThreadLocalPointer<void> tlp_;

  DISALLOW_COPY_AND_ASSIGN(ThreadLocalBoolean);
};

}  // namespace base

#endif  // BASE_THREADING_THREAD_LOCAL_H_
