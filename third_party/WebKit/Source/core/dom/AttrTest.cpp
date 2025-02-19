// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/dom/Attr.h"

#include "core/dom/Document.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

class AttrTest : public ::testing::Test {
 protected:
  void SetUp() override;

  Attr* CreateAttribute();
  const AtomicString& Value() const { return value_; }

 private:
  Persistent<Document> document_;
  AtomicString value_;
};

void AttrTest::SetUp() {
  document_ = Document::CreateForTest();
  value_ = "value";
}

Attr* AttrTest::CreateAttribute() {
  return document_->createAttribute("name", ASSERT_NO_EXCEPTION);
}

TEST_F(AttrTest, InitialValueState) {
  Attr* attr = CreateAttribute();
  EXPECT_EQ(g_empty_atom, attr->value());
  EXPECT_EQ(g_empty_string, attr->ToNode()->nodeValue());
  EXPECT_EQ(g_empty_string, attr->textContent());
}

TEST_F(AttrTest, SetValue) {
  Attr* attr = CreateAttribute();
  attr->setValue(Value());
  EXPECT_EQ(Value(), attr->value());
  EXPECT_EQ(Value(), attr->ToNode()->nodeValue());
  EXPECT_EQ(Value(), attr->textContent());
}

TEST_F(AttrTest, SetNodeValue) {
  Attr* attr = CreateAttribute();
  attr->ToNode()->setNodeValue(Value());
  EXPECT_EQ(Value(), attr->value());
  EXPECT_EQ(Value(), attr->ToNode()->nodeValue());
  EXPECT_EQ(Value(), attr->textContent());
}

TEST_F(AttrTest, SetTextContent) {
  Attr* attr = CreateAttribute();
  attr->setTextContent(Value());
  EXPECT_EQ(Value(), attr->value());
  EXPECT_EQ(Value(), attr->ToNode()->nodeValue());
  EXPECT_EQ(Value(), attr->textContent());
}

}  // namespace blink
