// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crazy_linker_proc_maps.h"

#include <limits.h>
#include <minitest/minitest.h>
#include "crazy_linker_system_mock.h"

namespace crazy {

namespace {

const char kProcMaps0[] =
    "4000b000-4000c000 r--p 00000000 00:00 0\n"
    "4005c000-40081000 r-xp 00000000 b3:01 141        /system/bin/mksh\n"
    "40082000-40083000 r--p 00025000 b3:01 141        /system/bin/mksh\n"
    "40083000-40084000 rw-p 00026000 b3:01 141        /system/bin/mksh\n"
    "40084000-40088000 rw-p 00000000 00:00 0\n"
    "40088000-40090000 r--s 00000000 00:0b 1704       /dev/__properties__\n"
    "400eb000-400ec000 r--p 00000000 00:00 0\n"
    "40141000-40150000 r-xp 00000000 b3:01 126        /system/bin/linker\n"
    "40150000-40151000 r--p 0000e000 b3:01 126        /system/bin/linker\n"
    "40151000-40152000 rw-p 0000f000 b3:01 126        /system/bin/linker\n"
    "40152000-40153000 rw-p 00000000 00:00 0\n"
    "40231000-40277000 r-xp 00001000 b3:01 638        /system/lib/libc.so\n"
    "40277000-40279000 r--p 00046000 b3:01 638        /system/lib/libc.so\n"
    "40279000-4027b000 rw-p 00048000 b3:01 638        /system/lib/libc.so\n"
    "4027b000-40289000 rw-p 00000000 00:00 0\n"
    "41e6b000-41e72000 rw-p 00000000 00:00 0          [heap]\n"
    "be91b000-be93c000 rw-p 00000000 00:00 0          [stack]\n"
    "ffff0000-ffff1000 r-xp 00000000 00:00 0          [vectors]\n";

class ScopedTestEnv {
 public:
  ScopedTestEnv() : sys_() {
    sys_.AddRegularFile("/proc/self/maps", kProcMaps0, sizeof(kProcMaps0) - 1);
  }

  ~ScopedTestEnv() {}

 private:
  SystemMock sys_;
};

}  // namespace

TEST(ProcMaps, FindElfBinaryForAddress) {
  ScopedTestEnv env;
  char path[512];
  uintptr_t load_address;

  EXPECT_TRUE(FindElfBinaryForAddress(
      reinterpret_cast<void*>(0x400694c2), &load_address, path, sizeof(path)));
  EXPECT_EQ(0x4005c000, load_address);
  EXPECT_STREQ("/system/bin/mksh", path);
}

TEST(ProcMaps, FindElfBinaryForAddressWithBadAddress) {
  ScopedTestEnv env;
  char path[512];
  uintptr_t load_address;

  EXPECT_FALSE(FindElfBinaryForAddress(
      reinterpret_cast<void*>(0x50000000), &load_address, path, sizeof(path)));
}

TEST(ProcMaps, FindProtectionFlagsForAddress) {
  ScopedTestEnv env;
  static const struct {
    uintptr_t address;
    bool success;
    int prot;
  } kData[] = {{0x4000afff, false, 0},
               {0x4000b000, true, PROT_READ},
               {0x4000bfff, true, PROT_READ},
               {0x4000c000, false, 0},
               {0x4005bfff, false, 0},
               {0x4005c000, true, PROT_READ | PROT_EXEC},
               {0x40067832, true, PROT_READ | PROT_EXEC},
               {0x40082000, true, PROT_READ},
               {0x40083000, true, PROT_READ | PROT_WRITE},
               {0x40084000, true, PROT_READ | PROT_WRITE}, };

  int prot;
  for (size_t n = 0; n < ARRAY_LEN(kData); ++n) {
    void* address = reinterpret_cast<void*>(kData[n].address);
    TEST_TEXT << minitest::Format("Checking address %p", address);
    EXPECT_EQ(kData[n].success, FindProtectionFlagsForAddress(address, &prot));
    if (kData[n].success) {
      TEST_TEXT << minitest::Format("Checking address %p", address);
      EXPECT_EQ(kData[n].prot, prot);
    }
  }
}

TEST(ProcMaps, FindLoadAddressForFile) {
  ScopedTestEnv env;
  static const struct {
    bool success;
    uintptr_t address;
    uintptr_t offset;
    const char* name;
  } kData[] = {{true, 0x4005c000, 0, "mksh"},
               {true, 0x40141000, 0, "/system/bin/linker"},
               {false, 0, 0, "[heap]"},
               {false, 0, 0, "bin/mksh"},
               {true, 0x4005c000, 0, "/system/bin/mksh"},
               {true, 0x40231000, 0x1000000, "libc.so"}, };
  for (size_t n = 0; n < ARRAY_LEN(kData); ++n) {
    uintptr_t address, offset;
    TEST_TEXT << "Checking " << kData[n].name;
    bool success = FindLoadAddressForFile(kData[n].name, &address, &offset);
    EXPECT_EQ(kData[n].success, success);
    if (success) {
      TEST_TEXT << "Checking " << kData[n].name;
      EXPECT_EQ(kData[n].address, address);

      TEST_TEXT << "Checking " << kData[n].name;
      EXPECT_EQ(kData[n].offset, offset);
    }
  }
}

TEST(ProcMaps, GetNextEntry) {
  ScopedTestEnv env;
  //     "4000b000-4000c000 r--p 00000000 00:00 0\n"
  //     "4005c000-40081000 r-xp 00000000 b3:01 141        /system/bin/mksh\n"
  //     "40082000-40083000 r--p 00025000 b3:01 141        /system/bin/mksh\n"
  //     "40083000-40084000 rw-p 00026000 b3:01 141        /system/bin/mksh\n"
  //     "40084000-40088000 rw-p 00000000 00:00 0\n"
  //     "40088000-40090000 r--s 00000000 00:0b 1704
  // /dev/__properties__\n"
  //     "400eb000-400ec000 r--p 00000000 00:00 0\n"
  //     "40141000-40150000 r-xp 00000000 b3:01 126        /system/bin/linker\n"
  //     "40150000-40151000 r--p 0000e000 b3:01 126        /system/bin/linker\n"
  //     "40151000-40152000 rw-p 0000f000 b3:01 126        /system/bin/linker\n"
  //     "40152000-40153000 rw-p 00000000 00:00 0\n"
  //     "40231000-40277000 r-xp 00001000 b3:01 638
  // /system/lib/libc.so\n"
  //     "40277000-40279000 r--p 00046000 b3:01 638
  // /system/lib/libc.so\n"
  //     "40279000-4027b000 rw-p 00048000 b3:01 638
  // /system/lib/libc.so\n"
  //     "4027b000-40289000 rw-p 00000000 00:00 0\n"
  //     "41e6b000-41e72000 rw-p 00000000 00:00 0          [heap]\n"
  //     "be91b000-be93c000 rw-p 00000000 00:00 0          [stack]\n"
  //     "ffff0000-ffff1000 r-xp 00000000 00:00 0          [vectors]\n"
  static const struct {
    size_t vma_start;
    size_t vma_end;
    int prot_flags;
    size_t load_offset;
    const char* path;
  } kData[] = {
        {0x4000b000, 0x4000c000, PROT_READ, 0, NULL},
        {0x4005c000, 0x40081000, PROT_READ | PROT_EXEC, 0, "/system/bin/mksh"},
        {0x40082000,          0x40083000,        PROT_READ,
         0x25000 * PAGE_SIZE, "/system/bin/mksh"},
        {0x40083000,          0x40084000,        PROT_READ | PROT_WRITE,
         0x26000 * PAGE_SIZE, "/system/bin/mksh"},
        {0x40084000, 0x40088000, PROT_READ | PROT_WRITE, 0, NULL},
        {0x40088000, 0x40090000, PROT_READ, 0, "/dev/__properties__"},
        {0x400eb000, 0x400ec000, PROT_READ, 0, NULL},
        {0x40141000, 0x40150000,          PROT_READ | PROT_EXEC,
         0,          "/system/bin/linker"},
        {0x40150000,         0x40151000,          PROT_READ,
         0xe000 * PAGE_SIZE, "/system/bin/linker"},
        {0x40151000,         0x40152000,          PROT_READ | PROT_WRITE,
         0xf000 * PAGE_SIZE, "/system/bin/linker"},
        {0x40152000, 0x40153000, PROT_READ | PROT_WRITE, 0, NULL},
        {0x40231000,         0x40277000,           PROT_READ | PROT_EXEC,
         0x1000 * PAGE_SIZE, "/system/lib/libc.so"},
        {0x40277000,          0x40279000,           PROT_READ,
         0x46000 * PAGE_SIZE, "/system/lib/libc.so"},
        {0x40279000,          0x4027b000,           PROT_READ | PROT_WRITE,
         0x48000 * PAGE_SIZE, "/system/lib/libc.so"},
        {0x4027b000, 0x40289000, PROT_READ | PROT_WRITE, 0, NULL},
        {0x41e6b000, 0x41e72000, PROT_READ | PROT_WRITE, 0, "[heap]"},
        {0xbe91b000, 0xbe93c000, PROT_READ | PROT_WRITE, 0, "[stack]"},
        {0xffff0000, 0xffff1000, PROT_READ | PROT_EXEC, 0, "[vectors]"}, };

  ProcMaps self_maps;
  ProcMaps::Entry entry;

  for (size_t n = 0; n < ARRAY_LEN(kData); ++n) {
    minitest::internal::String text =
        minitest::Format("Checking entry #%d %p-%p",
                         n + 1,
                         kData[n].vma_start,
                         kData[n].vma_end);
    TEST_TEXT << text;
    EXPECT_TRUE(self_maps.GetNextEntry(&entry));
    TEST_TEXT << text;
    EXPECT_EQ(kData[n].vma_start, entry.vma_start);
    TEST_TEXT << text;
    EXPECT_EQ(kData[n].vma_end, entry.vma_end);
    TEST_TEXT << text;
    EXPECT_EQ(kData[n].prot_flags, entry.prot_flags);
    TEST_TEXT << text;
    EXPECT_EQ(kData[n].load_offset, entry.load_offset);

    if (!kData[n].path) {
      TEST_TEXT << text;
      EXPECT_FALSE(entry.path);
    } else {
      EXPECT_MEMEQ(
          kData[n].path, strlen(kData[n].path), entry.path, entry.path_len);
    }
  }
  EXPECT_FALSE(self_maps.GetNextEntry(&entry));
}

}  // namespace crazy
