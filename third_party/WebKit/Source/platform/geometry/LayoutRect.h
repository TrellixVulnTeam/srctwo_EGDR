/*
 * Copyright (c) 2012, Google Inc. All rights reserved.
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

#ifndef LayoutRect_h
#define LayoutRect_h

#include <iosfwd>
#include "platform/geometry/IntRect.h"
#include "platform/geometry/LayoutPoint.h"
#include "platform/geometry/LayoutRectOutsets.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/Vector.h"

namespace blink {

class FloatRect;
class DoubleRect;

class PLATFORM_EXPORT LayoutRect {
  DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();

 public:
  LayoutRect() {}
  LayoutRect(const LayoutPoint& location, const LayoutSize& size)
      : location_(location), size_(size) {}
  LayoutRect(LayoutUnit x, LayoutUnit y, LayoutUnit width, LayoutUnit height)
      : location_(LayoutPoint(x, y)), size_(LayoutSize(width, height)) {}
  LayoutRect(int x, int y, int width, int height)
      : location_(LayoutPoint(x, y)), size_(LayoutSize(width, height)) {}
  LayoutRect(const FloatPoint& location, const FloatSize& size)
      : location_(location), size_(size) {}
  LayoutRect(const DoublePoint& location, const DoubleSize& size)
      : location_(location), size_(size) {}
  LayoutRect(const IntPoint& location, const IntSize& size)
      : location_(location), size_(size) {}
  explicit LayoutRect(const IntRect& rect)
      : location_(rect.Location()), size_(rect.Size()) {}

  explicit LayoutRect(
      const FloatRect&);  // don't do this implicitly since it's lossy
  explicit LayoutRect(
      const DoubleRect&);  // don't do this implicitly since it's lossy

  LayoutPoint Location() const { return location_; }
  LayoutSize Size() const { return size_; }

  IntPoint PixelSnappedLocation() const { return RoundedIntPoint(location_); }
  IntSize PixelSnappedSize() const {
    return IntSize(SnapSizeToPixel(size_.Width(), location_.X()),
                   SnapSizeToPixel(size_.Height(), location_.Y()));
  }

  void SetLocation(const LayoutPoint& location) { location_ = location; }
  void SetSize(const LayoutSize& size) { size_ = size; }

  ALWAYS_INLINE LayoutUnit X() const { return location_.X(); }
  ALWAYS_INLINE LayoutUnit Y() const { return location_.Y(); }
  ALWAYS_INLINE LayoutUnit MaxX() const { return X() + Width(); }
  ALWAYS_INLINE LayoutUnit MaxY() const { return Y() + Height(); }
  LayoutUnit Width() const { return size_.Width(); }
  LayoutUnit Height() const { return size_.Height(); }

  int PixelSnappedWidth() const { return SnapSizeToPixel(Width(), X()); }
  int PixelSnappedHeight() const { return SnapSizeToPixel(Height(), Y()); }

  void SetX(LayoutUnit x) { location_.SetX(x); }
  void SetY(LayoutUnit y) { location_.SetY(y); }
  void SetWidth(LayoutUnit width) { size_.SetWidth(width); }
  void SetHeight(LayoutUnit height) { size_.SetHeight(height); }

  ALWAYS_INLINE bool IsEmpty() const { return size_.IsEmpty(); }

  // NOTE: The result is rounded to integer values, and thus may be not the
  // exact center point.
  LayoutPoint Center() const {
    return LayoutPoint(X() + Width() / 2, Y() + Height() / 2);
  }

  void Move(const LayoutSize& size) { location_ += size; }
  void Move(const IntSize& size) {
    location_.Move(LayoutUnit(size.Width()), LayoutUnit(size.Height()));
  }
  void MoveBy(const LayoutPoint& offset) {
    location_.Move(offset.X(), offset.Y());
  }
  void MoveBy(const IntPoint& offset) {
    location_.Move(LayoutUnit(offset.X()), LayoutUnit(offset.Y()));
  }
  void Move(LayoutUnit dx, LayoutUnit dy) { location_.Move(dx, dy); }
  void Move(int dx, int dy) { location_.Move(LayoutUnit(dx), LayoutUnit(dy)); }

  void Expand(const LayoutSize& size) { size_ += size; }
  void Expand(const LayoutRectOutsets& box) {
    location_.Move(-box.Left(), -box.Top());
    size_.Expand(box.Left() + box.Right(), box.Top() + box.Bottom());
  }
  void Expand(LayoutUnit dw, LayoutUnit dh) { size_.Expand(dw, dh); }
  void ExpandEdges(LayoutUnit top,
                   LayoutUnit right,
                   LayoutUnit bottom,
                   LayoutUnit left) {
    location_.Move(-left, -top);
    size_.Expand(left + right, top + bottom);
  }
  void Contract(const LayoutSize& size) { size_ -= size; }
  void Contract(LayoutUnit dw, LayoutUnit dh) { size_.Expand(-dw, -dh); }
  void Contract(int dw, int dh) { size_.Expand(-dw, -dh); }
  void Contract(const LayoutRectOutsets& box) {
    location_.Move(box.Left(), box.Top());
    size_.Shrink(box.Left() + box.Right(), box.Top() + box.Bottom());
  }
  void ContractEdges(LayoutUnit top,
                     LayoutUnit right,
                     LayoutUnit bottom,
                     LayoutUnit left) {
    location_.Move(left, top);
    size_.Shrink(left + right, top + bottom);
  }

  void ShiftXEdgeTo(LayoutUnit edge) {
    LayoutUnit delta = edge - X();
    SetX(edge);
    SetWidth((Width() - delta).ClampNegativeToZero());
  }
  void ShiftMaxXEdgeTo(LayoutUnit edge) {
    LayoutUnit delta = edge - MaxX();
    SetWidth((Width() + delta).ClampNegativeToZero());
  }
  void ShiftYEdgeTo(LayoutUnit edge) {
    LayoutUnit delta = edge - Y();
    SetY(edge);
    SetHeight((Height() - delta).ClampNegativeToZero());
  }
  void ShiftMaxYEdgeTo(LayoutUnit edge) {
    LayoutUnit delta = edge - MaxY();
    SetHeight((Height() + delta).ClampNegativeToZero());
  }

  LayoutPoint MinXMinYCorner() const { return location_; }  // typically topLeft
  LayoutPoint MaxXMinYCorner() const {
    return LayoutPoint(location_.X() + size_.Width(), location_.Y());
  }  // typically topRight
  LayoutPoint MinXMaxYCorner() const {
    return LayoutPoint(location_.X(), location_.Y() + size_.Height());
  }  // typically bottomLeft
  LayoutPoint MaxXMaxYCorner() const {
    return LayoutPoint(location_.X() + size_.Width(),
                       location_.Y() + size_.Height());
  }  // typically bottomRight

  bool Intersects(const LayoutRect&) const;
  bool Contains(const LayoutRect&) const;

  // This checks to see if the rect contains x,y in the traditional sense.
  // Equivalent to checking if the rect contains a 1x1 rect below and to the
  // right of (px,py).
  bool Contains(LayoutUnit px, LayoutUnit py) const {
    return px >= X() && px < MaxX() && py >= Y() && py < MaxY();
  }
  bool Contains(const LayoutPoint& point) const {
    return Contains(point.X(), point.Y());
  }

  void Intersect(const LayoutRect&);
  void Unite(const LayoutRect&);
  void UniteIfNonZero(const LayoutRect&);

  // Set this rect to be the intersection of itself and the argument rect
  // using edge-inclusive geometry.  If the two rectangles overlap but the
  // overlap region is zero-area (either because one of the two rectangles
  // is zero-area, or because the rectangles overlap at an edge or a corner),
  // the result is the zero-area intersection.  The return value indicates
  // whether the two rectangle actually have an intersection, since checking
  // the result for isEmpty() is not conclusive.
  bool InclusiveIntersect(const LayoutRect&);

  // Besides non-empty rects, this method also unites empty rects (as points or
  // line segments).  For example, union of (100, 100, 0x0) and (200, 200, 50x0)
  // is (100, 100, 150x100).
  void UniteEvenIfEmpty(const LayoutRect&);

  void InflateX(LayoutUnit dx) {
    location_.SetX(location_.X() - dx);
    size_.SetWidth(size_.Width() + dx + dx);
  }
  void InflateY(LayoutUnit dy) {
    location_.SetY(location_.Y() - dy);
    size_.SetHeight(size_.Height() + dy + dy);
  }
  void Inflate(LayoutUnit d) {
    InflateX(d);
    InflateY(d);
  }
  void Inflate(int d) { Inflate(LayoutUnit(d)); }
  void Scale(float s);
  void Scale(float x_axis_scale, float y_axis_scale);

  LayoutRect TransposedRect() const {
    return LayoutRect(location_.TransposedPoint(), size_.TransposedSize());
  }

  static IntRect InfiniteIntRect() {
    // Due to saturated arithemetic this value is not the same as
    // LayoutRect(IntRect(INT_MIN/2, INT_MIN/2, INT_MAX, INT_MAX)).
    static IntRect infinite_int_rect(
        LayoutRect(LayoutUnit::NearlyMin() / 2, LayoutUnit::NearlyMin() / 2,
                   LayoutUnit::NearlyMax(), LayoutUnit::NearlyMax()));
    return infinite_int_rect;
  }

  String ToString() const;

 private:
  LayoutPoint location_;
  LayoutSize size_;
};

inline LayoutRect Intersection(const LayoutRect& a, const LayoutRect& b) {
  LayoutRect c = a;
  c.Intersect(b);
  return c;
}

inline LayoutRect UnionRect(const LayoutRect& a, const LayoutRect& b) {
  LayoutRect c = a;
  c.Unite(b);
  return c;
}

PLATFORM_EXPORT LayoutRect UnionRect(const Vector<LayoutRect>&);

inline LayoutRect UnionRectEvenIfEmpty(const LayoutRect& a,
                                       const LayoutRect& b) {
  LayoutRect c = a;
  c.UniteEvenIfEmpty(b);
  return c;
}

PLATFORM_EXPORT LayoutRect UnionRectEvenIfEmpty(const Vector<LayoutRect>&);

ALWAYS_INLINE bool operator==(const LayoutRect& a, const LayoutRect& b) {
  return a.Location() == b.Location() && a.Size() == b.Size();
}

inline bool operator!=(const LayoutRect& a, const LayoutRect& b) {
  return a.Location() != b.Location() || a.Size() != b.Size();
}

inline IntRect PixelSnappedIntRect(const LayoutRect& rect) {
  return IntRect(RoundedIntPoint(rect.Location()),
                 IntSize(SnapSizeToPixel(rect.Width(), rect.X()),
                         SnapSizeToPixel(rect.Height(), rect.Y())));
}

inline IntRect EnclosingIntRect(const LayoutRect& rect) {
  IntPoint location = FlooredIntPoint(rect.MinXMinYCorner());
  IntPoint max_point = CeiledIntPoint(rect.MaxXMaxYCorner());
  return IntRect(location, max_point - location);
}

PLATFORM_EXPORT LayoutRect EnclosingLayoutRect(const FloatRect&);

inline IntRect PixelSnappedIntRect(LayoutUnit left,
                                   LayoutUnit top,
                                   LayoutUnit width,
                                   LayoutUnit height) {
  return IntRect(left.Round(), top.Round(), SnapSizeToPixel(width, left),
                 SnapSizeToPixel(height, top));
}

inline IntRect PixelSnappedIntRectFromEdges(LayoutUnit left,
                                            LayoutUnit top,
                                            LayoutUnit right,
                                            LayoutUnit bottom) {
  return IntRect(left.Round(), top.Round(), SnapSizeToPixel(right - left, left),
                 SnapSizeToPixel(bottom - top, top));
}

inline IntRect PixelSnappedIntRect(LayoutPoint location, LayoutSize size) {
  return IntRect(RoundedIntPoint(location),
                 PixelSnappedIntSize(size, location));
}

// Redeclared here to avoid ODR issues.
// See platform/testing/GeometryPrinters.h.
void PrintTo(const LayoutRect&, std::ostream*);

}  // namespace blink

#endif  // LayoutRect_h
