/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "core/svg/SVGPathStringSource.h"

#include "core/svg/SVGParserUtilities.h"
#include "platform/geometry/FloatPoint.h"

namespace blink {

SVGPathStringSource::SVGPathStringSource(const String& string)
    : is8_bit_source_(string.Is8Bit()),
      previous_command_(kPathSegUnknown),
      string_(string) {
  DCHECK(!string.IsNull());

  if (is8_bit_source_) {
    current_.character8_ = string.Characters8();
    end_.character8_ = current_.character8_ + string.length();
  } else {
    current_.character16_ = string.Characters16();
    end_.character16_ = current_.character16_ + string.length();
  }
  EatWhitespace();
}

void SVGPathStringSource::EatWhitespace() {
  if (is8_bit_source_)
    SkipOptionalSVGSpaces(current_.character8_, end_.character8_);
  else
    SkipOptionalSVGSpaces(current_.character16_, end_.character16_);
}

static SVGPathSegType MapLetterToSegmentType(unsigned lookahead) {
  switch (lookahead) {
    case 'Z':
    case 'z':
      return kPathSegClosePath;
    case 'M':
      return kPathSegMoveToAbs;
    case 'm':
      return kPathSegMoveToRel;
    case 'L':
      return kPathSegLineToAbs;
    case 'l':
      return kPathSegLineToRel;
    case 'C':
      return kPathSegCurveToCubicAbs;
    case 'c':
      return kPathSegCurveToCubicRel;
    case 'Q':
      return kPathSegCurveToQuadraticAbs;
    case 'q':
      return kPathSegCurveToQuadraticRel;
    case 'A':
      return kPathSegArcAbs;
    case 'a':
      return kPathSegArcRel;
    case 'H':
      return kPathSegLineToHorizontalAbs;
    case 'h':
      return kPathSegLineToHorizontalRel;
    case 'V':
      return kPathSegLineToVerticalAbs;
    case 'v':
      return kPathSegLineToVerticalRel;
    case 'S':
      return kPathSegCurveToCubicSmoothAbs;
    case 's':
      return kPathSegCurveToCubicSmoothRel;
    case 'T':
      return kPathSegCurveToQuadraticSmoothAbs;
    case 't':
      return kPathSegCurveToQuadraticSmoothRel;
    default:
      return kPathSegUnknown;
  }
}

static bool IsNumberStart(unsigned lookahead) {
  return (lookahead >= '0' && lookahead <= '9') || lookahead == '+' ||
         lookahead == '-' || lookahead == '.';
}

static bool MaybeImplicitCommand(unsigned lookahead,
                                 SVGPathSegType previous_command,
                                 SVGPathSegType& next_command) {
  // Check if the current lookahead may start a number - in which case it
  // could be the start of an implicit command. The 'close' command does not
  // have any parameters though and hence can't have an implicit
  // 'continuation'.
  if (!IsNumberStart(lookahead) || previous_command == kPathSegClosePath)
    return false;
  // Implicit continuations of moveto command translate to linetos.
  if (previous_command == kPathSegMoveToAbs) {
    next_command = kPathSegLineToAbs;
    return true;
  }
  if (previous_command == kPathSegMoveToRel) {
    next_command = kPathSegLineToRel;
    return true;
  }
  next_command = previous_command;
  return true;
}

void SVGPathStringSource::SetErrorMark(SVGParseStatus status) {
  if (error_.Status() != SVGParseStatus::kNoError)
    return;
  size_t locus = is8_bit_source_
                     ? current_.character8_ - string_.Characters8()
                     : current_.character16_ - string_.Characters16();
  error_ = SVGParsingError(status, locus);
}

float SVGPathStringSource::ParseNumberWithError() {
  float number_value = 0;
  bool error;
  if (is8_bit_source_)
    error = !ParseNumber(current_.character8_, end_.character8_, number_value);
  else
    error =
        !ParseNumber(current_.character16_, end_.character16_, number_value);
  if (UNLIKELY(error))
    SetErrorMark(SVGParseStatus::kExpectedNumber);
  return number_value;
}

bool SVGPathStringSource::ParseArcFlagWithError() {
  bool flag_value = false;
  bool error;
  if (is8_bit_source_)
    error = !ParseArcFlag(current_.character8_, end_.character8_, flag_value);
  else
    error = !ParseArcFlag(current_.character16_, end_.character16_, flag_value);
  if (UNLIKELY(error))
    SetErrorMark(SVGParseStatus::kExpectedArcFlag);
  return flag_value;
}

PathSegmentData SVGPathStringSource::ParseSegment() {
  DCHECK(HasMoreData());
  PathSegmentData segment;
  unsigned lookahead =
      is8_bit_source_ ? *current_.character8_ : *current_.character16_;
  SVGPathSegType command = MapLetterToSegmentType(lookahead);
  if (UNLIKELY(previous_command_ == kPathSegUnknown)) {
    // First command has to be a moveto.
    if (command != kPathSegMoveToRel && command != kPathSegMoveToAbs) {
      SetErrorMark(SVGParseStatus::kExpectedMoveToCommand);
      return segment;
    }
    // Consume command letter.
    if (is8_bit_source_)
      current_.character8_++;
    else
      current_.character16_++;
  } else if (command == kPathSegUnknown) {
    // Possibly an implicit command.
    DCHECK_NE(previous_command_, kPathSegUnknown);
    if (!MaybeImplicitCommand(lookahead, previous_command_, command)) {
      SetErrorMark(SVGParseStatus::kExpectedPathCommand);
      return segment;
    }
  } else {
    // Valid explicit command.
    if (is8_bit_source_)
      current_.character8_++;
    else
      current_.character16_++;
  }

  segment.command = previous_command_ = command;

  DCHECK_EQ(error_.Status(), SVGParseStatus::kNoError);

  switch (segment.command) {
    case kPathSegCurveToCubicRel:
    case kPathSegCurveToCubicAbs:
      segment.point1.SetX(ParseNumberWithError());
      segment.point1.SetY(ParseNumberWithError());
    /* fall through */
    case kPathSegCurveToCubicSmoothRel:
    case kPathSegCurveToCubicSmoothAbs:
      segment.point2.SetX(ParseNumberWithError());
      segment.point2.SetY(ParseNumberWithError());
    /* fall through */
    case kPathSegMoveToRel:
    case kPathSegMoveToAbs:
    case kPathSegLineToRel:
    case kPathSegLineToAbs:
    case kPathSegCurveToQuadraticSmoothRel:
    case kPathSegCurveToQuadraticSmoothAbs:
      segment.target_point.SetX(ParseNumberWithError());
      segment.target_point.SetY(ParseNumberWithError());
      break;
    case kPathSegLineToHorizontalRel:
    case kPathSegLineToHorizontalAbs:
      segment.target_point.SetX(ParseNumberWithError());
      break;
    case kPathSegLineToVerticalRel:
    case kPathSegLineToVerticalAbs:
      segment.target_point.SetY(ParseNumberWithError());
      break;
    case kPathSegClosePath:
      EatWhitespace();
      break;
    case kPathSegCurveToQuadraticRel:
    case kPathSegCurveToQuadraticAbs:
      segment.point1.SetX(ParseNumberWithError());
      segment.point1.SetY(ParseNumberWithError());
      segment.target_point.SetX(ParseNumberWithError());
      segment.target_point.SetY(ParseNumberWithError());
      break;
    case kPathSegArcRel:
    case kPathSegArcAbs:
      segment.ArcRadii().SetX(ParseNumberWithError());
      segment.ArcRadii().SetY(ParseNumberWithError());
      segment.SetArcAngle(ParseNumberWithError());
      segment.arc_large = ParseArcFlagWithError();
      segment.arc_sweep = ParseArcFlagWithError();
      segment.target_point.SetX(ParseNumberWithError());
      segment.target_point.SetY(ParseNumberWithError());
      break;
    case kPathSegUnknown:
      NOTREACHED();
  }

  if (UNLIKELY(error_.Status() != SVGParseStatus::kNoError))
    segment.command = kPathSegUnknown;
  return segment;
}

}  // namespace blink
