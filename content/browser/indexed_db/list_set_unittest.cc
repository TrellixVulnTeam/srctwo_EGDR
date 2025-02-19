// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/indexed_db/list_set.h"

#include <memory>

#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace content {

TEST(ListSetTest, ListSetIterator) {
  list_set<int> set;
  for (int i = 3; i > 0; --i)
    set.insert(i);

  list_set<int>::iterator it = set.begin();
  EXPECT_EQ(3, *it);
  ++it;
  EXPECT_EQ(2, *it);
  it++;
  EXPECT_EQ(1, *it);
  --it;
  EXPECT_EQ(2, *it);
  it--;
  EXPECT_EQ(3, *it);
  ++it;
  EXPECT_EQ(2, *it);
  it++;
  EXPECT_EQ(1, *it);
  ++it;
  EXPECT_EQ(set.end(), it);
}

TEST(ListSetTest, ListSetConstIterator) {
  list_set<int> set;
  for (int i = 5; i > 0; --i)
    set.insert(i);

  const list_set<int>& ref = set;

  list_set<int>::const_iterator it = ref.begin();
  for (int i = 5; i > 0; --i) {
    EXPECT_EQ(i, *it);
    ++it;
  }
  EXPECT_EQ(ref.end(), it);
}

TEST(ListSetTest, ListSetInsertFront) {
  list_set<int> set;
  for (int i = 5; i > 0; --i)
    set.insert(i);
  for (int i = 6; i <= 10; ++i)
    set.insert_front(i);

  const list_set<int>& ref = set;

  list_set<int>::const_iterator it = ref.begin();
  for (int i = 10; i > 0; --i) {
    EXPECT_EQ(i, *it);
    ++it;
  }
  EXPECT_EQ(ref.end(), it);
}

TEST(ListSetTest, ListSetPrimitive) {
  list_set<int> set;
  EXPECT_TRUE(set.empty());
  EXPECT_EQ(0u, set.size());
  {
    list_set<int>::iterator it = set.begin();
    EXPECT_EQ(set.end(), it);
  }

  for (int i = 5; i > 0; --i)
    set.insert(i);
  EXPECT_EQ(5u, set.size());
  EXPECT_FALSE(set.empty());

  set.erase(3);
  EXPECT_EQ(4u, set.size());

  EXPECT_EQ(1u, set.count(2));
  set.erase(2);
  EXPECT_EQ(0u, set.count(2));
  EXPECT_EQ(3u, set.size());

  {
    list_set<int>::iterator it = set.begin();
    EXPECT_EQ(5, *it);
    ++it;
    EXPECT_EQ(4, *it);
    ++it;
    EXPECT_EQ(1, *it);
    ++it;
    EXPECT_EQ(set.end(), it);
  }

  set.erase(1);
  set.erase(4);
  set.erase(5);

  EXPECT_EQ(0u, set.size());
  EXPECT_TRUE(set.empty());
  {
    list_set<int>::iterator it = set.begin();
    EXPECT_EQ(set.end(), it);
  }
}

template <typename T>
class Wrapped {
 public:
  explicit Wrapped(const T& value) : value_(value) {}
  explicit Wrapped(const Wrapped<T>& other) : value_(other.value_) {}
  Wrapped& operator=(const Wrapped<T>& rhs) {
    value_ = rhs.value_;
    return *this;
  }
  int value() const { return value_; }
  bool operator<(const Wrapped<T>& rhs) const { return value_ < rhs.value_; }
  bool operator==(const Wrapped<T>& rhs) const { return value_ == rhs.value_; }

 private:
  T value_;
};

TEST(ListSetTest, ListSetObject) {
  list_set<Wrapped<int> > set;
  EXPECT_EQ(0u, set.size());
  {
    list_set<Wrapped<int> >::iterator it = set.begin();
    EXPECT_EQ(set.end(), it);
  }

  set.insert(Wrapped<int>(0));
  set.insert(Wrapped<int>(1));
  set.insert(Wrapped<int>(2));

  EXPECT_EQ(3u, set.size());

  {
    list_set<Wrapped<int> >::iterator it = set.begin();
    EXPECT_EQ(0, it->value());
    ++it;
    EXPECT_EQ(1, it->value());
    ++it;
    EXPECT_EQ(2, it->value());
    ++it;
    EXPECT_EQ(set.end(), it);
  }

  set.erase(Wrapped<int>(0));
  set.erase(Wrapped<int>(1));
  set.erase(Wrapped<int>(2));

  EXPECT_EQ(0u, set.size());
  {
    list_set<Wrapped<int> >::iterator it = set.begin();
    EXPECT_EQ(set.end(), it);
  }
}

TEST(ListSetTest, ListSetPointer) {
  std::unique_ptr<Wrapped<int>> w0 = base::MakeUnique<Wrapped<int>>(0);
  std::unique_ptr<Wrapped<int>> w1 = base::MakeUnique<Wrapped<int>>(1);
  std::unique_ptr<Wrapped<int>> w2 = base::MakeUnique<Wrapped<int>>(2);

  list_set<Wrapped<int>*> set;
  EXPECT_EQ(0u, set.size());
  {
    list_set<Wrapped<int>*>::iterator it = set.begin();
    EXPECT_EQ(set.end(), it);
  }

  set.insert(w0.get());
  set.insert(w1.get());
  set.insert(w2.get());

  EXPECT_EQ(3u, set.size());

  {
    list_set<Wrapped<int>*>::iterator it = set.begin();
    EXPECT_EQ(0, (*it)->value());
    ++it;
    EXPECT_EQ(1, (*it)->value());
    ++it;
    EXPECT_EQ(2, (*it)->value());
    ++it;
    EXPECT_EQ(set.end(), it);
  }

  set.erase(w0.get());
  set.erase(w1.get());
  set.erase(w2.get());

  EXPECT_EQ(0u, set.size());
  {
    list_set<Wrapped<int>*>::iterator it = set.begin();
    EXPECT_EQ(set.end(), it);
  }
}

template <typename T>
class RefCounted : public base::RefCounted<RefCounted<T> > {
 public:
  explicit RefCounted(const T& value) : value_(value) {}
  T value() { return value_; }

 private:
  virtual ~RefCounted() {}
  friend class base::RefCounted<RefCounted<T> >;
  T value_;
};

TEST(ListSetTest, ListSetRefCounted) {
  list_set<scoped_refptr<RefCounted<int> > > set;
  EXPECT_EQ(0u, set.size());
  {
    list_set<scoped_refptr<RefCounted<int> > >::iterator it = set.begin();
    EXPECT_EQ(set.end(), it);
  }

  scoped_refptr<RefCounted<int> > r0(new RefCounted<int>(0));
  scoped_refptr<RefCounted<int> > r1(new RefCounted<int>(1));
  scoped_refptr<RefCounted<int> > r2(new RefCounted<int>(2));

  set.insert(r0);
  set.insert(r1);
  set.insert(r2);

  EXPECT_EQ(3u, set.size());

  {
    list_set<scoped_refptr<RefCounted<int> > >::iterator it = set.begin();
    EXPECT_EQ(0, (*it)->value());
    ++it;
    EXPECT_EQ(1, (*it)->value());
    ++it;
    EXPECT_EQ(2, (*it)->value());
    ++it;
    EXPECT_EQ(set.end(), it);
  }

  set.erase(r0);
  set.erase(r1);
  set.erase(r2);

  EXPECT_EQ(0u, set.size());
  {
    list_set<scoped_refptr<RefCounted<int> > >::iterator it = set.begin();
    EXPECT_EQ(set.end(), it);
  }
}

}  // namespace content
