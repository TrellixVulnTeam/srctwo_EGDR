// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_GPU_GPU_BENCHMARKING_EXTENSION_H_
#define CONTENT_RENDERER_GPU_GPU_BENCHMARKING_EXTENSION_H_

#include "base/macros.h"
#include "content/common/input/input_injector.mojom.h"
#include "gin/wrappable.h"

namespace gin {
class Arguments;
}

namespace v8 {
class Isolate;
class Object;
}

namespace content {

class RenderFrameImpl;

// gin class for gpu benchmarking
class GpuBenchmarking : public gin::Wrappable<GpuBenchmarking> {
 public:
  static gin::WrapperInfo kWrapperInfo;
  static void Install(RenderFrameImpl* frame);

 private:
  explicit GpuBenchmarking(RenderFrameImpl* frame);
  ~GpuBenchmarking() override;
  void EnsureRemoteInterface();

  // gin::Wrappable.
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) override;

  // JavaScript handlers.
  void SetNeedsDisplayOnAllLayers();
  void SetRasterizeOnlyVisibleContent();
  void PrintToSkPicture(v8::Isolate* isolate, const std::string& dirname);
  void PrintPagesToSkPictures(v8::Isolate* isolate,
                              const std::string& filename);
  void PrintPagesToXPS(v8::Isolate* isolate,
                         const std::string& filename);
  bool GestureSourceTypeSupported(int gesture_source_type);
  bool SmoothScrollBy(gin::Arguments* args);
  bool SmoothDrag(gin::Arguments* args);
  bool Swipe(gin::Arguments* args);
  bool ScrollBounce(gin::Arguments* args);
  bool PinchBy(gin::Arguments* args);
  bool Tap(gin::Arguments* args);
  bool PointerActionSequence(gin::Arguments* args);
  float VisualViewportX();
  float VisualViewportY();
  float VisualViewportHeight();
  float VisualViewportWidth();
  float PageScaleFactor();
  void ClearImageCache();
  int RunMicroBenchmark(gin::Arguments* args);
  bool SendMessageToMicroBenchmark(int id, v8::Local<v8::Object> message);
  bool HasGpuChannel();
  bool HasGpuProcess();
  void GetGpuDriverBugWorkarounds(gin::Arguments* args);

  RenderFrameImpl* render_frame_;
  mojom::InputInjectorPtr input_injector_;
  DISALLOW_COPY_AND_ASSIGN(GpuBenchmarking);
};

}  // namespace content

#endif  // CONTENT_RENDERER_GPU_GPU_BENCHMARKING_EXTENSION_H_
