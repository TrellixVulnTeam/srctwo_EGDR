// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GL_GL_IMAGE_NATIVE_PIXMAP_H_
#define UI_GL_GL_IMAGE_NATIVE_PIXMAP_H_

#include <stdint.h>

#include <string>

#include "ui/gfx/native_pixmap.h"
#include "ui/gl/gl_export.h"
#include "ui/gl/gl_image_egl.h"

namespace gl {

class GL_EXPORT GLImageNativePixmap : public gl::GLImageEGL {
 public:
  GLImageNativePixmap(const gfx::Size& size, unsigned internalformat);

  bool Initialize(gfx::NativePixmap* pixmap, gfx::BufferFormat format);

  // Overridden from GLImage:
  unsigned GetInternalFormat() override;
  bool CopyTexImage(unsigned target) override;
  bool ScheduleOverlayPlane(gfx::AcceleratedWidget widget,
                            int z_order,
                            gfx::OverlayTransform transform,
                            const gfx::Rect& bounds_rect,
                            const gfx::RectF& crop_rect) override;
  void Flush() override;
  void OnMemoryDump(base::trace_event::ProcessMemoryDump* pmd,
                    uint64_t process_tracing_id,
                    const std::string& dump_name) override;

  static unsigned GetInternalFormatForTesting(gfx::BufferFormat format);

 protected:
  ~GLImageNativePixmap() override;

 private:
  unsigned internalformat_;
  scoped_refptr<gfx::NativePixmap> pixmap_;
  bool has_image_flush_external_;
};

}  // namespace gl

#endif  // UI_GL_GL_IMAGE_NATIVE_PIXMAP_H_
