/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2010 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "core/svg/graphics/filters/SVGFEImage.h"

#include "core/layout/LayoutObject.h"
#include "core/paint/SVGPaintContext.h"
#include "core/paint/TransformRecorder.h"
#include "core/svg/SVGElement.h"
#include "core/svg/SVGLengthContext.h"
#include "core/svg/SVGURIReference.h"
#include "platform/graphics/GraphicsContext.h"
#include "platform/graphics/filters/Filter.h"
#include "platform/graphics/filters/SkiaImageFilterBuilder.h"
#include "platform/graphics/paint/PaintRecord.h"
#include "platform/graphics/paint/PaintRecordBuilder.h"
#include "platform/text/TextStream.h"
#include "platform/transforms/AffineTransform.h"
#include "third_party/skia/include/effects/SkImageSource.h"
#include "third_party/skia/include/effects/SkPictureImageFilter.h"

namespace blink {

FEImage::FEImage(Filter* filter,
                 RefPtr<Image> image,
                 SVGPreserveAspectRatio* preserve_aspect_ratio)
    : FilterEffect(filter),
      image_(std::move(image)),
      tree_scope_(nullptr),
      preserve_aspect_ratio_(preserve_aspect_ratio) {
  FilterEffect::SetOperatingInterpolationSpace(kInterpolationSpaceSRGB);
}

FEImage::FEImage(Filter* filter,
                 TreeScope& tree_scope,
                 const String& href,
                 SVGPreserveAspectRatio* preserve_aspect_ratio)
    : FilterEffect(filter),
      tree_scope_(&tree_scope),
      href_(href),
      preserve_aspect_ratio_(preserve_aspect_ratio) {
  FilterEffect::SetOperatingInterpolationSpace(kInterpolationSpaceSRGB);
}

DEFINE_TRACE(FEImage) {
  visitor->Trace(tree_scope_);
  visitor->Trace(preserve_aspect_ratio_);
  FilterEffect::Trace(visitor);
}

FEImage* FEImage::CreateWithImage(
    Filter* filter,
    RefPtr<Image> image,
    SVGPreserveAspectRatio* preserve_aspect_ratio) {
  return new FEImage(filter, std::move(image), preserve_aspect_ratio);
}

FEImage* FEImage::CreateWithIRIReference(
    Filter* filter,
    TreeScope& tree_scope,
    const String& href,
    SVGPreserveAspectRatio* preserve_aspect_ratio) {
  return new FEImage(filter, tree_scope, href, preserve_aspect_ratio);
}

static FloatRect GetLayoutObjectRepaintRect(LayoutObject* layout_object) {
  return layout_object->LocalToSVGParentTransform().MapRect(
      layout_object->VisualRectInLocalSVGCoordinates());
}

AffineTransform MakeMapBetweenRects(const FloatRect& source,
                                    const FloatRect& dest) {
  AffineTransform transform;
  transform.Translate(dest.X() - source.X(), dest.Y() - source.Y());
  transform.Scale(dest.Width() / source.Width(),
                  dest.Height() / source.Height());
  return transform;
}

FloatRect FEImage::MapInputs(const FloatRect&) const {
  LayoutObject* layout_object = ReferencedLayoutObject();
  if (!image_ && !layout_object)
    return FloatRect();

  FloatRect dest_rect =
      GetFilter()->MapLocalRectToAbsoluteRect(FilterPrimitiveSubregion());
  FloatRect src_rect;
  if (layout_object) {
    src_rect = GetLayoutObjectRepaintRect(layout_object);
    SVGElement* context_node = ToSVGElement(layout_object->GetNode());

    if (context_node->HasRelativeLengths()) {
      // FIXME: This fixes relative lengths but breaks non-relative ones (see
      // crbug/260709).
      SVGLengthContext length_context(context_node);
      FloatSize viewport_size;
      if (length_context.DetermineViewport(viewport_size)) {
        src_rect = MakeMapBetweenRects(FloatRect(FloatPoint(), viewport_size),
                                       dest_rect)
                       .MapRect(src_rect);
      }
    } else {
      src_rect = GetFilter()->MapLocalRectToAbsoluteRect(src_rect);
      src_rect.Move(dest_rect.X(), dest_rect.Y());
    }
    dest_rect.Intersect(src_rect);
  } else {
    src_rect = FloatRect(FloatPoint(), FloatSize(image_->Size()));
    preserve_aspect_ratio_->TransformRect(dest_rect, src_rect);
  }
  return dest_rect;
}

LayoutObject* FEImage::ReferencedLayoutObject() const {
  if (!tree_scope_)
    return nullptr;
  Element* href_element =
      SVGURIReference::TargetElementFromIRIString(href_, *tree_scope_);
  if (!href_element || !href_element->IsSVGElement())
    return nullptr;
  return href_element->GetLayoutObject();
}

TextStream& FEImage::ExternalRepresentation(TextStream& ts, int indent) const {
  IntSize image_size;
  if (image_) {
    image_size = image_->Size();
  } else if (LayoutObject* layout_object = ReferencedLayoutObject()) {
    image_size =
        EnclosingIntRect(GetLayoutObjectRepaintRect(layout_object)).Size();
  }
  WriteIndent(ts, indent);
  ts << "[feImage";
  FilterEffect::ExternalRepresentation(ts);
  ts << " image-size=\"" << image_size.Width() << "x" << image_size.Height()
     << "\"]\n";
  // FIXME: should this dump also object returned by SVGFEImage::image() ?
  return ts;
}

sk_sp<SkImageFilter> FEImage::CreateImageFilterForLayoutObject(
    const LayoutObject& layout_object) {
  FloatRect dst_rect = FilterPrimitiveSubregion();

  AffineTransform transform;
  SVGElement* context_node = ToSVGElement(layout_object.GetNode());

  if (context_node->HasRelativeLengths()) {
    SVGLengthContext length_context(context_node);
    FloatSize viewport_size;

    // If we're referencing an element with percentage units, eg. <rect
    // with="30%"> those values were resolved against the viewport.  Build up a
    // transformation that maps from the viewport space to the filter primitive
    // subregion.
    if (length_context.DetermineViewport(viewport_size))
      transform =
          MakeMapBetweenRects(FloatRect(FloatPoint(), viewport_size), dst_rect);
  } else {
    transform.Translate(dst_rect.X(), dst_rect.Y());
  }

  // TODO(chrishtr): use tighter bounds for this.
  FloatRect bounds(LayoutRect::InfiniteIntRect());
  PaintRecordBuilder builder(bounds);
  SVGPaintContext::PaintResourceSubtree(builder.Context(), &layout_object);

  PaintRecorder paint_recorder;
  PaintCanvas* canvas = paint_recorder.beginRecording(dst_rect);
  canvas->concat(AffineTransformToSkMatrix(transform));
  builder.EndRecording(*canvas);

  return SkPictureImageFilter::Make(
      ToSkPicture(paint_recorder.finishRecordingAsPicture(), dst_rect));
}

sk_sp<SkImageFilter> FEImage::CreateImageFilter() {
  if (auto* layout_object = ReferencedLayoutObject())
    return CreateImageFilterForLayoutObject(*layout_object);

  sk_sp<SkImage> image =
      image_ ? image_->PaintImageForCurrentFrame().GetSkImage() : nullptr;
  if (!image) {
    // "A href reference that is an empty image (zero width or zero height),
    //  that fails to download, is non-existent, or that cannot be displayed
    //  (e.g. because it is not in a supported image format) fills the filter
    //  primitive subregion with transparent black."
    return CreateTransparentBlack();
  }

  FloatRect src_rect = FloatRect(FloatPoint(), FloatSize(image_->Size()));
  FloatRect dst_rect = FilterPrimitiveSubregion();

  preserve_aspect_ratio_->TransformRect(dst_rect, src_rect);

  return SkImageSource::Make(std::move(image), src_rect, dst_rect,
                             kHigh_SkFilterQuality);
}

}  // namespace blink
