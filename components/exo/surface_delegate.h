// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXO_SURFACE_DELEGATE_H_
#define COMPONENTS_EXO_SURFACE_DELEGATE_H_

namespace exo {

// Frame types that can be used to decorate a surface.
enum class SurfaceFrameType { NONE, NORMAL, SHADOW };

// Handles events on surfaces in context-specific ways.
class SurfaceDelegate {
 public:
  // Called when surface was requested to commit all double-buffered state.
  virtual void OnSurfaceCommit() = 0;

  // Called when surface content size was changed.
  virtual void OnSurfaceContentSizeChanged() = 0;

  // Returns true if surface is in synchronized mode. ie. commit of
  // double-buffered state should be synchronized with parent surface.
  virtual bool IsSurfaceSynchronized() const = 0;

  // Called when surface was requested specific frame type.
  virtual void OnSetFrame(SurfaceFrameType type) = 0;

 protected:
  virtual ~SurfaceDelegate() {}
};

}  // namespace exo

#endif  // COMPONENTS_EXO_SURFACE_DELEGATE_H_
