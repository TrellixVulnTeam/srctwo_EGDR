/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
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

#ifndef FEDisplacementMap_h
#define FEDisplacementMap_h

#include "platform/graphics/filters/FilterEffect.h"

namespace blink {

enum ChannelSelectorType {
  CHANNEL_UNKNOWN = 0,
  CHANNEL_R = 1,
  CHANNEL_G = 2,
  CHANNEL_B = 3,
  CHANNEL_A = 4
};

class PLATFORM_EXPORT FEDisplacementMap final : public FilterEffect {
 public:
  static FEDisplacementMap* Create(Filter*,
                                   ChannelSelectorType x_channel_selector,
                                   ChannelSelectorType y_channel_selector,
                                   float);

  ChannelSelectorType XChannelSelector() const;
  bool SetXChannelSelector(const ChannelSelectorType);

  ChannelSelectorType YChannelSelector() const;
  bool SetYChannelSelector(const ChannelSelectorType);

  float Scale() const;
  bool SetScale(float);

  TextStream& ExternalRepresentation(TextStream&, int indention) const override;

 private:
  FEDisplacementMap(Filter*,
                    ChannelSelectorType x_channel_selector,
                    ChannelSelectorType y_channel_selector,
                    float);

  FloatRect MapInputs(const FloatRect&) const override;
  FloatRect MapEffect(const FloatRect&) const override;

  sk_sp<SkImageFilter> CreateImageFilter() override;

  ChannelSelectorType x_channel_selector_;
  ChannelSelectorType y_channel_selector_;
  float scale_;
};

}  // namespace blink

#endif  // FEDisplacementMap_h
