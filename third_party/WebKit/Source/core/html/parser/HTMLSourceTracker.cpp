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

#include "core/html/parser/HTMLSourceTracker.h"

#include "core/html/parser/HTMLTokenizer.h"
#include "platform/wtf/text/StringBuilder.h"

namespace blink {

HTMLSourceTracker::HTMLSourceTracker() : is_started_(false) {}

void HTMLSourceTracker::Start(SegmentedString& current_input,
                              HTMLTokenizer* tokenizer,
                              HTMLToken& token) {
  if (token.GetType() == HTMLToken::kUninitialized && !is_started_) {
    previous_source_.Clear();
    if (NeedToCheckTokenizerBuffer(tokenizer) &&
        tokenizer->NumberOfBufferedCharacters())
      previous_source_ = tokenizer->BufferedCharacters();
  } else {
    previous_source_.Append(current_source_);
  }

  is_started_ = true;
  current_source_ = current_input;
  token.SetBaseOffset(current_source_.NumberOfCharactersConsumed() -
                      previous_source_.length());
}

void HTMLSourceTracker::end(SegmentedString& current_input,
                            HTMLTokenizer* tokenizer,
                            HTMLToken& token) {
  is_started_ = false;

  cached_source_for_token_ = String();

  // FIXME: This work should really be done by the HTMLTokenizer.
  size_t number_of_buffered_characters = 0u;
  if (NeedToCheckTokenizerBuffer(tokenizer)) {
    number_of_buffered_characters = tokenizer->NumberOfBufferedCharacters();
  }
  token.end(current_input.NumberOfCharactersConsumed() -
            number_of_buffered_characters);
}

String HTMLSourceTracker::SourceForToken(const HTMLToken& token) {
  if (!cached_source_for_token_.IsEmpty())
    return cached_source_for_token_;

  size_t length;
  if (token.GetType() == HTMLToken::kEndOfFile) {
    // Consume the remainder of the input, omitting the null character we use to
    // mark the end of the file.
    length = previous_source_.length() + current_source_.length() - 1;
  } else {
    DCHECK(!token.StartIndex());
    length = static_cast<size_t>(token.EndIndex() - token.StartIndex());
  }

  StringBuilder source;
  source.ReserveCapacity(length);

  size_t i = 0;
  for (; i < length && !previous_source_.IsEmpty(); ++i) {
    source.Append(previous_source_.CurrentChar());
    previous_source_.Advance();
  }
  for (; i < length; ++i) {
    DCHECK(!current_source_.IsEmpty());
    source.Append(current_source_.CurrentChar());
    current_source_.Advance();
  }

  cached_source_for_token_ = source.ToString();
  return cached_source_for_token_;
}

bool HTMLSourceTracker::NeedToCheckTokenizerBuffer(HTMLTokenizer* tokenizer) {
  HTMLTokenizer::State state = tokenizer->GetState();
  // The temporary buffer must not be used unconditionally, because in some
  // states (e.g. ScriptDataDoubleEscapedStartState), data is appended to
  // both the temporary buffer and the token itself.
  return state == HTMLTokenizer::kDataState ||
         HTMLTokenizer::IsEndTagBufferingState(state);
}

}  // namespace blink
