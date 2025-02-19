// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/SharedBuffer.h"
#include "platform/heap/Handle.h"
#include "platform/mhtml/ArchiveResource.h"
#include "platform/mhtml/MHTMLParser.h"
#include "platform/testing/BlinkFuzzerTestSupport.h"
#include <stddef.h>
#include <stdint.h>

namespace blink {

// Fuzzer for blink::MHTMLParser.
int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  static BlinkFuzzerTestSupport test_support = BlinkFuzzerTestSupport();
  MHTMLParser mhtml_parser(SharedBuffer::Create(data, size));
  HeapVector<Member<ArchiveResource>> mhtml_archives =
      mhtml_parser.ParseArchive();
  mhtml_archives.clear();
  ThreadState::Current()->CollectAllGarbage();

  return 0;
}

}  // namespace blink

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  return blink::LLVMFuzzerTestOneInput(data, size);
}
