/*
 * Copyright (C) 2008 Alex Mathews <possessedpenguinbob@gmail.com>
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
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

#ifndef DistantLightSource_h
#define DistantLightSource_h

#include "platform/graphics/filters/LightSource.h"

namespace blink {

class PLATFORM_EXPORT DistantLightSource final : public LightSource {
 public:
  static PassRefPtr<DistantLightSource> Create(float azimuth, float elevation) {
    return AdoptRef(new DistantLightSource(azimuth, elevation));
  }

  float Azimuth() const { return azimuth_; }
  float Elevation() const { return elevation_; }

  bool SetAzimuth(float) override;
  bool SetElevation(float) override;

  TextStream& ExternalRepresentation(TextStream&) const override;

 private:
  DistantLightSource(float azimuth, float elevation)
      : LightSource(LS_DISTANT), azimuth_(azimuth), elevation_(elevation) {}

  float azimuth_;
  float elevation_;
};

}  // namespace blink

#endif  // DistantLightSource_h
