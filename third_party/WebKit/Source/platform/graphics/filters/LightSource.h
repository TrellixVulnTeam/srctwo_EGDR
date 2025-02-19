/*
 * Copyright (C) 2008 Alex Mathews <possessedpenguinbob@gmail.com>
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2010 Zoltan Herczeg <zherczeg@webkit.org>
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

#ifndef LightSource_h
#define LightSource_h

#include "platform/PlatformExport.h"
#include "platform/geometry/FloatPoint3D.h"
#include "platform/wtf/Noncopyable.h"
#include "platform/wtf/PassRefPtr.h"
#include "platform/wtf/RefCounted.h"

namespace blink {

enum LightType { LS_DISTANT, LS_POINT, LS_SPOT };

class TextStream;

class PLATFORM_EXPORT LightSource : public RefCounted<LightSource> {
  WTF_MAKE_NONCOPYABLE(LightSource);

 public:
  LightSource(LightType type) : type_(type) {}

  virtual ~LightSource();

  LightType GetType() const { return type_; }
  virtual TextStream& ExternalRepresentation(TextStream&) const = 0;

  virtual bool SetAzimuth(float) { return false; }
  virtual bool SetElevation(float) { return false; }
  virtual bool SetPosition(const FloatPoint3D&) { return false; }
  virtual bool SetPointsAt(const FloatPoint3D&) { return false; }
  virtual bool SetSpecularExponent(float) { return false; }
  virtual bool SetLimitingConeAngle(float) { return false; }

 private:
  LightType type_;
};

}  // namespace blink

#endif  // LightSource_h
