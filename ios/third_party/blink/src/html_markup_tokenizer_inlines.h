/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 * Copyright (C) 2009 Torch Mobile, Inc. http://www.torchmobile.com/
 * Copyright (C) 2010 Google, Inc. All Rights Reserved.
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

#ifndef MarkupTokenizerInlines_h
#define MarkupTokenizerInlines_h

#include "html_character_provider.h"

namespace WebCore {

template <typename CharType>
inline bool isTokenizerWhitespace(CharType cc)
{
    return cc == ' ' || cc == '\x0A' || cc == '\x09' || cc == '\x0C';
}

void advanceStringAndASSERTIgnoringCase(CharacterProvider& source, const LChar* expectedCharacters)
{
    while (*expectedCharacters) {
#ifndef NDEBUG
        ASSERT(isASCIILower(*expectedCharacters));

        UChar currentCharacter = source.currentCharacter();
        if (isASCIIUpper(currentCharacter))
            currentCharacter = toLowerCase(currentCharacter);

        ASSERT(currentCharacter == *expectedCharacters);
#endif

        ++expectedCharacters;
        source.next();
    }
}

inline void advanceAndASSERT(CharacterProvider& source, UChar expectedCharacter)
{
    ASSERT(source.currentCharacter() == expectedCharacter);
    source.next();
}

#define BEGIN_STATE(prefix, stateName) case prefix::stateName: stateName:
#define END_STATE() ASSERT_NOT_REACHED(); break;

// We use this macro when the HTML5 spec says "reconsume the current input
// character in the <mumble> state."
#define RECONSUME_IN(prefix, stateName)                                    \
    do {                                                                   \
        m_state = prefix::stateName;                                       \
        goto stateName;                                                    \
    } while (false)

// We use this macro when the HTML5 spec says "consume the next input
// character ... and switch to the <mumble> state."
#define ADVANCE_TO(prefix, stateName)                                      \
    do {                                                                   \
        m_state = prefix::stateName;                                       \
        if (!m_inputStreamPreprocessor.advance(source))                    \
            return haveBufferedCharacterToken();                           \
        cc = m_inputStreamPreprocessor.nextInputCharacter();               \
        goto stateName;                                                    \
    } while (false)

// Sometimes there's more complicated logic in the spec that separates when
// we consume the next input character and when we switch to a particular
// state. We handle those cases by advancing the source directly and using
// this macro to switch to the indicated state.
#define SWITCH_TO(prefix, stateName)                                       \
    do {                                                                   \
        m_state = prefix::stateName;                                       \
        if (source.isEmpty() || !m_inputStreamPreprocessor.peek(source))   \
            return haveBufferedCharacterToken();                           \
        cc = m_inputStreamPreprocessor.nextInputCharacter();               \
        goto stateName;                                                    \
    } while (false)

}

#endif // MarkupTokenizerInlines_h
