/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef InspectorLayerTreeAgent_h
#define InspectorLayerTreeAgent_h

#include "core/CoreExport.h"
#include "core/inspector/InspectorBaseAgent.h"
#include "core/inspector/protocol/LayerTree.h"
#include "core/page/PageOverlay.h"
#include "platform/Timer.h"
#include "platform/wtf/Noncopyable.h"
#include "platform/wtf/RefPtr.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class GraphicsContext;
class GraphicsLayer;
class InspectedFrames;
class LayoutRect;
class PictureSnapshot;
class PaintLayer;
class PaintLayerCompositor;

class CORE_EXPORT InspectorLayerTreeAgent final
    : public InspectorBaseAgent<protocol::LayerTree::Metainfo> {
  WTF_MAKE_NONCOPYABLE(InspectorLayerTreeAgent);

 public:
  class Client {
   public:
    virtual ~Client() {}
    virtual bool IsInspectorLayer(GraphicsLayer*) = 0;
  };

  static InspectorLayerTreeAgent* Create(InspectedFrames* inspected_frames,
                                         Client* client) {
    return new InspectorLayerTreeAgent(inspected_frames, client);
  }

  ~InspectorLayerTreeAgent() override;
  DECLARE_VIRTUAL_TRACE();

  void Restore() override;

  // Called from InspectorInstrumentation
  void LayerTreeDidChange();
  void DidPaint(const GraphicsLayer*, GraphicsContext&, const LayoutRect&);

  // Called from the front-end.
  protocol::Response enable() override;
  protocol::Response disable() override;
  protocol::Response compositingReasons(
      const String& layer_id,
      std::unique_ptr<protocol::Array<String>>* compositing_reasons) override;
  protocol::Response makeSnapshot(const String& layer_id,
                                  String* snapshot_id) override;
  protocol::Response loadSnapshot(
      std::unique_ptr<protocol::Array<protocol::LayerTree::PictureTile>> tiles,
      String* snapshot_id) override;
  protocol::Response releaseSnapshot(const String& snapshot_id) override;
  protocol::Response profileSnapshot(
      const String& snapshot_id,
      protocol::Maybe<int> min_repeat_count,
      protocol::Maybe<double> min_duration,
      protocol::Maybe<protocol::DOM::Rect> clip_rect,
      std::unique_ptr<protocol::Array<protocol::Array<double>>>* timings)
      override;
  protocol::Response replaySnapshot(const String& snapshot_id,
                                    protocol::Maybe<int> from_step,
                                    protocol::Maybe<int> to_step,
                                    protocol::Maybe<double> scale,
                                    String* data_url) override;
  protocol::Response snapshotCommandLog(
      const String& snapshot_id,
      std::unique_ptr<protocol::Array<protocol::DictionaryValue>>* command_log)
      override;

  // Called by other agents.
  std::unique_ptr<protocol::Array<protocol::LayerTree::Layer>> BuildLayerTree();

 private:
  static unsigned last_snapshot_id_;

  InspectorLayerTreeAgent(InspectedFrames*, Client*);

  GraphicsLayer* RootGraphicsLayer();

  PaintLayerCompositor* GetPaintLayerCompositor();
  protocol::Response LayerById(const String& layer_id, GraphicsLayer*&);
  protocol::Response GetSnapshotById(const String& snapshot_id,
                                     const PictureSnapshot*&);

  typedef HashMap<int, int> LayerIdToNodeIdMap;
  void BuildLayerIdToNodeIdMap(PaintLayer*, LayerIdToNodeIdMap&);
  void GatherGraphicsLayers(
      GraphicsLayer*,
      HashMap<int, int>& layer_id_to_node_id_map,
      std::unique_ptr<protocol::Array<protocol::LayerTree::Layer>>&,
      bool has_wheel_event_handlers,
      int scrolling_root_layer_id);
  int IdForNode(Node*);

  Member<InspectedFrames> inspected_frames_;
  Client* client_;

  typedef HashMap<String, RefPtr<PictureSnapshot>> SnapshotById;
  SnapshotById snapshot_by_id_;
  bool suppress_layer_paint_events_;
};

}  // namespace blink

#endif  // !defined(InspectorLayerTreeAgent_h)
