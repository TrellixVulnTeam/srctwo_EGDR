/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#include "core/svg/SVGPathByteStreamSource.h"

namespace blink {

PathSegmentData SVGPathByteStreamSource::ParseSegment() {
  DCHECK(HasMoreData());
  PathSegmentData segment;
  segment.command = static_cast<SVGPathSegType>(ReadSVGSegmentType());

  switch (segment.command) {
    case kPathSegCurveToCubicRel:
    case kPathSegCurveToCubicAbs:
      segment.point1 = ReadFloatPoint();
    /* fall through */
    case kPathSegCurveToCubicSmoothRel:
    case kPathSegCurveToCubicSmoothAbs:
      segment.point2 = ReadFloatPoint();
    /* fall through */
    case kPathSegMoveToRel:
    case kPathSegMoveToAbs:
    case kPathSegLineToRel:
    case kPathSegLineToAbs:
    case kPathSegCurveToQuadraticSmoothRel:
    case kPathSegCurveToQuadraticSmoothAbs:
      segment.target_point = ReadFloatPoint();
      break;
    case kPathSegLineToHorizontalRel:
    case kPathSegLineToHorizontalAbs:
      segment.target_point.SetX(ReadFloat());
      break;
    case kPathSegLineToVerticalRel:
    case kPathSegLineToVerticalAbs:
      segment.target_point.SetY(ReadFloat());
      break;
    case kPathSegClosePath:
      break;
    case kPathSegCurveToQuadraticRel:
    case kPathSegCurveToQuadraticAbs:
      segment.point1 = ReadFloatPoint();
      segment.target_point = ReadFloatPoint();
      break;
    case kPathSegArcRel:
    case kPathSegArcAbs: {
      segment.ArcRadii() = ReadFloatPoint();
      segment.SetArcAngle(ReadFloat());
      segment.arc_large = ReadFlag();
      segment.arc_sweep = ReadFlag();
      segment.target_point = ReadFloatPoint();
      break;
    }
    default:
      NOTREACHED();
  }
  return segment;
}

}  // namespace blink
