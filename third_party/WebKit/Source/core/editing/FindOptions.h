/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FindOptions_h
#define FindOptions_h

namespace blink {

enum FindOptionFlag {
  kCaseInsensitive = 1 << 0,
  kAtWordStarts = 1 << 1,
  // When combined with AtWordStarts, accepts a match in the middle of a word if
  // the match begins with an uppercase letter followed by a lowercase or
  // non-letter. Accepts several other intra-word matches.
  kTreatMedialCapitalAsWordStart = 1 << 2,
  kBackwards = 1 << 3,
  kWrapAround = 1 << 4,
  kStartInSelection = 1 << 5,
  kWholeWord = 1 << 6,  // WholeWord should imply AtWordStarts
  // TODO(yosin) Once find UI works on flat tree and it doesn't use
  // |rangeOfString()|, we should get rid of |FindAPICall| enum member.
  kFindAPICall = 1 << 7,  // Used for Window.find or execCommand('find')
};

typedef unsigned FindOptions;

}  // namespace blink

#endif  // FindOptions_h
