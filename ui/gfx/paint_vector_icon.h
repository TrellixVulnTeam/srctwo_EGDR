// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_PAINT_VECTOR_ICON_H_
#define UI_GFX_PAINT_VECTOR_ICON_H_

#include "base/time/time.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/gfx_export.h"
#include "ui/gfx/image/image_skia.h"

namespace gfx {

class Canvas;
struct VectorIcon;

// Describes an instance of an icon: an icon definition and a set of drawing
// parameters.
struct GFX_EXPORT IconDescription {
  IconDescription(const IconDescription& other);

  IconDescription(const VectorIcon& icon,
                  int dip_size,
                  SkColor color,
                  const base::TimeDelta& elapsed_time,
                  const VectorIcon& badge_icon);

  ~IconDescription();

  const VectorIcon& icon;
  int dip_size;
  SkColor color;
  const base::TimeDelta elapsed_time;
  const VectorIcon& badge_icon;
};

GFX_EXPORT extern const VectorIcon kNoneIcon;

// Draws a vector icon identified by |id| onto |canvas| at (0, 0). |color| is
// used as the fill. The size will come from the .icon file (the 1x version, if
// multiple versions exist). |elapsed_time| is used to determine the state of
// any transitions the icon may define.
GFX_EXPORT void PaintVectorIcon(
    Canvas* canvas,
    const VectorIcon& icon,
    SkColor color,
    const base::TimeDelta& elapsed_time = base::TimeDelta());

// As above, with a specificed size. |dip_size| is the length of a single edge
// of the square icon, in device independent pixels.
GFX_EXPORT void PaintVectorIcon(
    Canvas* canvas,
    const VectorIcon& icon,
    int dip_size,
    SkColor color,
    const base::TimeDelta& elapsed_time = base::TimeDelta());

// Creates an ImageSkia which will render the icon on demand.
// TODO(estade): update clients to use this version and remove the other
// CreateVectorIcon()s.
GFX_EXPORT ImageSkia CreateVectorIcon(const IconDescription& params);

// Creates an ImageSkia which will render the icon on demand. The size will come
// from the .icon file (the 1x version, if multiple versions exist).
GFX_EXPORT ImageSkia CreateVectorIcon(const VectorIcon& icon, SkColor color);

// As above, but creates the image at the given size.
GFX_EXPORT ImageSkia CreateVectorIcon(const VectorIcon& icon,
                                      int dip_size,
                                      SkColor color);

// As above, but also paints a badge defined by |badge_id| on top of the icon.
// The badge uses the same canvas size and default color as the icon.
GFX_EXPORT ImageSkia CreateVectorIconWithBadge(const VectorIcon& icon,
                                               int dip_size,
                                               SkColor color,
                                               const VectorIcon& badge_icon);

#if defined(GFX_VECTOR_ICONS_UNSAFE) || defined(GFX_IMPLEMENTATION)
// Takes a string of the format expected of .icon files and renders onto
// a canvas. This should only be used as a debugging aid and should never be
// used in production code.
GFX_EXPORT ImageSkia CreateVectorIconFromSource(const std::string& source,
                                                int dip_size,
                                                SkColor color);
#endif

// Calculates the size that will be default for |icon|, in dip.
GFX_EXPORT int GetDefaultSizeOfVectorIcon(const gfx::VectorIcon& icon);

// Calculates and returns the elapsed time at which all animations/transitions
// will be finished.
GFX_EXPORT base::TimeDelta GetDurationOfAnimation(const VectorIcon& icon);

}  // namespace gfx

#endif  // UI_GFX_PAINT_VECTOR_ICON_H_
