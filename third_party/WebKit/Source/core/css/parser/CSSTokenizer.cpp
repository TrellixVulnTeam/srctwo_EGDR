// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/parser/CSSTokenizer.h"

namespace blink {
#include "core/CSSTokenizerCodepoints.cpp"
}

#include "core/css/parser/CSSParserIdioms.h"
#include "core/css/parser/CSSParserObserverWrapper.h"
#include "core/css/parser/CSSParserTokenRange.h"
#include "core/html/parser/HTMLParserIdioms.h"
#include "platform/wtf/text/CharacterNames.h"

namespace blink {

CSSTokenizer::CSSTokenizer(const String& string) : input_(string) {
  // According to the spec, we should perform preprocessing here.
  // See: http://dev.w3.org/csswg/css-syntax/#input-preprocessing
  //
  // However, we can skip this step since:
  // * We're using HTML spaces (which accept \r and \f as a valid white space)
  // * Do not count white spaces
  // * CSSTokenizerInputStream::NextInputChar() replaces NULLs for replacement
  //   characters

  if (string.IsEmpty())
    return;

  // To avoid resizing we err on the side of reserving too much space.
  // Most strings we tokenize have about 3.5 to 5 characters per token.
  tokens_.ReserveInitialCapacity(string.length() / 3);
}

CSSTokenizer::CSSTokenizer(const String& string,
                           CSSParserObserverWrapper& wrapper)
    : input_(string) {
  if (string.IsEmpty())
    return;

  // TODO(shend): Do not tokenize all in one go. We should be tokenizing on the
  // fly.
  unsigned offset = 0;
  while (true) {
    CSSParserToken token = NextToken();
    if (token.GetType() == kEOFToken)
      break;
    if (token.GetType() == kCommentToken) {
      wrapper.AddComment(offset, input_.Offset(), tokens_.size());
    } else {
      tokens_.push_back(token);
      wrapper.AddToken(offset);
    }
    offset = input_.Offset();
  }

  wrapper.AddToken(offset);
  wrapper.FinalizeConstruction(tokens_.begin());
}

CSSParserToken CSSTokenizer::TokenizeSingle() {
  while (true) {
    const CSSParserToken token = NextToken();
    if (token.GetType() == kCommentToken)
      continue;
    if (!token.IsEOF())
      tokens_.push_back(token);
    return token;
  }
}

void CSSTokenizer::EnsureTokenizedToEOF() {
  while (!TokenizeSingle().IsEOF()) {
  }
}

CSSParserTokenRange CSSTokenizer::TokenRange() {
  EnsureTokenizedToEOF();
  return tokens_;
}

unsigned CSSTokenizer::CurrentSize() const {
  return tokens_.size();
}

unsigned CSSTokenizer::TokenCount() {
  EnsureTokenizedToEOF();
  return tokens_.size();
}

static bool IsNewLine(UChar cc) {
  // We check \r and \f here, since we have no preprocessing stage
  return (cc == '\r' || cc == '\n' || cc == '\f');
}

// http://dev.w3.org/csswg/css-syntax/#check-if-two-code-points-are-a-valid-escape
static bool TwoCharsAreValidEscape(UChar first, UChar second) {
  return first == '\\' && !IsNewLine(second);
}

void CSSTokenizer::Reconsume(UChar c) {
  input_.PushBack(c);
}

UChar CSSTokenizer::Consume() {
  UChar current = input_.NextInputChar();
  input_.Advance();
  return current;
}

CSSParserToken CSSTokenizer::WhiteSpace(UChar cc) {
  input_.AdvanceUntilNonWhitespace();
  return CSSParserToken(kWhitespaceToken);
}

CSSParserToken CSSTokenizer::BlockStart(CSSParserTokenType type) {
  block_stack_.push_back(type);
  return CSSParserToken(type, CSSParserToken::kBlockStart);
}

CSSParserToken CSSTokenizer::BlockStart(CSSParserTokenType block_type,
                                        CSSParserTokenType type,
                                        StringView name) {
  block_stack_.push_back(block_type);
  return CSSParserToken(type, name, CSSParserToken::kBlockStart);
}

CSSParserToken CSSTokenizer::BlockEnd(CSSParserTokenType type,
                                      CSSParserTokenType start_type) {
  if (!block_stack_.IsEmpty() && block_stack_.back() == start_type) {
    block_stack_.pop_back();
    return CSSParserToken(type, CSSParserToken::kBlockEnd);
  }
  return CSSParserToken(type);
}

CSSParserToken CSSTokenizer::LeftParenthesis(UChar cc) {
  return BlockStart(kLeftParenthesisToken);
}

CSSParserToken CSSTokenizer::RightParenthesis(UChar cc) {
  return BlockEnd(kRightParenthesisToken, kLeftParenthesisToken);
}

CSSParserToken CSSTokenizer::LeftBracket(UChar cc) {
  return BlockStart(kLeftBracketToken);
}

CSSParserToken CSSTokenizer::RightBracket(UChar cc) {
  return BlockEnd(kRightBracketToken, kLeftBracketToken);
}

CSSParserToken CSSTokenizer::LeftBrace(UChar cc) {
  return BlockStart(kLeftBraceToken);
}

CSSParserToken CSSTokenizer::RightBrace(UChar cc) {
  return BlockEnd(kRightBraceToken, kLeftBraceToken);
}

CSSParserToken CSSTokenizer::PlusOrFullStop(UChar cc) {
  if (NextCharsAreNumber(cc)) {
    Reconsume(cc);
    return ConsumeNumericToken();
  }
  return CSSParserToken(kDelimiterToken, cc);
}

CSSParserToken CSSTokenizer::Asterisk(UChar cc) {
  DCHECK_EQ(cc, '*');
  if (ConsumeIfNext('='))
    return CSSParserToken(kSubstringMatchToken);
  return CSSParserToken(kDelimiterToken, '*');
}

CSSParserToken CSSTokenizer::LessThan(UChar cc) {
  DCHECK_EQ(cc, '<');
  if (input_.PeekWithoutReplacement(0) == '!' &&
      input_.PeekWithoutReplacement(1) == '-' &&
      input_.PeekWithoutReplacement(2) == '-') {
    input_.Advance(3);
    return CSSParserToken(kCDOToken);
  }
  return CSSParserToken(kDelimiterToken, '<');
}

CSSParserToken CSSTokenizer::Comma(UChar cc) {
  return CSSParserToken(kCommaToken);
}

CSSParserToken CSSTokenizer::HyphenMinus(UChar cc) {
  if (NextCharsAreNumber(cc)) {
    Reconsume(cc);
    return ConsumeNumericToken();
  }
  if (input_.PeekWithoutReplacement(0) == '-' &&
      input_.PeekWithoutReplacement(1) == '>') {
    input_.Advance(2);
    return CSSParserToken(kCDCToken);
  }
  if (NextCharsAreIdentifier(cc)) {
    Reconsume(cc);
    return ConsumeIdentLikeToken();
  }
  return CSSParserToken(kDelimiterToken, cc);
}

CSSParserToken CSSTokenizer::Solidus(UChar cc) {
  if (ConsumeIfNext('*')) {
    // These get ignored, but we need a value to return.
    ConsumeUntilCommentEndFound();
    return CSSParserToken(kCommentToken);
  }

  return CSSParserToken(kDelimiterToken, cc);
}

CSSParserToken CSSTokenizer::Colon(UChar cc) {
  return CSSParserToken(kColonToken);
}

CSSParserToken CSSTokenizer::SemiColon(UChar cc) {
  return CSSParserToken(kSemicolonToken);
}

CSSParserToken CSSTokenizer::Hash(UChar cc) {
  UChar next_char = input_.PeekWithoutReplacement(0);
  if (IsNameCodePoint(next_char) ||
      TwoCharsAreValidEscape(next_char, input_.PeekWithoutReplacement(1))) {
    HashTokenType type =
        NextCharsAreIdentifier() ? kHashTokenId : kHashTokenUnrestricted;
    return CSSParserToken(type, ConsumeName());
  }

  return CSSParserToken(kDelimiterToken, cc);
}

CSSParserToken CSSTokenizer::CircumflexAccent(UChar cc) {
  DCHECK_EQ(cc, '^');
  if (ConsumeIfNext('='))
    return CSSParserToken(kPrefixMatchToken);
  return CSSParserToken(kDelimiterToken, '^');
}

CSSParserToken CSSTokenizer::DollarSign(UChar cc) {
  DCHECK_EQ(cc, '$');
  if (ConsumeIfNext('='))
    return CSSParserToken(kSuffixMatchToken);
  return CSSParserToken(kDelimiterToken, '$');
}

CSSParserToken CSSTokenizer::VerticalLine(UChar cc) {
  DCHECK_EQ(cc, '|');
  if (ConsumeIfNext('='))
    return CSSParserToken(kDashMatchToken);
  if (ConsumeIfNext('|'))
    return CSSParserToken(kColumnToken);
  return CSSParserToken(kDelimiterToken, '|');
}

CSSParserToken CSSTokenizer::Tilde(UChar cc) {
  DCHECK_EQ(cc, '~');
  if (ConsumeIfNext('='))
    return CSSParserToken(kIncludeMatchToken);
  return CSSParserToken(kDelimiterToken, '~');
}

CSSParserToken CSSTokenizer::CommercialAt(UChar cc) {
  DCHECK_EQ(cc, '@');
  if (NextCharsAreIdentifier())
    return CSSParserToken(kAtKeywordToken, ConsumeName());
  return CSSParserToken(kDelimiterToken, '@');
}

CSSParserToken CSSTokenizer::ReverseSolidus(UChar cc) {
  if (TwoCharsAreValidEscape(cc, input_.PeekWithoutReplacement(0))) {
    Reconsume(cc);
    return ConsumeIdentLikeToken();
  }
  return CSSParserToken(kDelimiterToken, cc);
}

CSSParserToken CSSTokenizer::AsciiDigit(UChar cc) {
  Reconsume(cc);
  return ConsumeNumericToken();
}

CSSParserToken CSSTokenizer::LetterU(UChar cc) {
  if (input_.PeekWithoutReplacement(0) == '+' &&
      (IsASCIIHexDigit(input_.PeekWithoutReplacement(1)) ||
       input_.PeekWithoutReplacement(1) == '?')) {
    input_.Advance();
    return ConsumeUnicodeRange();
  }
  Reconsume(cc);
  return ConsumeIdentLikeToken();
}

CSSParserToken CSSTokenizer::NameStart(UChar cc) {
  Reconsume(cc);
  return ConsumeIdentLikeToken();
}

CSSParserToken CSSTokenizer::StringStart(UChar cc) {
  return ConsumeStringTokenUntil(cc);
}

CSSParserToken CSSTokenizer::EndOfFile(UChar cc) {
  return CSSParserToken(kEOFToken);
}

CSSParserToken CSSTokenizer::NextToken() {
  // Unlike the HTMLTokenizer, the CSS Syntax spec is written
  // as a stateless, (fixed-size) look-ahead tokenizer.
  // We could move to the stateful model and instead create
  // states for all the "next 3 codepoints are X" cases.
  // State-machine tokenizers are easier to write to handle
  // incremental tokenization of partial sources.
  // However, for now we follow the spec exactly.
  UChar cc = Consume();
  CodePoint code_point_func = 0;

  if (IsASCII(cc)) {
    SECURITY_DCHECK(cc < codePointsNumber);
    code_point_func = kCodePoints[cc];
  } else {
    code_point_func = &CSSTokenizer::NameStart;
  }

  if (code_point_func)
    return ((this)->*(code_point_func))(cc);
  return CSSParserToken(kDelimiterToken, cc);
}

// This method merges the following spec sections for efficiency
// http://www.w3.org/TR/css3-syntax/#consume-a-number
// http://www.w3.org/TR/css3-syntax/#convert-a-string-to-a-number
CSSParserToken CSSTokenizer::ConsumeNumber() {
  DCHECK(NextCharsAreNumber());

  NumericValueType type = kIntegerValueType;
  NumericSign sign = kNoSign;
  unsigned number_length = 0;

  UChar next = input_.PeekWithoutReplacement(0);
  if (next == '+') {
    ++number_length;
    sign = kPlusSign;
  } else if (next == '-') {
    ++number_length;
    sign = kMinusSign;
  }

  number_length = input_.SkipWhilePredicate<IsASCIIDigit>(number_length);
  next = input_.PeekWithoutReplacement(number_length);
  if (next == '.' &&
      IsASCIIDigit(input_.PeekWithoutReplacement(number_length + 1))) {
    type = kNumberValueType;
    number_length = input_.SkipWhilePredicate<IsASCIIDigit>(number_length + 2);
    next = input_.PeekWithoutReplacement(number_length);
  }

  if (next == 'E' || next == 'e') {
    next = input_.PeekWithoutReplacement(number_length + 1);
    if (IsASCIIDigit(next)) {
      type = kNumberValueType;
      number_length =
          input_.SkipWhilePredicate<IsASCIIDigit>(number_length + 1);
    } else if ((next == '+' || next == '-') &&
               IsASCIIDigit(input_.PeekWithoutReplacement(number_length + 2))) {
      type = kNumberValueType;
      number_length =
          input_.SkipWhilePredicate<IsASCIIDigit>(number_length + 3);
    }
  }

  double value = input_.GetDouble(0, number_length);
  input_.Advance(number_length);

  return CSSParserToken(kNumberToken, value, type, sign);
}

// http://www.w3.org/TR/css3-syntax/#consume-a-numeric-token
CSSParserToken CSSTokenizer::ConsumeNumericToken() {
  CSSParserToken token = ConsumeNumber();
  if (NextCharsAreIdentifier())
    token.ConvertToDimensionWithUnit(ConsumeName());
  else if (ConsumeIfNext('%'))
    token.ConvertToPercentage();
  return token;
}

// http://dev.w3.org/csswg/css-syntax/#consume-ident-like-token
CSSParserToken CSSTokenizer::ConsumeIdentLikeToken() {
  StringView name = ConsumeName();
  if (ConsumeIfNext('(')) {
    if (EqualIgnoringASCIICase(name, "url")) {
      // The spec is slightly different so as to avoid dropping whitespace
      // tokens, but they wouldn't be used and this is easier.
      input_.AdvanceUntilNonWhitespace();
      UChar next = input_.PeekWithoutReplacement(0);
      if (next != '"' && next != '\'')
        return ConsumeUrlToken();
    }
    return BlockStart(kLeftParenthesisToken, kFunctionToken, name);
  }
  return CSSParserToken(kIdentToken, name);
}

// http://dev.w3.org/csswg/css-syntax/#consume-a-string-token
CSSParserToken CSSTokenizer::ConsumeStringTokenUntil(UChar ending_code_point) {
  // Strings without escapes get handled without allocations
  for (unsigned size = 0;; size++) {
    UChar cc = input_.PeekWithoutReplacement(size);
    if (cc == ending_code_point) {
      unsigned start_offset = input_.Offset();
      input_.Advance(size + 1);
      return CSSParserToken(kStringToken, input_.RangeAt(start_offset, size));
    }
    if (IsNewLine(cc)) {
      input_.Advance(size);
      return CSSParserToken(kBadStringToken);
    }
    if (cc == '\0' || cc == '\\')
      break;
  }

  StringBuilder output;
  while (true) {
    UChar cc = Consume();
    if (cc == ending_code_point || cc == kEndOfFileMarker)
      return CSSParserToken(kStringToken, RegisterString(output.ToString()));
    if (IsNewLine(cc)) {
      Reconsume(cc);
      return CSSParserToken(kBadStringToken);
    }
    if (cc == '\\') {
      if (input_.NextInputChar() == kEndOfFileMarker)
        continue;
      if (IsNewLine(input_.PeekWithoutReplacement(0)))
        ConsumeSingleWhitespaceIfNext();  // This handles \r\n for us
      else
        output.Append(ConsumeEscape());
    } else {
      output.Append(cc);
    }
  }
}

CSSParserToken CSSTokenizer::ConsumeUnicodeRange() {
  DCHECK(IsASCIIHexDigit(input_.PeekWithoutReplacement(0)) ||
         input_.PeekWithoutReplacement(0) == '?');
  int length_remaining = 6;
  UChar32 start = 0;

  while (length_remaining &&
         IsASCIIHexDigit(input_.PeekWithoutReplacement(0))) {
    start = start * 16 + ToASCIIHexValue(Consume());
    --length_remaining;
  }

  UChar32 end = start;
  if (length_remaining && ConsumeIfNext('?')) {
    do {
      start *= 16;
      end = end * 16 + 0xF;
      --length_remaining;
    } while (length_remaining && ConsumeIfNext('?'));
  } else if (input_.PeekWithoutReplacement(0) == '-' &&
             IsASCIIHexDigit(input_.PeekWithoutReplacement(1))) {
    input_.Advance();
    length_remaining = 6;
    end = 0;
    do {
      end = end * 16 + ToASCIIHexValue(Consume());
      --length_remaining;
    } while (length_remaining &&
             IsASCIIHexDigit(input_.PeekWithoutReplacement(0)));
  }

  return CSSParserToken(kUnicodeRangeToken, start, end);
}

// http://dev.w3.org/csswg/css-syntax/#non-printable-code-point
static bool IsNonPrintableCodePoint(UChar cc) {
  return (cc >= '\0' && cc <= '\x8') || cc == '\xb' ||
         (cc >= '\xe' && cc <= '\x1f') || cc == '\x7f';
}

// http://dev.w3.org/csswg/css-syntax/#consume-url-token
CSSParserToken CSSTokenizer::ConsumeUrlToken() {
  input_.AdvanceUntilNonWhitespace();

  // URL tokens without escapes get handled without allocations
  for (unsigned size = 0;; size++) {
    UChar cc = input_.PeekWithoutReplacement(size);
    if (cc == ')') {
      unsigned start_offset = input_.Offset();
      input_.Advance(size + 1);
      return CSSParserToken(kUrlToken, input_.RangeAt(start_offset, size));
    }
    if (cc <= ' ' || cc == '\\' || cc == '"' || cc == '\'' || cc == '(' ||
        cc == '\x7f')
      break;
  }

  StringBuilder result;
  while (true) {
    UChar cc = Consume();
    if (cc == ')' || cc == kEndOfFileMarker)
      return CSSParserToken(kUrlToken, RegisterString(result.ToString()));

    if (IsHTMLSpace(cc)) {
      input_.AdvanceUntilNonWhitespace();
      if (ConsumeIfNext(')') || input_.NextInputChar() == kEndOfFileMarker)
        return CSSParserToken(kUrlToken, RegisterString(result.ToString()));
      break;
    }

    if (cc == '"' || cc == '\'' || cc == '(' || IsNonPrintableCodePoint(cc))
      break;

    if (cc == '\\') {
      if (TwoCharsAreValidEscape(cc, input_.PeekWithoutReplacement(0))) {
        result.Append(ConsumeEscape());
        continue;
      }
      break;
    }

    result.Append(cc);
  }

  ConsumeBadUrlRemnants();
  return CSSParserToken(kBadUrlToken);
}

// http://dev.w3.org/csswg/css-syntax/#consume-the-remnants-of-a-bad-url
void CSSTokenizer::ConsumeBadUrlRemnants() {
  while (true) {
    UChar cc = Consume();
    if (cc == ')' || cc == kEndOfFileMarker)
      return;
    if (TwoCharsAreValidEscape(cc, input_.PeekWithoutReplacement(0)))
      ConsumeEscape();
  }
}

void CSSTokenizer::ConsumeSingleWhitespaceIfNext() {
  // We check for \r\n and HTML spaces since we don't do preprocessing
  UChar next = input_.PeekWithoutReplacement(0);
  if (next == '\r' && input_.PeekWithoutReplacement(1) == '\n')
    input_.Advance(2);
  else if (IsHTMLSpace(next))
    input_.Advance();
}

void CSSTokenizer::ConsumeUntilCommentEndFound() {
  UChar c = Consume();
  while (true) {
    if (c == kEndOfFileMarker)
      return;
    if (c != '*') {
      c = Consume();
      continue;
    }
    c = Consume();
    if (c == '/')
      return;
  }
}

bool CSSTokenizer::ConsumeIfNext(UChar character) {
  // Since we're not doing replacement we can't tell the difference from
  // a NUL in the middle and the kEndOfFileMarker, so character must not be
  // NUL.
  DCHECK(character);
  if (input_.PeekWithoutReplacement(0) == character) {
    input_.Advance();
    return true;
  }
  return false;
}

// http://www.w3.org/TR/css3-syntax/#consume-a-name
StringView CSSTokenizer::ConsumeName() {
  // Names without escapes get handled without allocations
  for (unsigned size = 0;; ++size) {
    UChar cc = input_.PeekWithoutReplacement(size);
    if (IsNameCodePoint(cc))
      continue;
    // peekWithoutReplacement will return NUL when we hit the end of the
    // input. In that case we want to still use the rangeAt() fast path
    // below.
    if (cc == '\0' && input_.Offset() + size < input_.length())
      break;
    if (cc == '\\')
      break;
    unsigned start_offset = input_.Offset();
    input_.Advance(size);
    return input_.RangeAt(start_offset, size);
  }

  StringBuilder result;
  while (true) {
    UChar cc = Consume();
    if (IsNameCodePoint(cc)) {
      result.Append(cc);
      continue;
    }
    if (TwoCharsAreValidEscape(cc, input_.PeekWithoutReplacement(0))) {
      result.Append(ConsumeEscape());
      continue;
    }
    Reconsume(cc);
    return RegisterString(result.ToString());
  }
}

// http://dev.w3.org/csswg/css-syntax/#consume-an-escaped-code-point
UChar32 CSSTokenizer::ConsumeEscape() {
  UChar cc = Consume();
  DCHECK(!IsNewLine(cc));
  if (IsASCIIHexDigit(cc)) {
    unsigned consumed_hex_digits = 1;
    StringBuilder hex_chars;
    hex_chars.Append(cc);
    while (consumed_hex_digits < 6 &&
           IsASCIIHexDigit(input_.PeekWithoutReplacement(0))) {
      cc = Consume();
      hex_chars.Append(cc);
      consumed_hex_digits++;
    };
    ConsumeSingleWhitespaceIfNext();
    bool ok = false;
    UChar32 code_point = hex_chars.ToString().HexToUIntStrict(&ok);
    DCHECK(ok);
    if (code_point == 0 || (0xD800 <= code_point && code_point <= 0xDFFF) ||
        code_point > 0x10FFFF)
      return kReplacementCharacter;
    return code_point;
  }

  if (cc == kEndOfFileMarker)
    return kReplacementCharacter;
  return cc;
}

bool CSSTokenizer::NextTwoCharsAreValidEscape() {
  return TwoCharsAreValidEscape(input_.PeekWithoutReplacement(0),
                                input_.PeekWithoutReplacement(1));
}

// http://www.w3.org/TR/css3-syntax/#starts-with-a-number
bool CSSTokenizer::NextCharsAreNumber(UChar first) {
  UChar second = input_.PeekWithoutReplacement(0);
  if (IsASCIIDigit(first))
    return true;
  if (first == '+' || first == '-')
    return ((IsASCIIDigit(second)) ||
            (second == '.' && IsASCIIDigit(input_.PeekWithoutReplacement(1))));
  if (first == '.')
    return (IsASCIIDigit(second));
  return false;
}

bool CSSTokenizer::NextCharsAreNumber() {
  UChar first = Consume();
  bool are_number = NextCharsAreNumber(first);
  Reconsume(first);
  return are_number;
}

// http://dev.w3.org/csswg/css-syntax/#would-start-an-identifier
bool CSSTokenizer::NextCharsAreIdentifier(UChar first) {
  UChar second = input_.PeekWithoutReplacement(0);
  if (IsNameStartCodePoint(first) || TwoCharsAreValidEscape(first, second))
    return true;

  if (first == '-')
    return IsNameStartCodePoint(second) || second == '-' ||
           NextTwoCharsAreValidEscape();

  return false;
}

bool CSSTokenizer::NextCharsAreIdentifier() {
  UChar first = Consume();
  bool are_identifier = NextCharsAreIdentifier(first);
  Reconsume(first);
  return are_identifier;
}

StringView CSSTokenizer::RegisterString(const String& string) {
  string_pool_.push_back(string);
  return string;
}

}  // namespace blink
