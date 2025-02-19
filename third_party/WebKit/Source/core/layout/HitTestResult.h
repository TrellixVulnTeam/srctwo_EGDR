/*
 * Copyright (C) 2006 Apple Computer, Inc.
 * Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
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
 *
*/

#ifndef HitTestResult_h
#define HitTestResult_h

#include "core/CoreExport.h"
#include "core/editing/PositionWithAffinity.h"
#include "core/layout/HitTestLocation.h"
#include "core/layout/HitTestRequest.h"
#include "platform/geometry/FloatQuad.h"
#include "platform/geometry/FloatRect.h"
#include "platform/heap/Handle.h"
#include "platform/text/TextDirection.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/ListHashSet.h"
#include "platform/wtf/VectorTraits.h"

namespace blink {

class Element;
class LocalFrame;
class HTMLAreaElement;
class HTMLMediaElement;
class Image;
class KURL;
class Node;
class LayoutObject;
class Region;
class Scrollbar;

// List-based hit test testing can continue even after a hit has been found.
// This is used to support fuzzy matching with rect-based hit tests as well as
// penetrating tests which collect all nodes (see: HitTestRequest::RequestType).
enum ListBasedHitTestBehavior { kContinueHitTesting, kStopHitTesting };

class CORE_EXPORT HitTestResult {
  DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();

 public:
  typedef HeapListHashSet<Member<Node>> NodeSet;

  HitTestResult();
  HitTestResult(const HitTestRequest&, const LayoutPoint&);
  // Pass positive padding values to perform a rect-based hit test.
  HitTestResult(const HitTestRequest&,
                const LayoutPoint& center_point,
                unsigned top_padding,
                unsigned right_padding,
                unsigned bottom_padding,
                unsigned left_padding);
  HitTestResult(const HitTestRequest&, const HitTestLocation&);
  HitTestResult(const HitTestResult&);
  ~HitTestResult();
  HitTestResult& operator=(const HitTestResult&);
  DECLARE_TRACE();

  bool EqualForCacheability(const HitTestResult&) const;
  void CacheValues(const HitTestResult&);

  // Populate this object based on another HitTestResult; similar to assignment
  // operator but don't assign any of the request parameters. ie. This method
  // avoids setting |m_hitTestLocation|, |m_hitTestRequest|.
  void PopulateFromCachedResult(const HitTestResult&);

  // For point-based hit tests, these accessors provide information about the
  // node under the point. For rect-based hit tests they are meaningless
  // (reflect the last candidate node observed in the rect).
  // FIXME: Make these less error-prone for rect-based hit tests (center point
  // or fail).
  Node* InnerNode() const { return inner_node_.Get(); }
  Node* InnerPossiblyPseudoNode() const {
    return inner_possibly_pseudo_node_.Get();
  }
  Element* InnerElement() const;

  // If innerNode is an image map or image map area, return the associated image
  // node.
  Node* InnerNodeOrImageMapImage() const;

  Element* URLElement() const { return inner_url_element_.Get(); }
  Scrollbar* GetScrollbar() const { return scrollbar_.Get(); }
  bool IsOverEmbeddedContentView() const {
    return is_over_embedded_content_view_;
  }

  // Forwarded from HitTestLocation
  bool IsRectBasedTest() const { return hit_test_location_.IsRectBasedTest(); }

  // The hit-tested point in the coordinates of the main frame.
  const LayoutPoint& PointInMainFrame() const {
    return hit_test_location_.Point();
  }
  IntPoint RoundedPointInMainFrame() const {
    return RoundedIntPoint(PointInMainFrame());
  }

  // The hit-tested point in the coordinates of the innerNode frame, the frame
  // containing innerNode.
  const LayoutPoint& PointInInnerNodeFrame() const {
    return point_in_inner_node_frame_;
  }
  IntPoint RoundedPointInInnerNodeFrame() const {
    return RoundedIntPoint(PointInInnerNodeFrame());
  }
  LocalFrame* InnerNodeFrame() const;

  // The hit-tested point in the coordinates of the inner node.
  const LayoutPoint& LocalPoint() const { return local_point_; }
  void SetNodeAndPosition(Node* node, const LayoutPoint& p) {
    local_point_ = p;
    SetInnerNode(node);
  }

  PositionWithAffinity GetPosition() const;
  LayoutObject* GetLayoutObject() const;

  void SetToShadowHostIfInRestrictedShadowRoot();

  const HitTestLocation& GetHitTestLocation() const {
    return hit_test_location_;
  }
  const HitTestRequest& GetHitTestRequest() const { return hit_test_request_; }

  void SetInnerNode(Node*);
  HTMLAreaElement* ImageAreaForImage() const;
  void SetURLElement(Element*);
  void SetScrollbar(Scrollbar*);
  void SetIsOverEmbeddedContentView(bool b) {
    is_over_embedded_content_view_ = b;
  }

  bool IsSelected() const;
  String Title(TextDirection&) const;
  const AtomicString& AltDisplayString() const;
  Image* GetImage() const;
  IntRect ImageRect() const;
  KURL AbsoluteImageURL() const;
  KURL AbsoluteMediaURL() const;
  KURL AbsoluteLinkURL() const;
  String TextContent() const;
  bool IsLiveLink() const;
  bool IsContentEditable() const;

  const String& CanvasRegionId() const { return canvas_region_id_; }
  void SetCanvasRegionId(const String& id) { canvas_region_id_ = id; }

  bool IsOverLink() const;

  bool IsCacheable() const { return cacheable_; }
  void SetCacheable(bool cacheable) { cacheable_ = cacheable; }

  // TODO(pdr): When using the default rect argument, this function does not
  // check if the tapped area is entirely contained by the HitTestLocation's
  // bounding box. Callers should pass a LayoutRect as the third parameter so
  // hit testing can early-out when a tapped area is covered.
  ListBasedHitTestBehavior AddNodeToListBasedTestResult(
      Node*,
      const HitTestLocation&,
      const LayoutRect& = LayoutRect());
  ListBasedHitTestBehavior AddNodeToListBasedTestResult(Node*,
                                                        const HitTestLocation&,
                                                        const Region&);

  void Append(const HitTestResult&);

  // If m_listBasedTestResult is 0 then set it to a new NodeSet. Return
  // *m_listBasedTestResult. Lazy allocation makes sense because the NodeSet is
  // seldom necessary, and it's somewhat expensive to allocate and initialize.
  // This method does the same thing as mutableListBasedTestResult(), but here
  // the return value is const.
  const NodeSet& ListBasedTestResult() const;

  // Collapse the rect-based test result into a single target at the specified
  // location.
  void ResolveRectBasedTest(Node* resolved_inner_node,
                            const LayoutPoint& resolved_point_in_main_frame);

 private:
  NodeSet& MutableListBasedTestResult();  // See above.
  HTMLMediaElement* MediaElement() const;

  HitTestLocation hit_test_location_;
  HitTestRequest hit_test_request_;
  bool cacheable_;

  Member<Node> inner_node_;
  Member<Node> inner_possibly_pseudo_node_;
  // FIXME: Nothing changes this to a value different from m_hitTestLocation!
  // The hit-tested point in innerNode frame coordinates.
  LayoutPoint point_in_inner_node_frame_;
  // A point in the local coordinate space of m_innerNode's layoutObject.Allows
  // us to efficiently determine where inside the layoutObject we hit on
  // subsequent operations.
  LayoutPoint local_point_;
  // For non-URL, this is the enclosing that triggers navigation.
  Member<Element> inner_url_element_;
  Member<Scrollbar> scrollbar_;
  // Returns true if we are over a EmbeddedContentView (and not in the
  // border/padding area of a LayoutEmbeddedContent for example).
  bool is_over_embedded_content_view_;

  mutable Member<NodeSet> list_based_test_result_;
  String canvas_region_id_;
};

}  // namespace blink

WTF_ALLOW_CLEAR_UNUSED_SLOTS_WITH_MEM_FUNCTIONS(blink::HitTestResult);

#endif  // HitTestResult_h
