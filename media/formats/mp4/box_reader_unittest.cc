// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/formats/mp4/box_reader.h"

#include <stdint.h>
#include <string.h>

#include <memory>

#include "base/logging.h"
#include "media/base/mock_media_log.h"
#include "media/formats/mp4/box_definitions.h"
#include "media/formats/mp4/rcheck.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::HasSubstr;
using ::testing::StrictMock;

namespace media {
namespace mp4 {

static const uint8_t kSkipBox[] = {
    // Top-level test box containing three children
    0x00, 0x00, 0x00, 0x40, 's', 'k', 'i', 'p', 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0xf9, 0x0a, 0x0b, 0x0c, 0xfd, 0x0e, 0x0f, 0x10,
    // Ordinary (8-byte header) child box
    0x00, 0x00, 0x00, 0x0c, 'p', 's', 's', 'h', 0xde, 0xad, 0xbe, 0xef,
    // Extended-size header child box
    0x00, 0x00, 0x00, 0x01, 'p', 's', 's', 'h', 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x14, 0xfa, 0xce, 0xca, 0xfe,
    // Empty free box
    0x00, 0x00, 0x00, 0x08, 'f', 'r', 'e', 'e',
    // Trailing garbage
    0x00};

struct FreeBox : Box {
  bool Parse(BoxReader* reader) override {
    return true;
  }
  FourCC BoxType() const override { return FOURCC_FREE; }
};

struct PsshBox : Box {
  uint32_t val;

  bool Parse(BoxReader* reader) override {
    return reader->Read4(&val);
  }
  FourCC BoxType() const override { return FOURCC_PSSH; }
};

struct SkipBox : Box {
  uint8_t a, b;
  uint16_t c;
  int32_t d;
  int64_t e;

  std::vector<PsshBox> kids;
  FreeBox mpty;

  bool Parse(BoxReader* reader) override {
    RCHECK(reader->ReadFullBoxHeader() &&
           reader->Read1(&a) &&
           reader->Read1(&b) &&
           reader->Read2(&c) &&
           reader->Read4s(&d) &&
           reader->Read4sInto8s(&e));
    return reader->ScanChildren() &&
           reader->ReadChildren(&kids) &&
           reader->MaybeReadChild(&mpty);
  }
  FourCC BoxType() const override { return FOURCC_SKIP; }

  SkipBox();
  ~SkipBox() override;
};

SkipBox::SkipBox() {}
SkipBox::~SkipBox() {}

class BoxReaderTest : public testing::Test {
 public:
  BoxReaderTest() {}

 protected:
  std::vector<uint8_t> GetBuf() {
    return std::vector<uint8_t>(kSkipBox, kSkipBox + sizeof(kSkipBox));
  }

  void TestTopLevelBox(const uint8_t* data, size_t data_size, uint32_t fourCC) {
    std::vector<uint8_t> buf(data, data + data_size);

    bool err;
    std::unique_ptr<BoxReader> reader(
        BoxReader::ReadTopLevelBox(&buf[0], buf.size(), &media_log_, &err));

    EXPECT_FALSE(err);
    EXPECT_TRUE(reader);
    EXPECT_EQ(fourCC, reader->type());
    EXPECT_EQ(reader->box_size(), data_size);
  }

  template <typename ChildType>
  void TestParsing32bitOverflow(const uint8_t* buffer,
                                size_t size,
                                const std::string& overflow_error) {
    // Wrap whatever we're passed in a dummy EMSG so we can satisfy requirements
    // for ReadTopLevelBox and to kick off parsing.
    std::vector<uint8_t> buffer_wrapper = {
        0x00, 0x00, 0x00, 0x00,  // dummy size
        'e',  'm',  's',  'g',   // fourcc
    };
    buffer_wrapper.insert(buffer_wrapper.end(), buffer, buffer + size);

    // Basic check of the nested buffer size. If box_size > buffer size the test
    // will exit early (waiting for more bytes to be appended).
    ASSERT_TRUE(base::IsValueInRangeForNumericType<uint8_t>(size));
    ASSERT_LE(buffer[3], size);

    // Update the size (keep it simple).
    ASSERT_TRUE(
        base::IsValueInRangeForNumericType<uint8_t>(buffer_wrapper.size()));
    buffer_wrapper[3] = buffer_wrapper.size();

    bool err;
    std::unique_ptr<BoxReader> reader(BoxReader::ReadTopLevelBox(
        &buffer_wrapper[0], buffer_wrapper.size(), &media_log_, &err));
    EXPECT_FALSE(err);
    EXPECT_TRUE(reader);
    EXPECT_EQ(FOURCC_EMSG, reader->type());

// Overflow is only triggered/caught on 32-bit systems. 64-bit systems will
// instead fail parsing because tests are written such that |buffer| never
// contains enough bytes for parsing to succeed.
#if defined(ARCH_CPU_32_BITS)
    const int kOverflowLogCount = 1;
#else
    const int kOverflowLogCount = 0;
#endif

    if (!overflow_error.empty())
      EXPECT_MEDIA_LOG(HasSubstr(overflow_error)).Times(kOverflowLogCount);

    std::vector<ChildType> children;
    EXPECT_FALSE(reader->ReadAllChildrenAndCheckFourCC(&children));
  }

  StrictMock<MockMediaLog> media_log_;
};

TEST_F(BoxReaderTest, ExpectedOperationTest) {
  std::vector<uint8_t> buf = GetBuf();
  bool err;
  std::unique_ptr<BoxReader> reader(
      BoxReader::ReadTopLevelBox(&buf[0], buf.size(), &media_log_, &err));
  EXPECT_FALSE(err);
  EXPECT_TRUE(reader.get());

  SkipBox box;
  EXPECT_TRUE(box.Parse(reader.get()));
  EXPECT_EQ(0x01, reader->version());
  EXPECT_EQ(0x020304u, reader->flags());
  EXPECT_EQ(0x05, box.a);
  EXPECT_EQ(0x06, box.b);
  EXPECT_EQ(0x0708, box.c);
  EXPECT_EQ(static_cast<int32_t>(0xf90a0b0c), box.d);
  EXPECT_EQ(static_cast<int32_t>(0xfd0e0f10), box.e);

  EXPECT_EQ(2u, box.kids.size());
  EXPECT_EQ(0xdeadbeef, box.kids[0].val);
  EXPECT_EQ(0xfacecafe, box.kids[1].val);

  // Accounting for the extra byte outside of the box above
  EXPECT_EQ(buf.size(), static_cast<uint64_t>(reader->box_size() + 1));
}

TEST_F(BoxReaderTest, OuterTooShortTest) {
  std::vector<uint8_t> buf = GetBuf();
  bool err;

  // Create a soft failure by truncating the outer box.
  std::unique_ptr<BoxReader> r(
      BoxReader::ReadTopLevelBox(&buf[0], buf.size() - 2, &media_log_, &err));

  EXPECT_FALSE(err);
  EXPECT_FALSE(r.get());
}

TEST_F(BoxReaderTest, InnerTooLongTest) {
  std::vector<uint8_t> buf = GetBuf();
  bool err;

  // Make an inner box too big for its outer box.
  buf[25] = 1;
  std::unique_ptr<BoxReader> reader(
      BoxReader::ReadTopLevelBox(&buf[0], buf.size(), &media_log_, &err));

  SkipBox box;
  EXPECT_FALSE(box.Parse(reader.get()));
}

TEST_F(BoxReaderTest, WrongFourCCTest) {
  std::vector<uint8_t> buf = GetBuf();
  bool err;

  // Set an unrecognized top-level FourCC.
  buf[4] = 0x44;
  buf[5] = 0x41;
  buf[6] = 0x4c;
  buf[7] = 0x45;

  EXPECT_MEDIA_LOG(HasSubstr("Unrecognized top-level box type DALE"));

  std::unique_ptr<BoxReader> reader(
      BoxReader::ReadTopLevelBox(&buf[0], buf.size(), &media_log_, &err));
  EXPECT_FALSE(reader.get());
  EXPECT_TRUE(err);
}

TEST_F(BoxReaderTest, ScanChildrenTest) {
  std::vector<uint8_t> buf = GetBuf();
  bool err;
  std::unique_ptr<BoxReader> reader(
      BoxReader::ReadTopLevelBox(&buf[0], buf.size(), &media_log_, &err));

  EXPECT_TRUE(reader->SkipBytes(16) && reader->ScanChildren());

  FreeBox free;
  EXPECT_TRUE(reader->ReadChild(&free));
  EXPECT_FALSE(reader->ReadChild(&free));
  EXPECT_TRUE(reader->MaybeReadChild(&free));

  std::vector<PsshBox> kids;

  EXPECT_TRUE(reader->ReadChildren(&kids));
  EXPECT_EQ(2u, kids.size());
  kids.clear();
  EXPECT_FALSE(reader->ReadChildren(&kids));
  EXPECT_TRUE(reader->MaybeReadChildren(&kids));
}

TEST_F(BoxReaderTest, ReadAllChildrenTest) {
  std::vector<uint8_t> buf = GetBuf();
  // Modify buffer to exclude its last 'free' box
  buf[3] = 0x38;
  bool err;
  std::unique_ptr<BoxReader> reader(
      BoxReader::ReadTopLevelBox(&buf[0], buf.size(), &media_log_, &err));

  std::vector<PsshBox> kids;
  EXPECT_TRUE(reader->SkipBytes(16) && reader->ReadAllChildren(&kids));
  EXPECT_EQ(2u, kids.size());
  EXPECT_EQ(kids[0].val, 0xdeadbeef);   // Ensure order is preserved
}

TEST_F(BoxReaderTest, SkippingBloc) {
  static const uint8_t kData[] = {0x00, 0x00, 0x00, 0x09, 'b',
                                  'l',  'o',  'c',  0x00};

  TestTopLevelBox(kData, sizeof(kData), FOURCC_BLOC);
}

TEST_F(BoxReaderTest, SkippingEmsg) {
  static const uint8_t kData[] = {
      0x00, 0x00, 0x00, 0x24, 'e', 'm', 's', 'g',
      0x00,                    // version = 0
      0x00, 0x00, 0x00,        // flags = 0
      0x61, 0x00,              // scheme_id_uri = "a"
      0x61, 0x00,              // value = "a"
      0x00, 0x00, 0x00, 0x01,  // timescale = 1
      0x00, 0x00, 0x00, 0x02,  // presentation_time_delta = 2
      0x00, 0x00, 0x00, 0x03,  // event_duration = 3
      0x00, 0x00, 0x00, 0x04,  // id = 4
      0x05, 0x06, 0x07, 0x08,  // message_data[4] = 0x05060708
  };

  TestTopLevelBox(kData, sizeof(kData), FOURCC_EMSG);
}

TEST_F(BoxReaderTest, SkippingUuid) {
  static const uint8_t kData[] = {
      0x00, 0x00, 0x00, 0x19, 'u',  'u',  'i',  'd',
      0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
      0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,  // usertype
      0x00,
  };

  TestTopLevelBox(kData, sizeof(kData), FOURCC_UUID);
}

TEST_F(BoxReaderTest, NestedBoxWithHugeSize) {
  // This data is not a valid 'emsg' box. It is just used as a top-level box
  // as ReadTopLevelBox() has a restricted set of boxes it allows. |kData|
  // contains all the bytes as specified by the 'emsg' header size.
  // The nested box ('junk') has a large size that was chosen to catch
  // integer overflows. The nested box should not specify more than the
  // number of remaining bytes in the enclosing box.
  static const uint8_t kData[] = {
      0x00, 0x00, 0x00, 0x24, 'e',  'm',  's',  'g',   // outer box
      0x7f, 0xff, 0xff, 0xff, 'j',  'u',  'n',  'k',   // nested box
      0x00, 0x01, 0x00, 0xff, 0xff, 0x00, 0x3b, 0x03,  // random data for rest
      0x00, 0x01, 0x00, 0x03, 0x00, 0x03, 0x00, 0x04, 0x05, 0x06, 0x07, 0x08};

  bool err;
  std::unique_ptr<BoxReader> reader(
      BoxReader::ReadTopLevelBox(kData, sizeof(kData), &media_log_, &err));

  EXPECT_FALSE(err);
  EXPECT_TRUE(reader);
  EXPECT_EQ(FOURCC_EMSG, reader->type());
  EXPECT_FALSE(reader->ScanChildren());
}

TEST_F(BoxReaderTest, ScanChildrenWithInvalidChild) {
  // This data is not a valid 'emsg' box. It is just used as a top-level box
  // as ReadTopLevelBox() has a restricted set of boxes it allows.
  // The nested 'elst' box is used as it includes a count of EditListEntry's.
  // The sample specifies a large number of EditListEntry's, but only 1 is
  // actually included in the box. This test verifies that the code checks
  // properly that the buffer contains the specified number of EditListEntry's
  static const uint8_t kData[] = {
      0x00, 0x00, 0x00, 0x2c, 'e',  'm',  's',  'g',  // outer box
      0x00, 0x00, 0x00, 0x24, 'e',  'l',  's',  't',  // nested box
      0x01, 0x00, 0x00, 0x00,                         // version = 1, flags = 0
      0x00, 0x00, 0x00, 0x0a,  // count = 10, but only 1 actually included
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  bool err;
  std::unique_ptr<BoxReader> reader(
      BoxReader::ReadTopLevelBox(kData, sizeof(kData), &media_log_, &err));

  EXPECT_FALSE(err);
  EXPECT_TRUE(reader);
  EXPECT_EQ(FOURCC_EMSG, reader->type());
  EXPECT_TRUE(reader->ScanChildren());

  // 'elst' specifies lots of EditListEntry's but only includes 1. Thus
  // parsing it should fail.
  EditList child;
  EXPECT_FALSE(reader->ReadChild(&child));
}

TEST_F(BoxReaderTest, ReadAllChildrenWithChildLargerThanParent) {
  static const uint8_t kData[] = {
      0x00, 0x00, 0x00, 0x10, 's', 'k', 'i', 'p',  // outer box
      0x00, 0x00, 0x00, 0x10, 'p', 's', 's', 'h',  // nested box
  };

  bool err;
  std::unique_ptr<BoxReader> reader(
      BoxReader::ReadTopLevelBox(kData, sizeof(kData), &media_log_, &err));

  EXPECT_FALSE(err);
  EXPECT_TRUE(reader);
  EXPECT_EQ(FOURCC_SKIP, reader->type());

  std::vector<PsshBox> tmp;
  EXPECT_FALSE(reader->ReadAllChildren(&tmp));
}

TEST_F(BoxReaderTest, TrunSampleCount32bitOverflow) {
  // This 'trun' box specifies an unusually high sample count, though only one
  // sample is  included in the bytes below. The values for "sample_count" and
  // "flags" are chosen such that the needed number of bytes will overflow 32
  // bits to yield a very small number (4), potentially passing the
  // internal check for HasBytes(). http://crbug.com/679640
  static const uint8_t kData[] = {
      0x00, 0x00, 0x00, 0x18, 't', 'r', 'u', 'n',  // header
      0x00, 0x00,                                  // version = 0
      0x03, 0x00,  // flags = 2 fields present (sample duration and sample size)
      0x80, 0x00, 0x00, 0x02,  // sample count = 2147483650
      0x00, 0x00, 0x00, 0x00,  // only one sample present
      0x00, 0x00, 0x00, 0x00};

  // Verify we catch the overflow to avoid OOB reads/writes.
  TestParsing32bitOverflow<TrackFragmentRun>(
      kData, sizeof(kData),
      "Extreme TRUN sample count exceeds implementation limit.");
}

TEST_F(BoxReaderTest, SaioCount32bitOverflow) {
  // This 'saio' box specifies an unusually high number of offset counts, though
  // only one offset is included in the bytes below. The values for "count" and
  // "version" are chosen such that the needed number of bytes will overflow 32
  // bits to yield a very small number (4), potentially passing the internal
  // check for HasBytes(). http://crbug.com/679641
  static const uint8_t kData[] = {
      0x00, 0x00, 0x00, 0x14, 's', 'a', 'i', 'o',  // header
      0x00, 0x00,              // version = 0 (4 bytes per offset entry)
      0x00, 0x00,              // flags = 0
      0x40, 0x00, 0x00, 0x01,  // offsets count = 1073741825
      0x00, 0x00, 0x00, 0x00,  // single offset entry
  };

  // Verify we catch the overflow to avoid OOB reads/writes.
  TestParsing32bitOverflow<SampleAuxiliaryInformationOffset>(
      kData, sizeof(kData), "Extreme SAIO count exceeds implementation limit.");
}

TEST_F(BoxReaderTest, ElstCount32bitOverflow) {
  // This 'elst' box specifies an unusually high number of edit counts, though
  // only one edit is included in the bytes below. The values for "count" and
  // "version" are chosen such that the needed number of bytes will overflow 32
  // bits to yield a very small number (12), potentially passing the internal
  // check for HasBytes(). http://crbug.com/679645
  static const uint8_t kData[] = {
      0x00, 0x00, 0x00, 0x1c, 'e', 'l', 's', 't',  // header
      0x00, 0x00,              // version = 0 (12 bytes per edit entry)
      0x00, 0x00,              // flags = 0
      0x80, 0x00, 0x00, 0x01,  // edits count = 2147483649
      0x00, 0x00, 0x00, 0x00,  // single edit entry
      0x00, 0x00, 0x00, 0x00,  // ...
      0x00, 0x00, 0x00, 0x00,
  };

  // Verify we catch the overflow to avoid OOB reads/writes.
  TestParsing32bitOverflow<EditList>(
      kData, sizeof(kData), "Extreme ELST count exceeds implementation limit.");
}

TEST_F(BoxReaderTest, SbgpCount32bitOverflow) {
  // This 'sbgp' box specifies an unusually high count of entries, though only
  // one partial entry is included in the bytes below. The value for "count" is
  // chosen such that we could overflow attempting to allocate the vector for
  // parsed entries. http://crbug.com/679646
  static const uint8_t kData[] = {
      0x00, 0x00, 0x00, 0x1c, 's', 'b', 'g', 'p',  // header
      0x00, 0x00, 0x00, 0x00,                      // version = 0, flags = 0
      's',  'e',  'i',  'g',                       // required grouping "type"
      0xff, 0xff, 0xff, 0xff,                      // count = 4294967295
      0x00, 0x00, 0x00, 0x00,                      // partial entry
      0x00, 0x00, 0x00, 0x00,
  };

  // Verify we catch the overflow to avoid OOB reads/writes.
  TestParsing32bitOverflow<SampleToGroup>(
      kData, sizeof(kData), "Extreme SBGP count exceeds implementation limit.");
}

TEST_F(BoxReaderTest, SgpdCount32bitOverflow) {
  // This 'sgpd' box specifies an unusually high count of entries, though only
  // one partial entry is included in the bytes below. The value for "count" is
  // chosen such that we could overflow attempting to allocate the vector for
  // parsed entries. http://crbug.com/679647
  static const uint8_t kData[] = {
      0x00, 0x00, 0x00, 0x1c, 's', 'g', 'p', 'd',  // header
      0x00, 0x00, 0x00, 0x00,                      // version = 0, flags = 0
      's',  'e',  'i',  'g',                       // required grouping "type"
      0xff, 0xff, 0xff, 0xff,                      // count = 4294967295
      0x00, 0x00, 0x00, 0x00,                      // partial entry
      0x00, 0x00, 0x00, 0x00,
  };

  // Verify we catch the overflow to avoid OOB reads/writes.
  TestParsing32bitOverflow<SampleGroupDescription>(
      kData, sizeof(kData), "Extreme SGPD count exceeds implementation limit.");
}

}  // namespace mp4
}  // namespace media
