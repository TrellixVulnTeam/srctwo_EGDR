/*
 * Copyright (C) 2010 University of Szeged
 * Copyright (C) 2010 Zoltan Herczeg
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "platform/graphics/filters/FELighting.h"

#include "SkLightingImageFilter.h"
#include "SkPoint3.h"
#include "platform/graphics/filters/DistantLightSource.h"
#include "platform/graphics/filters/PointLightSource.h"
#include "platform/graphics/filters/SkiaImageFilterBuilder.h"
#include "platform/graphics/filters/SpotLightSource.h"

namespace blink {

FELighting::FELighting(Filter* filter,
                       LightingType lighting_type,
                       const Color& lighting_color,
                       float surface_scale,
                       float diffuse_constant,
                       float specular_constant,
                       float specular_exponent,
                       PassRefPtr<LightSource> light_source)
    : FilterEffect(filter),
      lighting_type_(lighting_type),
      light_source_(std::move(light_source)),
      lighting_color_(lighting_color),
      surface_scale_(surface_scale),
      diffuse_constant_(std::max(diffuse_constant, 0.0f)),
      specular_constant_(std::max(specular_constant, 0.0f)),
      specular_exponent_(clampTo(specular_exponent, 1.0f, 128.0f)) {}

sk_sp<SkImageFilter> FELighting::CreateImageFilter() {
  if (!light_source_)
    return CreateTransparentBlack();

  SkImageFilter::CropRect rect = GetCropRect();
  Color light_color = AdaptColorToOperatingInterpolationSpace(lighting_color_);
  sk_sp<SkImageFilter> input(SkiaImageFilterBuilder::Build(
      InputEffect(0), OperatingInterpolationSpace()));
  switch (light_source_->GetType()) {
    case LS_DISTANT: {
      DistantLightSource* distant_light_source =
          static_cast<DistantLightSource*>(light_source_.Get());
      float azimuth_rad = deg2rad(distant_light_source->Azimuth());
      float elevation_rad = deg2rad(distant_light_source->Elevation());
      const SkPoint3 direction = SkPoint3::Make(
          cosf(azimuth_rad) * cosf(elevation_rad),
          sinf(azimuth_rad) * cosf(elevation_rad), sinf(elevation_rad));
      if (specular_constant_ > 0)
        return SkLightingImageFilter::MakeDistantLitSpecular(
            direction, light_color.Rgb(), surface_scale_, specular_constant_,
            specular_exponent_, std::move(input), &rect);
      return SkLightingImageFilter::MakeDistantLitDiffuse(
          direction, light_color.Rgb(), surface_scale_, diffuse_constant_,
          std::move(input), &rect);
    }
    case LS_POINT: {
      PointLightSource* point_light_source =
          static_cast<PointLightSource*>(light_source_.Get());
      const FloatPoint3D position = point_light_source->GetPosition();
      const SkPoint3 sk_position =
          SkPoint3::Make(position.X(), position.Y(), position.Z());
      if (specular_constant_ > 0)
        return SkLightingImageFilter::MakePointLitSpecular(
            sk_position, light_color.Rgb(), surface_scale_, specular_constant_,
            specular_exponent_, std::move(input), &rect);
      return SkLightingImageFilter::MakePointLitDiffuse(
          sk_position, light_color.Rgb(), surface_scale_, diffuse_constant_,
          std::move(input), &rect);
    }
    case LS_SPOT: {
      SpotLightSource* spot_light_source =
          static_cast<SpotLightSource*>(light_source_.Get());
      const SkPoint3 location =
          SkPoint3::Make(spot_light_source->GetPosition().X(),
                         spot_light_source->GetPosition().Y(),
                         spot_light_source->GetPosition().Z());
      const SkPoint3 target =
          SkPoint3::Make(spot_light_source->Direction().X(),
                         spot_light_source->Direction().Y(),
                         spot_light_source->Direction().Z());
      float specular_exponent = spot_light_source->SpecularExponent();
      float limiting_cone_angle = spot_light_source->LimitingConeAngle();
      if (!limiting_cone_angle || limiting_cone_angle > 90 ||
          limiting_cone_angle < -90)
        limiting_cone_angle = 90;
      if (specular_constant_ > 0)
        return SkLightingImageFilter::MakeSpotLitSpecular(
            location, target, specular_exponent, limiting_cone_angle,
            light_color.Rgb(), surface_scale_, specular_constant_,
            specular_exponent_, std::move(input), &rect);
      return SkLightingImageFilter::MakeSpotLitDiffuse(
          location, target, specular_exponent, limiting_cone_angle,
          light_color.Rgb(), surface_scale_, diffuse_constant_,
          std::move(input), &rect);
    }
    default:
      NOTREACHED();
      return nullptr;
  }
}

}  // namespace blink
