/*
 * Copyright (c) 2013, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DummyPageHolder_h
#define DummyPageHolder_h

#include <memory>
#include "core/frame/LocalFrameClient.h"
#include "core/page/Page.h"
#include "platform/geometry/IntSize.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/Noncopyable.h"

namespace blink {

class Document;
class IntSize;
class LocalFrame;
class LocalFrameView;
class Settings;

typedef void (*FrameSettingOverrideFunction)(Settings&);

// Creates a dummy Page, LocalFrame, and LocalFrameView whose clients are all
// no-op.
//
// This class can be used when you write unit tests for components which do not
// work correctly without layoutObjects.  To make sure the layoutObjects are
// created, you need to call |frameView().layout()| after you add nodes into
// |document()|.
//
// Since DummyPageHolder stores empty clients in it, it must outlive the Page,
// LocalFrame, LocalFrameView and any other objects created by it.
// DummyPageHolder's destructor ensures this condition by checking remaining
// references to the LocalFrame.

class DummyPageHolder {
  WTF_MAKE_NONCOPYABLE(DummyPageHolder);
  USING_FAST_MALLOC(DummyPageHolder);

 public:
  static std::unique_ptr<DummyPageHolder> Create(
      const IntSize& initial_view_size = IntSize(),
      Page::PageClients* = 0,
      LocalFrameClient* = nullptr,
      FrameSettingOverrideFunction = nullptr);
  ~DummyPageHolder();

  Page& GetPage() const;
  LocalFrame& GetFrame() const;
  LocalFrameView& GetFrameView() const;
  Document& GetDocument() const;

 private:
  DummyPageHolder(const IntSize& initial_view_size,
                  Page::PageClients*,
                  LocalFrameClient*,
                  FrameSettingOverrideFunction setting_overrider);

  Persistent<Page> page_;

  // The LocalFrame is accessed from worker threads by unit tests
  // (WorkerThreadableLoaderTest), hence we need to allow cross-thread
  // usage of |m_frame|.
  //
  // TODO: rework the tests to not require cross-thread access.
  CrossThreadPersistent<LocalFrame> frame_;

  Persistent<LocalFrameClient> local_frame_client_;
};

}  // namespace blink

#endif  // DummyPageHolder_h
