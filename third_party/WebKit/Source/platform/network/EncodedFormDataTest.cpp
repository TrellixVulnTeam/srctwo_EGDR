// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/network/EncodedFormData.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

namespace {

class EncodedFormDataTest : public ::testing::Test {
 public:
  void CheckDeepCopied(const String& a, const String& b) {
    EXPECT_EQ(a, b);
    if (b.Impl())
      EXPECT_NE(a.Impl(), b.Impl());
  }

  void CheckDeepCopied(const KURL& a, const KURL& b) {
    EXPECT_EQ(a, b);
    CheckDeepCopied(a.GetString(), b.GetString());
    if (a.InnerURL() && b.InnerURL())
      CheckDeepCopied(*a.InnerURL(), *b.InnerURL());
  }

  void CheckDeepCopied(const FormDataElement& a, const FormDataElement& b) {
    EXPECT_EQ(a, b);
    CheckDeepCopied(a.filename_, b.filename_);
    CheckDeepCopied(a.blob_uuid_, b.blob_uuid_);
    CheckDeepCopied(a.file_system_url_, b.file_system_url_);
  }
};

TEST_F(EncodedFormDataTest, DeepCopy) {
  RefPtr<EncodedFormData> original(EncodedFormData::Create());
  original->AppendData("Foo", 3);
  original->AppendFileRange("example.txt", 12345, 56789, 9999.0);
  original->AppendBlob("originalUUID", nullptr);
  original->AppendFileSystemURLRange(KURL(NullURL(), "ws://localhost/"), 23456,
                                     34567, 1111.0);

  Vector<char> boundary_vector;
  boundary_vector.Append("----boundaryForTest", 19);
  original->SetIdentifier(45678);
  original->SetBoundary(boundary_vector);
  original->SetContainsPasswordData(true);

  RefPtr<EncodedFormData> copy = original->DeepCopy();

  // Check that contents are copied (compare the copy with expected values).
  const Vector<FormDataElement>& original_elements = original->Elements();
  const Vector<FormDataElement>& copy_elements = copy->Elements();
  ASSERT_EQ(4ul, copy_elements.size());

  Vector<char> foo_vector;
  foo_vector.Append("Foo", 3);

  EXPECT_EQ(FormDataElement::kData, copy_elements[0].type_);
  EXPECT_EQ(foo_vector, copy_elements[0].data_);

  EXPECT_EQ(FormDataElement::kEncodedFile, copy_elements[1].type_);
  EXPECT_EQ(String("example.txt"), copy_elements[1].filename_);
  EXPECT_EQ(12345ll, copy_elements[1].file_start_);
  EXPECT_EQ(56789ll, copy_elements[1].file_length_);
  EXPECT_EQ(9999.0, copy_elements[1].expected_file_modification_time_);

  EXPECT_EQ(FormDataElement::kEncodedBlob, copy_elements[2].type_);
  EXPECT_EQ(String("originalUUID"), copy_elements[2].blob_uuid_);

  EXPECT_EQ(FormDataElement::kEncodedFileSystemURL, copy_elements[3].type_);
  EXPECT_EQ(KURL(NullURL(), String("ws://localhost/")),
            copy_elements[3].file_system_url_);
  EXPECT_EQ(23456ll, copy_elements[3].file_start_);
  EXPECT_EQ(34567ll, copy_elements[3].file_length_);
  EXPECT_EQ(1111.0, copy_elements[3].expected_file_modification_time_);

  EXPECT_EQ(45678, copy->Identifier());
  EXPECT_EQ(boundary_vector, copy->Boundary());
  EXPECT_EQ(true, copy->ContainsPasswordData());

  // Check that contents are copied (compare the copy with the original).
  EXPECT_EQ(*original, *copy);

  // Check pointers are different, i.e. deep-copied.
  ASSERT_NE(original.Get(), copy.Get());

  for (size_t i = 0; i < 4; ++i) {
    if (copy_elements[i].filename_.Impl()) {
      EXPECT_NE(original_elements[i].filename_.Impl(),
                copy_elements[i].filename_.Impl());
      EXPECT_TRUE(copy_elements[i].filename_.Impl()->HasOneRef());
    }

    if (copy_elements[i].blob_uuid_.Impl()) {
      EXPECT_NE(original_elements[i].blob_uuid_.Impl(),
                copy_elements[i].blob_uuid_.Impl());
      EXPECT_TRUE(copy_elements[i].blob_uuid_.Impl()->HasOneRef());
    }

    if (copy_elements[i].file_system_url_.GetString().Impl()) {
      EXPECT_NE(original_elements[i].file_system_url_.GetString().Impl(),
                copy_elements[i].file_system_url_.GetString().Impl());
      EXPECT_TRUE(
          copy_elements[i].file_system_url_.GetString().Impl()->HasOneRef());
    }

    EXPECT_EQ(nullptr, copy_elements[i].file_system_url_.InnerURL());

    // m_optionalBlobDataHandle is not checked, because BlobDataHandle is
    // ThreadSafeRefCounted.
  }
}

}  // namespace

}  // namespace blink
