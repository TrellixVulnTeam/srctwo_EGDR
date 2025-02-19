/*
 * Copyright (C) 2012 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FloatPolygon_h
#define FloatPolygon_h

#include <memory>
#include "platform/PODIntervalTree.h"
#include "platform/geometry/FloatPoint.h"
#include "platform/geometry/FloatRect.h"
#include "platform/graphics/GraphicsTypes.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/Vector.h"

namespace blink {

class FloatPolygonEdge;

// This class is used by PODIntervalTree for debugging.
#ifndef NDEBUG
template <class>
struct ValueToString;
#endif

class PLATFORM_EXPORT FloatPolygon {
  USING_FAST_MALLOC(FloatPolygon);
  WTF_MAKE_NONCOPYABLE(FloatPolygon);

 public:
  FloatPolygon(std::unique_ptr<Vector<FloatPoint>> vertices,
               WindRule fill_rule);

  const FloatPoint& VertexAt(unsigned index) const {
    return (*vertices_)[index];
  }
  unsigned NumberOfVertices() const { return vertices_->size(); }

  WindRule FillRule() const { return fill_rule_; }

  const FloatPolygonEdge& EdgeAt(unsigned index) const { return edges_[index]; }
  unsigned NumberOfEdges() const { return edges_.size(); }

  FloatRect BoundingBox() const { return bounding_box_; }
  bool OverlappingEdges(float min_y,
                        float max_y,
                        Vector<const FloatPolygonEdge*>& result) const;
  bool Contains(const FloatPoint&) const;
  bool IsEmpty() const { return empty_; }

 private:
  typedef PODInterval<float, FloatPolygonEdge*> EdgeInterval;
  typedef PODIntervalTree<float, FloatPolygonEdge*> EdgeIntervalTree;

  bool ContainsNonZero(const FloatPoint&) const;
  bool ContainsEvenOdd(const FloatPoint&) const;

  std::unique_ptr<Vector<FloatPoint>> vertices_;
  WindRule fill_rule_;
  FloatRect bounding_box_;
  bool empty_;
  Vector<FloatPolygonEdge> edges_;
  EdgeIntervalTree edge_tree_;  // Each EdgeIntervalTree node stores minY, maxY,
                                // and a ("UserData") pointer to a
                                // FloatPolygonEdge.
};

class PLATFORM_EXPORT VertexPair {
 public:
  virtual ~VertexPair() {}

  virtual const FloatPoint& Vertex1() const = 0;
  virtual const FloatPoint& Vertex2() const = 0;

  float MinX() const { return std::min(Vertex1().X(), Vertex2().X()); }
  float MinY() const { return std::min(Vertex1().Y(), Vertex2().Y()); }
  float MaxX() const { return std::max(Vertex1().X(), Vertex2().X()); }
  float MaxY() const { return std::max(Vertex1().Y(), Vertex2().Y()); }

  bool Intersection(const VertexPair&, FloatPoint&) const;
};

class PLATFORM_EXPORT FloatPolygonEdge final : public VertexPair {
  DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();
  friend class FloatPolygon;

 public:
  const FloatPoint& Vertex1() const override {
    DCHECK(polygon_);
    return polygon_->VertexAt(vertex_index1_);
  }

  const FloatPoint& Vertex2() const override {
    DCHECK(polygon_);
    return polygon_->VertexAt(vertex_index2_);
  }

  const FloatPolygonEdge& PreviousEdge() const {
    DCHECK(polygon_);
    DCHECK_GT(polygon_->NumberOfEdges(), 1UL);
    return polygon_->EdgeAt((edge_index_ + polygon_->NumberOfEdges() - 1) %
                            polygon_->NumberOfEdges());
  }

  const FloatPolygonEdge& NextEdge() const {
    DCHECK(polygon_);
    DCHECK_GT(polygon_->NumberOfEdges(), 1UL);
    return polygon_->EdgeAt((edge_index_ + 1) % polygon_->NumberOfEdges());
  }

  const FloatPolygon* Polygon() const { return polygon_; }
  unsigned VertexIndex1() const { return vertex_index1_; }
  unsigned VertexIndex2() const { return vertex_index2_; }
  unsigned EdgeIndex() const { return edge_index_; }

 private:
  // Edge vertex index1 is less than index2, except the last edge, where index2
  // is 0. When a polygon edge is defined by 3 or more colinear vertices, index2
  // can be the the index of the last colinear vertex.
  unsigned vertex_index1_;
  unsigned vertex_index2_;
  unsigned edge_index_;
  const FloatPolygon* polygon_;
};

// These structures are used by PODIntervalTree for debugging.
#ifndef NDEBUG
template <>
struct ValueToString<float> {
  STATIC_ONLY(ValueToString);
  static String ToString(const float value) { return String::Number(value); }
};

template <>
struct ValueToString<FloatPolygonEdge*> {
  STATIC_ONLY(ValueToString);
  static String ToString(const FloatPolygonEdge* edge) {
    return String::Format("%p (%f,%f %f,%f)", edge, edge->Vertex1().X(),
                          edge->Vertex1().Y(), edge->Vertex2().X(),
                          edge->Vertex2().Y());
  }
};
#endif

}  // namespace blink

#endif  // FloatPolygon_h
