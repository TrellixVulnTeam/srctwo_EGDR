// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/test/fake_tile_manager.h"

#include <stddef.h>
#include <stdint.h>

#include <deque>
#include <limits>

#include "base/memory/ptr_util.h"
#include "base/threading/thread_task_runner_handle.h"
#include "cc/raster/raster_buffer.h"
#include "cc/raster/synchronous_task_graph_runner.h"
#include "cc/test/fake_raster_buffer_provider.h"
#include "cc/test/fake_tile_task_manager.h"
#include "cc/trees/layer_tree_settings.h"

namespace cc {

namespace {

SynchronousTaskGraphRunner* GetGlobalTaskGraphRunner() {
  static auto* task_graph_runner = new SynchronousTaskGraphRunner;
  return task_graph_runner;
}

FakeRasterBufferProviderImpl* GetGlobalRasterBufferProvider() {
  static auto* buffer_provider = new FakeRasterBufferProviderImpl;
  return buffer_provider;
}

}  // namespace

FakeTileManager::FakeTileManager(TileManagerClient* client,
                                 ResourcePool* resource_pool)
    : TileManager(client,
                  base::ThreadTaskRunnerHandle::Get().get(),
                  nullptr,
                  std::numeric_limits<size_t>::max(),
                  TileManagerSettings()),
      image_decode_cache_(
          kN32_SkColorType,
          LayerTreeSettings().decoded_image_working_set_budget_bytes) {
  SetDecodedImageTracker(&decoded_image_tracker_);
  SetResources(resource_pool, &image_decode_cache_, GetGlobalTaskGraphRunner(),
               GetGlobalRasterBufferProvider(),
               std::numeric_limits<size_t>::max(),
               false /* use_gpu_rasterization */);
  SetTileTaskManagerForTesting(std::make_unique<FakeTileTaskManagerImpl>());
}

FakeTileManager::~FakeTileManager() {}

bool FakeTileManager::HasBeenAssignedMemory(Tile* tile) {
  return std::find(tiles_for_raster.begin(),
                   tiles_for_raster.end(),
                   tile) != tiles_for_raster.end();
}

}  // namespace cc
