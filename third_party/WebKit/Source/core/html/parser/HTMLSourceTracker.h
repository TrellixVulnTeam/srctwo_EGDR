/*
 * Copyright (C) 2010 Adam Barth. All Rights Reserved.
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

#ifndef HTMLSourceTracker_h
#define HTMLSourceTracker_h

#include "core/html/parser/HTMLToken.h"
#include "platform/text/SegmentedString.h"
#include "platform/wtf/Allocator.h"

namespace blink {

class HTMLTokenizer;

class HTMLSourceTracker {
  DISALLOW_NEW();
  WTF_MAKE_NONCOPYABLE(HTMLSourceTracker);

 public:
  HTMLSourceTracker();

  // FIXME: Once we move "end" into HTMLTokenizer, rename "start" to
  // something that makes it obvious that this method can be called multiple
  // times.
  void Start(SegmentedString&, HTMLTokenizer*, HTMLToken&);
  void end(SegmentedString&, HTMLTokenizer*, HTMLToken&);

  String SourceForToken(const HTMLToken&);

 private:
  bool NeedToCheckTokenizerBuffer(HTMLTokenizer*);

  SegmentedString previous_source_;
  SegmentedString current_source_;

  String cached_source_for_token_;

  bool is_started_;
};

}  // namespace blink

#endif
