// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_IPC_GFX_SKIA_IPC_EXPORT_H_
#define UI_GFX_IPC_GFX_SKIA_IPC_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(GFX_SKIA_IPC_IMPLEMENTATION)
#define GFX_SKIA_IPC_EXPORT __declspec(dllexport)
#else
#define GFX_SKIA_IPC_EXPORT __declspec(dllimport)
#endif  // defined(GFX_SKIA_IPC_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(GFX_SKIA_IPC_IMPLEMENTATION)
#define GFX_SKIA_IPC_EXPORT __attribute__((visibility("default")))
#else
#define GFX_SKIA_IPC_EXPORT
#endif
#endif

#else  // defined(COMPONENT_BUILD)
#define GFX_SKIA_IPC_EXPORT
#endif

#endif  // UI_GFX_IPC_GFX_SKIA_IPC_EXPORT_H_
