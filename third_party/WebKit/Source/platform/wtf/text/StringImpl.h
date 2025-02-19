/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2013 Apple Inc. All rights
 * reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef StringImpl_h
#define StringImpl_h

#include <limits.h>
#include <string.h>
#include "build/build_config.h"
#include "platform/wtf/ASCIICType.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/HashMap.h"
#include "platform/wtf/StringHasher.h"
#include "platform/wtf/Vector.h"
#include "platform/wtf/WTFExport.h"
#include "platform/wtf/text/ASCIIFastPath.h"
#include "platform/wtf/text/NumberParsingOptions.h"
#include "platform/wtf/text/Unicode.h"

#if DCHECK_IS_ON()
#include "platform/wtf/ThreadRestrictionVerifier.h"
#endif

#if defined(OS_MACOSX)
typedef const struct __CFString* CFStringRef;
#endif

#ifdef __OBJC__
@class NSString;
#endif

namespace WTF {

struct AlreadyHashed;
template <typename>
class RetainPtr;

enum TextCaseSensitivity {
  kTextCaseSensitive,
  kTextCaseASCIIInsensitive,

  // Unicode aware case insensitive matching. Non-ASCII characters might match
  // to ASCII characters. This flag is rarely used to implement web platform
  // features.
  kTextCaseUnicodeInsensitive
};

enum StripBehavior { kStripExtraWhiteSpace, kDoNotStripWhiteSpace };

typedef bool (*CharacterMatchFunctionPtr)(UChar);
typedef bool (*IsWhiteSpaceFunctionPtr)(UChar);
typedef HashMap<unsigned, StringImpl*, AlreadyHashed> StaticStringsTable;

// You can find documentation about this class in this doc:
// https://docs.google.com/document/d/1kOCUlJdh2WJMJGDf-WoEQhmnjKLaOYRbiHz5TiGJl14/edit?usp=sharing
class WTF_EXPORT StringImpl {
  WTF_MAKE_NONCOPYABLE(StringImpl);

 private:
  // StringImpls are allocated out of the WTF buffer partition.
  void* operator new(size_t);
  void* operator new(size_t, void* ptr) { return ptr; }
  void operator delete(void*);

  // Used to construct static strings, which have an special refCount that can
  // never hit zero.  This means that the static string will never be
  // destroyed, which is important because static strings will be shared
  // across threads & ref-counted in a non-threadsafe manner.
  enum ConstructEmptyStringTag { kConstructEmptyString };
  explicit StringImpl(ConstructEmptyStringTag)
      : ref_count_(1),
        length_(0),
        hash_(0),
        contains_only_ascii_(true),
        needs_ascii_check_(false),
        is_atomic_(false),
        is8_bit_(true),
        is_static_(true) {
    // Ensure that the hash is computed so that AtomicStringHash can call
    // existingHash() with impunity. The empty string is special because it
    // is never entered into AtomicString's HashKey, but still needs to
    // compare correctly.
    GetHash();
  }

  enum ConstructEmptyString16BitTag { kConstructEmptyString16Bit };
  explicit StringImpl(ConstructEmptyString16BitTag)
      : ref_count_(1),
        length_(0),
        hash_(0),
        contains_only_ascii_(true),
        needs_ascii_check_(false),
        is_atomic_(false),
        is8_bit_(false),
        is_static_(true) {
    GetHash();
  }

  // FIXME: there has to be a less hacky way to do this.
  enum Force8Bit { kForce8BitConstructor };
  StringImpl(unsigned length, Force8Bit)
      : ref_count_(1),
        length_(length),
        hash_(0),
        contains_only_ascii_(!length),
        needs_ascii_check_(static_cast<bool>(length)),
        is_atomic_(false),
        is8_bit_(true),
        is_static_(false) {
    DCHECK(length_);
  }

  StringImpl(unsigned length)
      : ref_count_(1),
        length_(length),
        hash_(0),
        contains_only_ascii_(!length),
        needs_ascii_check_(static_cast<bool>(length)),
        is_atomic_(false),
        is8_bit_(false),
        is_static_(false) {
    DCHECK(length_);
  }

  enum StaticStringTag { kStaticString };
  StringImpl(unsigned length, unsigned hash, StaticStringTag)
      : ref_count_(1),
        length_(length),
        hash_(hash),
        contains_only_ascii_(!length),
        needs_ascii_check_(static_cast<bool>(length)),
        is_atomic_(false),
        is8_bit_(true),
        is_static_(true) {}

 public:
  static StringImpl* empty_;
  static StringImpl* empty16_bit_;

  ~StringImpl();

  static void InitStatics();

  static StringImpl* CreateStatic(const char* string,
                                  unsigned length,
                                  unsigned hash);
  static void ReserveStaticStringsCapacityForSize(unsigned size);
  static void FreezeStaticStrings();
  static const StaticStringsTable& AllStaticStrings();
  static unsigned HighestStaticStringLength() {
    return highest_static_string_length_;
  }

  static RefPtr<StringImpl> Create(const UChar*, unsigned length);
  static RefPtr<StringImpl> Create(const LChar*, unsigned length);
  static RefPtr<StringImpl> Create8BitIfPossible(const UChar*, unsigned length);
  template <size_t inlineCapacity>
  static RefPtr<StringImpl> Create8BitIfPossible(
      const Vector<UChar, inlineCapacity>& vector) {
    return Create8BitIfPossible(vector.data(), vector.size());
  }

  ALWAYS_INLINE static RefPtr<StringImpl> Create(const char* s,
                                                 unsigned length) {
    return Create(reinterpret_cast<const LChar*>(s), length);
  }
  static RefPtr<StringImpl> Create(const LChar*);
  ALWAYS_INLINE static RefPtr<StringImpl> Create(const char* s) {
    return Create(reinterpret_cast<const LChar*>(s));
  }

  static RefPtr<StringImpl> CreateUninitialized(unsigned length, LChar*& data);
  static RefPtr<StringImpl> CreateUninitialized(unsigned length, UChar*& data);

  unsigned length() const { return length_; }
  bool Is8Bit() const { return is8_bit_; }

  ALWAYS_INLINE const LChar* Characters8() const {
    DCHECK(Is8Bit());
    return reinterpret_cast<const LChar*>(this + 1);
  }
  ALWAYS_INLINE const UChar* Characters16() const {
    DCHECK(!Is8Bit());
    return reinterpret_cast<const UChar*>(this + 1);
  }
  ALWAYS_INLINE const void* Bytes() const {
    return reinterpret_cast<const void*>(this + 1);
  }

  template <typename CharType>
  ALWAYS_INLINE const CharType* GetCharacters() const;

  size_t CharactersSizeInBytes() const {
    return length() * (Is8Bit() ? sizeof(LChar) : sizeof(UChar));
  }

  bool IsAtomic() const { return is_atomic_; }
  void SetIsAtomic(bool is_atomic) { is_atomic_ = is_atomic; }

  bool IsStatic() const { return is_static_; }

  bool ContainsOnlyASCII() const;

  bool IsSafeToSendToAnotherThread() const;

  // The high bits of 'hash' are always empty, but we prefer to store our
  // flags in the low bits because it makes them slightly more efficient to
  // access.  So, we shift left and right when setting and getting our hash
  // code.
  void SetHash(unsigned hash) const {
    DCHECK(!HasHash());
    // Multiple clients assume that StringHasher is the canonical string
    // hash function.
    DCHECK(hash == (Is8Bit() ? StringHasher::ComputeHashAndMaskTop8Bits(
                                   Characters8(), length_)
                             : StringHasher::ComputeHashAndMaskTop8Bits(
                                   Characters16(), length_)));
    hash_ = hash;
    DCHECK(hash);  // Verify that 0 is a valid sentinel hash value.
  }

  bool HasHash() const { return hash_ != 0; }

  unsigned ExistingHash() const {
    DCHECK(HasHash());
    return hash_;
  }

  unsigned GetHash() const {
    if (HasHash())
      return ExistingHash();
    return HashSlowCase();
  }

  ALWAYS_INLINE bool HasOneRef() const {
#if DCHECK_IS_ON()
    DCHECK(IsStatic() || verifier_.IsSafeToUse()) << AsciiForDebugging();
#endif
    return ref_count_ == 1;
  }

  ALWAYS_INLINE void Ref() const {
#if DCHECK_IS_ON()
    DCHECK(IsStatic() || verifier_.OnRef(ref_count_)) << AsciiForDebugging();
#endif
    ++ref_count_;
  }

  ALWAYS_INLINE void Deref() const {
#if DCHECK_IS_ON()
    DCHECK(IsStatic() || verifier_.OnDeref(ref_count_))
        << AsciiForDebugging() << " " << CurrentThread();
#endif
    if (!--ref_count_)
      DestroyIfNotStatic();
  }

  // FIXME: Does this really belong in StringImpl?
  template <typename T>
  static void CopyChars(T* destination,
                        const T* source,
                        unsigned num_characters) {
    memcpy(destination, source, num_characters * sizeof(T));
  }

  ALWAYS_INLINE static void CopyChars(UChar* destination,
                                      const LChar* source,
                                      unsigned num_characters) {
    for (unsigned i = 0; i < num_characters; ++i)
      destination[i] = source[i];
  }

  // Some string features, like refcounting and the atomicity flag, are not
  // thread-safe. We achieve thread safety by isolation, giving each thread
  // its own copy of the string.
  RefPtr<StringImpl> IsolatedCopy() const;

  RefPtr<StringImpl> Substring(unsigned pos, unsigned len = UINT_MAX) const;

  UChar operator[](unsigned i) const {
    SECURITY_DCHECK(i < length_);
    if (Is8Bit())
      return Characters8()[i];
    return Characters16()[i];
  }
  UChar32 CharacterStartingAt(unsigned);

  bool ContainsOnlyWhitespace();

  int ToInt(NumberParsingOptions, bool* ok) const;
  unsigned ToUInt(NumberParsingOptions, bool* ok) const;
  int64_t ToInt64(NumberParsingOptions, bool* ok) const;
  uint64_t ToUInt64(NumberParsingOptions, bool* ok) const;

  unsigned HexToUIntStrict(bool* ok);

  // FIXME: Like NumberParsingOptions::kStrict, these give false for "ok" when
  // there is trailing garbage.  Like NumberParsingOptions::kLoose, these return
  // the value when there is trailing garbage.  It would be better if these were
  // more consistent with the above functions instead.
  double ToDouble(bool* ok = 0);
  float ToFloat(bool* ok = 0);

  RefPtr<StringImpl> LowerUnicode();
  RefPtr<StringImpl> LowerASCII();
  RefPtr<StringImpl> UpperUnicode();
  RefPtr<StringImpl> UpperASCII();
  RefPtr<StringImpl> LowerUnicode(const AtomicString& locale_identifier);
  RefPtr<StringImpl> UpperUnicode(const AtomicString& locale_identifier);

  RefPtr<StringImpl> Fill(UChar);
  // FIXME: Do we need fill(char) or can we just do the right thing if UChar is
  // ASCII?
  RefPtr<StringImpl> FoldCase();

  RefPtr<StringImpl> Truncate(unsigned length);

  RefPtr<StringImpl> StripWhiteSpace();
  RefPtr<StringImpl> StripWhiteSpace(IsWhiteSpaceFunctionPtr);
  RefPtr<StringImpl> SimplifyWhiteSpace(StripBehavior = kStripExtraWhiteSpace);
  RefPtr<StringImpl> SimplifyWhiteSpace(IsWhiteSpaceFunctionPtr,
                                        StripBehavior = kStripExtraWhiteSpace);

  RefPtr<StringImpl> RemoveCharacters(CharacterMatchFunctionPtr);
  template <typename CharType>
  ALWAYS_INLINE RefPtr<StringImpl> RemoveCharacters(const CharType* characters,
                                                    CharacterMatchFunctionPtr);

  // Remove characters between [start, start+lengthToRemove). The range is
  // clamped to the size of the string. Does nothing if start >= length().
  RefPtr<StringImpl> Remove(unsigned start, unsigned length_to_remove = 1);

  // Find characters.
  size_t Find(LChar character, unsigned start = 0);
  size_t Find(char character, unsigned start = 0);
  size_t Find(UChar character, unsigned start = 0);
  size_t Find(CharacterMatchFunctionPtr, unsigned index = 0);

  // Find substrings.
  size_t Find(const StringView&, unsigned index = 0);
  // Unicode aware case insensitive string matching. Non-ASCII characters might
  // match to ASCII characters. This function is rarely used to implement web
  // platform features.
  size_t FindIgnoringCase(const StringView&, unsigned index = 0);
  size_t FindIgnoringASCIICase(const StringView&, unsigned index = 0);

  size_t ReverseFind(UChar, unsigned index = UINT_MAX);
  size_t ReverseFind(const StringView&, unsigned index = UINT_MAX);

  bool StartsWith(UChar) const;
  bool StartsWith(const StringView&) const;
  bool StartsWithIgnoringCase(const StringView&) const;
  bool StartsWithIgnoringASCIICase(const StringView&) const;

  bool EndsWith(UChar) const;
  bool EndsWith(const StringView&) const;
  bool EndsWithIgnoringCase(const StringView&) const;
  bool EndsWithIgnoringASCIICase(const StringView&) const;

  // Replace parts of the string.
  RefPtr<StringImpl> Replace(UChar pattern, UChar replacement);
  RefPtr<StringImpl> Replace(UChar pattern, const StringView& replacement);
  RefPtr<StringImpl> Replace(const StringView& pattern,
                             const StringView& replacement);
  RefPtr<StringImpl> Replace(unsigned index,
                             unsigned length_to_replace,
                             const StringView& replacement);

  RefPtr<StringImpl> UpconvertedString();

  // Copy characters from string starting at |start| up until |maxLength| or
  // the end of the string is reached. Returns the actual number of characters
  // copied.
  unsigned CopyTo(UChar* buffer, unsigned start, unsigned max_length) const;

  // Append characters from this string into a buffer. Expects the buffer to
  // have the methods:
  //    append(const UChar*, unsigned length);
  //    append(const LChar*, unsigned length);
  // StringBuilder and Vector conform to this protocol.
  template <typename BufferType>
  void AppendTo(BufferType&,
                unsigned start = 0,
                unsigned length = UINT_MAX) const;

  // Prepend characters from this string into a buffer. Expects the buffer to
  // have the methods:
  //    prepend(const UChar*, unsigned length);
  //    prepend(const LChar*, unsigned length);
  // Vector conforms to this protocol.
  template <typename BufferType>
  void PrependTo(BufferType&,
                 unsigned start = 0,
                 unsigned length = UINT_MAX) const;

#if defined(OS_MACOSX)
  RetainPtr<CFStringRef> CreateCFString();
#endif
#ifdef __OBJC__
  operator NSString*();
#endif

  static const UChar kLatin1CaseFoldTable[256];

 private:
  template <typename CharType>
  static size_t AllocationSize(unsigned length) {
    CHECK_LE(length,
             ((std::numeric_limits<unsigned>::max() - sizeof(StringImpl)) /
              sizeof(CharType)));
    return sizeof(StringImpl) + length * sizeof(CharType);
  }

  RefPtr<StringImpl> Replace(UChar pattern,
                             const LChar* replacement,
                             unsigned replacement_length);
  RefPtr<StringImpl> Replace(UChar pattern,
                             const UChar* replacement,
                             unsigned replacement_length);

  template <class UCharPredicate>
  RefPtr<StringImpl> StripMatchedCharacters(UCharPredicate);
  template <typename CharType, class UCharPredicate>
  RefPtr<StringImpl> SimplifyMatchedCharactersToSpace(UCharPredicate,
                                                      StripBehavior);
  NEVER_INLINE unsigned HashSlowCase() const;

  void DestroyIfNotStatic() const;
  void UpdateContainsOnlyASCII() const;

#if DCHECK_IS_ON()
  std::string AsciiForDebugging() const;
#endif

  static unsigned highest_static_string_length_;

#if DCHECK_IS_ON()
  void AssertHashIsCorrect() {
    DCHECK(HasHash());
    DCHECK_EQ(ExistingHash(), StringHasher::ComputeHashAndMaskTop8Bits(
                                  Characters8(), length()));
  }
#endif

 private:
#if DCHECK_IS_ON()
  mutable ThreadRestrictionVerifier verifier_;
#endif
  mutable unsigned ref_count_;
  const unsigned length_;
  mutable unsigned hash_ : 24;
  mutable unsigned contains_only_ascii_ : 1;
  mutable unsigned needs_ascii_check_ : 1;
  unsigned is_atomic_ : 1;
  const unsigned is8_bit_ : 1;
  const unsigned is_static_ : 1;
};

template <>
ALWAYS_INLINE const LChar* StringImpl::GetCharacters<LChar>() const {
  return Characters8();
}

template <>
ALWAYS_INLINE const UChar* StringImpl::GetCharacters<UChar>() const {
  return Characters16();
}

WTF_EXPORT bool Equal(const StringImpl*, const StringImpl*);
WTF_EXPORT bool Equal(const StringImpl*, const LChar*);
inline bool Equal(const StringImpl* a, const char* b) {
  return Equal(a, reinterpret_cast<const LChar*>(b));
}
WTF_EXPORT bool Equal(const StringImpl*, const LChar*, unsigned);
WTF_EXPORT bool Equal(const StringImpl*, const UChar*, unsigned);
inline bool Equal(const StringImpl* a, const char* b, unsigned length) {
  return Equal(a, reinterpret_cast<const LChar*>(b), length);
}
inline bool Equal(const LChar* a, StringImpl* b) {
  return Equal(b, a);
}
inline bool Equal(const char* a, StringImpl* b) {
  return Equal(b, reinterpret_cast<const LChar*>(a));
}
WTF_EXPORT bool EqualNonNull(const StringImpl* a, const StringImpl* b);

ALWAYS_INLINE bool StringImpl::ContainsOnlyASCII() const {
  if (needs_ascii_check_)
    UpdateContainsOnlyASCII();
  return contains_only_ascii_;
}

template <typename CharType>
ALWAYS_INLINE bool Equal(const CharType* a,
                         const CharType* b,
                         unsigned length) {
  return !memcmp(a, b, length * sizeof(CharType));
}

ALWAYS_INLINE bool Equal(const LChar* a, const UChar* b, unsigned length) {
  for (unsigned i = 0; i < length; ++i) {
    if (a[i] != b[i])
      return false;
  }
  return true;
}

ALWAYS_INLINE bool Equal(const UChar* a, const LChar* b, unsigned length) {
  return Equal(b, a, length);
}

// Unicode aware case insensitive string matching. Non-ASCII characters might
// match to ASCII characters. These functions are rarely used to implement web
// platform features.
// These functions are deprecated. Use EqualIgnoringASCIICase(), or introduce
// EqualIgnoringUnicodeCase(). See crbug.com/627682
WTF_EXPORT bool DeprecatedEqualIgnoringCase(const LChar*,
                                            const LChar*,
                                            unsigned length);
WTF_EXPORT bool DeprecatedEqualIgnoringCase(const UChar*,
                                            const LChar*,
                                            unsigned length);
inline bool DeprecatedEqualIgnoringCase(const LChar* a,
                                        const UChar* b,
                                        unsigned length) {
  return DeprecatedEqualIgnoringCase(b, a, length);
}
WTF_EXPORT bool DeprecatedEqualIgnoringCase(const UChar*,
                                            const UChar*,
                                            unsigned length);

WTF_EXPORT bool EqualIgnoringNullity(StringImpl*, StringImpl*);

template <typename CharacterTypeA, typename CharacterTypeB>
inline bool EqualIgnoringASCIICase(const CharacterTypeA* a,
                                   const CharacterTypeB* b,
                                   unsigned length) {
  for (unsigned i = 0; i < length; ++i) {
    if (ToASCIILower(a[i]) != ToASCIILower(b[i]))
      return false;
  }
  return true;
}

WTF_EXPORT int CodePointCompareIgnoringASCIICase(const StringImpl*,
                                                 const LChar*);

inline size_t Find(const LChar* characters,
                   unsigned length,
                   LChar match_character,
                   unsigned index = 0) {
  // Some clients rely on being able to pass index >= length.
  if (index >= length)
    return kNotFound;
  const LChar* found = static_cast<const LChar*>(
      memchr(characters + index, match_character, length - index));
  return found ? found - characters : kNotFound;
}

inline size_t Find(const UChar* characters,
                   unsigned length,
                   UChar match_character,
                   unsigned index = 0) {
  while (index < length) {
    if (characters[index] == match_character)
      return index;
    ++index;
  }
  return kNotFound;
}

ALWAYS_INLINE size_t Find(const UChar* characters,
                          unsigned length,
                          LChar match_character,
                          unsigned index = 0) {
  return Find(characters, length, static_cast<UChar>(match_character), index);
}

inline size_t Find(const LChar* characters,
                   unsigned length,
                   UChar match_character,
                   unsigned index = 0) {
  if (match_character & ~0xFF)
    return kNotFound;
  return Find(characters, length, static_cast<LChar>(match_character), index);
}

template <typename CharacterType>
inline size_t Find(const CharacterType* characters,
                   unsigned length,
                   char match_character,
                   unsigned index = 0) {
  return Find(characters, length, static_cast<LChar>(match_character), index);
}

inline size_t Find(const LChar* characters,
                   unsigned length,
                   CharacterMatchFunctionPtr match_function,
                   unsigned index = 0) {
  while (index < length) {
    if (match_function(characters[index]))
      return index;
    ++index;
  }
  return kNotFound;
}

inline size_t Find(const UChar* characters,
                   unsigned length,
                   CharacterMatchFunctionPtr match_function,
                   unsigned index = 0) {
  while (index < length) {
    if (match_function(characters[index]))
      return index;
    ++index;
  }
  return kNotFound;
}

template <typename CharacterType>
inline size_t ReverseFind(const CharacterType* characters,
                          unsigned length,
                          CharacterType match_character,
                          unsigned index = UINT_MAX) {
  if (!length)
    return kNotFound;
  if (index >= length)
    index = length - 1;
  while (characters[index] != match_character) {
    if (!index--)
      return kNotFound;
  }
  return index;
}

ALWAYS_INLINE size_t ReverseFind(const UChar* characters,
                                 unsigned length,
                                 LChar match_character,
                                 unsigned index = UINT_MAX) {
  return ReverseFind(characters, length, static_cast<UChar>(match_character),
                     index);
}

inline size_t ReverseFind(const LChar* characters,
                          unsigned length,
                          UChar match_character,
                          unsigned index = UINT_MAX) {
  if (match_character & ~0xFF)
    return kNotFound;
  return ReverseFind(characters, length, static_cast<LChar>(match_character),
                     index);
}

inline size_t StringImpl::Find(LChar character, unsigned start) {
  if (Is8Bit())
    return WTF::Find(Characters8(), length_, character, start);
  return WTF::Find(Characters16(), length_, character, start);
}

ALWAYS_INLINE size_t StringImpl::Find(char character, unsigned start) {
  return Find(static_cast<LChar>(character), start);
}

inline size_t StringImpl::Find(UChar character, unsigned start) {
  if (Is8Bit())
    return WTF::Find(Characters8(), length_, character, start);
  return WTF::Find(Characters16(), length_, character, start);
}

inline unsigned LengthOfNullTerminatedString(const UChar* string) {
  size_t length = 0;
  while (string[length] != UChar(0))
    ++length;
  CHECK_LE(length, std::numeric_limits<unsigned>::max());
  return static_cast<unsigned>(length);
}

template <size_t inlineCapacity>
bool EqualIgnoringNullity(const Vector<UChar, inlineCapacity>& a,
                          StringImpl* b) {
  if (!b)
    return !a.size();
  if (a.size() != b->length())
    return false;
  if (b->Is8Bit())
    return Equal(a.data(), b->Characters8(), b->length());
  return Equal(a.data(), b->Characters16(), b->length());
}

template <typename CharacterType1, typename CharacterType2>
static inline int CodePointCompare(unsigned l1,
                                   unsigned l2,
                                   const CharacterType1* c1,
                                   const CharacterType2* c2) {
  const unsigned lmin = l1 < l2 ? l1 : l2;
  unsigned pos = 0;
  while (pos < lmin && *c1 == *c2) {
    ++c1;
    ++c2;
    ++pos;
  }

  if (pos < lmin)
    return (c1[0] > c2[0]) ? 1 : -1;

  if (l1 == l2)
    return 0;

  return (l1 > l2) ? 1 : -1;
}

static inline int CodePointCompare8(const StringImpl* string1,
                                    const StringImpl* string2) {
  return CodePointCompare(string1->length(), string2->length(),
                          string1->Characters8(), string2->Characters8());
}

static inline int CodePointCompare16(const StringImpl* string1,
                                     const StringImpl* string2) {
  return CodePointCompare(string1->length(), string2->length(),
                          string1->Characters16(), string2->Characters16());
}

static inline int CodePointCompare8To16(const StringImpl* string1,
                                        const StringImpl* string2) {
  return CodePointCompare(string1->length(), string2->length(),
                          string1->Characters8(), string2->Characters16());
}

static inline int CodePointCompare(const StringImpl* string1,
                                   const StringImpl* string2) {
  if (!string1)
    return (string2 && string2->length()) ? -1 : 0;

  if (!string2)
    return string1->length() ? 1 : 0;

  bool string1_is8_bit = string1->Is8Bit();
  bool string2_is8_bit = string2->Is8Bit();
  if (string1_is8_bit) {
    if (string2_is8_bit)
      return CodePointCompare8(string1, string2);
    return CodePointCompare8To16(string1, string2);
  }
  if (string2_is8_bit)
    return -CodePointCompare8To16(string2, string1);
  return CodePointCompare16(string1, string2);
}

static inline bool IsSpaceOrNewline(UChar c) {
  // Use IsASCIISpace() for basic Latin-1.
  // This will include newlines, which aren't included in Unicode DirWS.
  return c <= 0x7F
             ? WTF::IsASCIISpace(c)
             : WTF::Unicode::Direction(c) == WTF::Unicode::kWhiteSpaceNeutral;
}

inline RefPtr<StringImpl> StringImpl::IsolatedCopy() const {
  if (Is8Bit())
    return Create(Characters8(), length_);
  return Create(Characters16(), length_);
}

template <typename BufferType>
inline void StringImpl::AppendTo(BufferType& result,
                                 unsigned start,
                                 unsigned length) const {
  unsigned number_of_characters_to_copy = std::min(length, length_ - start);
  if (!number_of_characters_to_copy)
    return;
  if (Is8Bit())
    result.Append(Characters8() + start, number_of_characters_to_copy);
  else
    result.Append(Characters16() + start, number_of_characters_to_copy);
}

template <typename BufferType>
inline void StringImpl::PrependTo(BufferType& result,
                                  unsigned start,
                                  unsigned length) const {
  unsigned number_of_characters_to_copy = std::min(length, length_ - start);
  if (!number_of_characters_to_copy)
    return;
  if (Is8Bit())
    result.Prepend(Characters8() + start, number_of_characters_to_copy);
  else
    result.Prepend(Characters16() + start, number_of_characters_to_copy);
}

// TODO(rob.buis) possibly find a better place for this method.
// Turns a UChar32 to uppercase based on localeIdentifier.
WTF_EXPORT UChar32 ToUpper(UChar32, const AtomicString& locale_identifier);

struct StringHash;

// StringHash is the default hash for StringImpl* and RefPtr<StringImpl>
template <typename T>
struct DefaultHash;
template <>
struct DefaultHash<StringImpl*> {
  typedef StringHash Hash;
};
template <>
struct DefaultHash<RefPtr<StringImpl>> {
  typedef StringHash Hash;
};

}  // namespace WTF

using WTF::StringImpl;
using WTF::kTextCaseASCIIInsensitive;
using WTF::kTextCaseUnicodeInsensitive;
using WTF::kTextCaseSensitive;
using WTF::TextCaseSensitivity;
using WTF::Equal;
using WTF::EqualNonNull;
using WTF::LengthOfNullTerminatedString;
using WTF::ReverseFind;

#endif
