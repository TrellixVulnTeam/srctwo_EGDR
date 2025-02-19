/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WTF_StdLibExtras_h
#define WTF_StdLibExtras_h

#include <cstddef>
#include "base/numerics/safe_conversions.h"
#include "build/build_config.h"
#include "platform/wtf/Assertions.h"
#include "platform/wtf/LeakAnnotations.h"
#include "platform/wtf/Noncopyable.h"
#include "platform/wtf/TypeTraits.h"

#if DCHECK_IS_ON()
#include "platform/wtf/Threading.h"
#endif

#define DEFINE_STATIC_LOCAL_IMPL(Type, Name, Arguments, allow_cross_thread)    \
  static WTF::StaticSingleton<Type> s_##Name(                                  \
      [&]() { return new WTF::StaticSingleton<Type>::WrapperType Arguments; }, \
      [&](void* leaked_ptr) {                                                  \
        new (leaked_ptr) WTF::StaticSingleton<Type>::WrapperType Arguments;    \
      });                                                                      \
  Type& Name = s_##Name.Get(allow_cross_thread)

// Use |DEFINE_STATIC_LOCAL()| to declare and define a static local variable
// (|static T;|) so that it is leaked and its destructors are not called at
// exit. T may also be a Blink garbage collected object, in which case it is
// wrapped up by an off-heap |Persistent<T>| reference to the object, keeping
// it alive across GCs.
//
// A |DEFINE_STATIC_LOCAL()| static should only be used on the thread it was
// created on.
//
#define DEFINE_STATIC_LOCAL(Type, Name, Arguments) \
  DEFINE_STATIC_LOCAL_IMPL(Type, Name, Arguments, false)

// |DEFINE_THREAD_SAFE_STATIC_LOCAL()| is the cross-thread accessible variant
// of |DEFINE_STATIC_LOCAL()|; use it if the singleton can be accessed by
// multiple threads.
//
// TODO: rename as DEFINE_CROSS_THREAD_STATIC_LOCAL() ?
#define DEFINE_THREAD_SAFE_STATIC_LOCAL(Type, Name, Arguments) \
  DEFINE_STATIC_LOCAL_IMPL(Type, Name, Arguments, true)

namespace blink {
template <typename T>
class Persistent;

}  // namespace blink

namespace WTF {

template <typename Type>
class StaticSingleton final {
  WTF_MAKE_NONCOPYABLE(StaticSingleton);

 public:
  template <typename T,
            bool = WTF::IsGarbageCollectedType<T>::value &&
                   !WTF::IsPersistentReferenceType<T>::value>
  struct Wrapper {
    using type = T;

    static T& Unwrap(T* singleton) { return *singleton; }
  };

  template <typename T>
  struct Wrapper<T, true> {
    using type = blink::Persistent<T>;

    static T& Unwrap(blink::Persistent<T>* singleton) {
      DCHECK(singleton);
      // If this assert triggers, you're supplying an empty ("()") 'Arguments'
      // argument to DEFINE_STATIC_LOCAL() - it must be the heap object you wish
      // to create as a static singleton and wrapped up with a Persistent
      // reference.
      DCHECK(*singleton);
      return **singleton;
    }
  };

  using WrapperType = typename Wrapper<Type>::type;

  // To cooperate with leak detection(LSan) for Blink garbage collected objects,
  // the objects owned by persistent local statics will in some cases have to be
  // finalized prior to leak checking. This only applies to static references to
  // Blink heap objects and what they transitively hold on to. Hence the
  // LEAK_SANITIZER_REGISTER_STATIC_LOCAL() use, it taking care of the grungy
  // details.

  template <typename HeapNew, typename PlacementNew>
  StaticSingleton(const HeapNew& heap_new, const PlacementNew& placement_new)
      : instance_(heap_new, placement_new)
#if DCHECK_IS_ON()
        ,
        safely_initialized_(WTF::IsBeforeThreadCreated()),
        thread_(WTF::internal::CurrentThreadSyscall())
#endif
  {
    LEAK_SANITIZER_REGISTER_STATIC_LOCAL(WrapperType, instance_.Get());
  }

  Type& Get(bool allow_cross_thread_use) {
#if DCHECK_IS_ON()
    DCHECK(IsNotRacy(allow_cross_thread_use));
#endif
    ALLOW_UNUSED_LOCAL(allow_cross_thread_use);
    return Wrapper<Type>::Unwrap(instance_.Get());
  }

  operator Type&() { return Get(); }

 private:
#if DCHECK_IS_ON()

  bool IsNotRacy(bool allow_cross_thread_use) const {
    // Make sure that singleton is safely initialized, or
    // keeps being called on the same thread if cross-thread
    // use is not permitted.
    return allow_cross_thread_use || safely_initialized_ ||
           thread_ == WTF::internal::CurrentThreadSyscall();
  }
#endif
  template <typename T, bool is_small = sizeof(T) <= 32>
  class InstanceStorage {
   public:
    template <typename HeapNew, typename PlacementNew>
    InstanceStorage(const HeapNew& heap_new, const PlacementNew&)
        : pointer_(heap_new()) {}
    T* Get() { return pointer_; }

   private:
    T* pointer_;
  };

  template <typename T>
  class InstanceStorage<T, true> {
   public:
    template <typename HeapNew, typename PlacementNew>
    InstanceStorage(const HeapNew&, const PlacementNew& placement_new) {
      placement_new(&object_);
    }
    T* Get() { return reinterpret_cast<T*>(object_); }

   private:
    alignas(T) char object_[sizeof(T)];
  };

  InstanceStorage<WrapperType> instance_;
#if DCHECK_IS_ON()
  bool safely_initialized_;
  ThreadIdentifier thread_;
#endif
};

}  // namespace WTF

// Use this to declare and define a static local pointer to a ref-counted object
// so that it is leaked so that the object's destructors are not called at
// exit.  This macro should be used with ref-counted objects rather than
// DEFINE_STATIC_LOCAL macro, as this macro does not lead to an extra memory
// allocation.
#define DEFINE_STATIC_REF(type, name, arguments) \
  static type* name = RefPtr<type>(arguments).LeakRef();

/*
 * The reinterpret_cast<Type1*>([pointer to Type2]) expressions - where
 * sizeof(Type1) > sizeof(Type2) - cause the following warning on ARM with GCC:
 * increases required alignment of target type.
 *
 * An implicit or an extra static_cast<void*> bypasses the warning.
 * For more info see the following bugzilla entries:
 * - https://bugs.webkit.org/show_bug.cgi?id=38045
 * - http://gcc.gnu.org/bugzilla/show_bug.cgi?id=43976
 */
#if defined(ARCH_CPU_ARMEL) && defined(COMPILER_GCC)
template <typename Type>
bool isPointerTypeAlignmentOkay(Type* ptr) {
  return !(reinterpret_cast<intptr_t>(ptr) % __alignof__(Type));
}

template <typename TypePtr>
TypePtr reinterpret_cast_ptr(void* ptr) {
  DCHECK(isPointerTypeAlignmentOkay(reinterpret_cast<TypePtr>(ptr)));
  return reinterpret_cast<TypePtr>(ptr);
}

template <typename TypePtr>
TypePtr reinterpret_cast_ptr(const void* ptr) {
  DCHECK(isPointerTypeAlignmentOkay(reinterpret_cast<TypePtr>(ptr)));
  return reinterpret_cast<TypePtr>(ptr);
}
#else
template <typename Type>
bool isPointerTypeAlignmentOkay(Type*) {
  return true;
}
#define reinterpret_cast_ptr reinterpret_cast
#endif

namespace WTF {

/*
 * C++'s idea of a reinterpret_cast lacks sufficient cojones.
 */
template <typename TO, typename FROM>
inline TO BitwiseCast(FROM from) {
  static_assert(sizeof(TO) == sizeof(FROM),
                "WTF::bitwiseCast sizeof casted types should be equal");
  union {
    FROM from;
    TO to;
  } u;
  u.from = from;
  return u.to;
}

template <typename To, typename From>
inline To SafeCast(From value) {
  return base::checked_cast<To>(value);
}

// Use the following macros to prevent errors caused by accidental
// implicit casting of function arguments.  For example, this can
// be used to prevent overflows from non-promoting conversions.
//
// Example:
//
// HAS_STRICTLY_TYPED_ARG
// void sendData(void* data, STRICTLY_TYPED_ARG(size))
// {
//    ALLOW_NUMERIC_ARG_TYPES_PROMOTABLE_TO(size_t);
//    ...
// }
//
// The previous example will prevent callers from passing, for example, an
// 'int'. On a 32-bit build, it will prevent use of an 'unsigned long long'.
#define HAS_STRICTLY_TYPED_ARG template <typename ActualArgType>
#define STRICTLY_TYPED_ARG(argName) ActualArgType argName
#define STRICT_ARG_TYPE(ExpectedArgType)                                     \
  static_assert(std::is_same<ActualArgType, ExpectedArgType>::value,         \
                "Strictly typed argument must be of type '" #ExpectedArgType \
                "'.")
#define ALLOW_NUMERIC_ARG_TYPES_PROMOTABLE_TO(ExpectedArgType)              \
  static_assert(                                                            \
      std::numeric_limits<ExpectedArgType>::is_integer ==                   \
          std::numeric_limits<ActualArgType>::is_integer,                   \
      "Conversion between integer and non-integer types not allowed.");     \
  static_assert(sizeof(ExpectedArgType) >= sizeof(ActualArgType),           \
                "Truncating conversions not allowed.");                     \
  static_assert(!std::numeric_limits<ActualArgType>::is_signed ||           \
                    std::numeric_limits<ExpectedArgType>::is_signed,        \
                "Signed to unsigned conversion not allowed.");              \
  static_assert((sizeof(ExpectedArgType) != sizeof(ActualArgType)) ||       \
                    (std::numeric_limits<ActualArgType>::is_signed ==       \
                     std::numeric_limits<ExpectedArgType>::is_signed),      \
                "Unsigned to signed conversion not allowed for types with " \
                "identical size (could overflow).");

// Macro that returns a compile time constant with the length of an array, but
// gives an error if passed a non-array.
template <typename T, size_t Size>
char (&ArrayLengthHelperFunction(T (&)[Size]))[Size];
// GCC needs some help to deduce a 0 length array.
#if defined(COMPILER_GCC)
template <typename T>
char (&ArrayLengthHelperFunction(T (&)[0]))[0];
#endif
#define WTF_ARRAY_LENGTH(array) sizeof(::WTF::ArrayLengthHelperFunction(array))

}  // namespace WTF

using WTF::BitwiseCast;
using WTF::SafeCast;

#endif  // WTF_StdLibExtras_h
