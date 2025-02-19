// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/test/content_test_suite.h"

#include "base/base_paths.h"
#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/macros.h"
#include "build/build_config.h"
#include "content/public/common/content_client.h"
#include "content/public/common/content_paths.h"
#include "content/public/test/test_content_client_initializer.h"
#include "gpu/config/gpu_info_collector.h"
#include "gpu/config/gpu_util.h"
#include "gpu/ipc/in_process_command_buffer.h"
#include "media/base/media.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gl/init/gl_factory.h"
#include "ui/gl/test/gl_surface_test_support.h"

#if defined(OS_WIN)
#include "ui/display/win/dpi.h"
#endif

#if defined(OS_MACOSX)
#include "base/mac/scoped_nsautorelease_pool.h"
#include "base/test/mock_chrome_application_mac.h"
#endif

#if defined(OS_ANDROID)
#include "content/browser/media/android/browser_media_player_manager.h"
#endif

namespace content {
namespace {

class TestInitializationListener : public testing::EmptyTestEventListener {
 public:
  TestInitializationListener() : test_content_client_initializer_(NULL) {
  }

  void OnTestStart(const testing::TestInfo& test_info) override {
    test_content_client_initializer_ =
        new content::TestContentClientInitializer();
  }

  void OnTestEnd(const testing::TestInfo& test_info) override {
    delete test_content_client_initializer_;
  }

 private:
  content::TestContentClientInitializer* test_content_client_initializer_;

  DISALLOW_COPY_AND_ASSIGN(TestInitializationListener);
};

}  // namespace

ContentTestSuite::ContentTestSuite(int argc, char** argv)
    : ContentTestSuiteBase(argc, argv) {
}

ContentTestSuite::~ContentTestSuite() {
}

void ContentTestSuite::Initialize() {
#if defined(OS_MACOSX)
  base::mac::ScopedNSAutoreleasePool autorelease_pool;
  mock_cr_app::RegisterMockCrApp();
#endif

#if defined(OS_WIN)
  display::win::SetDefaultDeviceScaleFactor(1.0f);
#endif

  ContentTestSuiteBase::Initialize();
  {
    ContentClient client;
    ContentTestSuiteBase::RegisterContentSchemes(&client);
  }
  RegisterPathProvider();
  media::InitializeMediaLibrary();
  // When running in a child process for Mac sandbox tests, the sandbox exists
  // to initialize GL, so don't do it here.
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  bool is_child_process = command_line->HasSwitch(switches::kTestChildProcess);
  if (!is_child_process) {
    gpu::GPUInfo gpu_info;
    gpu::CollectBasicGraphicsInfo(&gpu_info);
    gpu::GpuFeatureInfo gpu_feature_info =
        gpu::ComputeGpuFeatureInfo(gpu_info, command_line);
    gpu::InProcessCommandBuffer::InitializeDefaultServiceForTesting(
        gpu_feature_info);
    gl::GLSurfaceTestSupport::InitializeNoExtensionsOneOff();
    gl::init::SetDisabledExtensionsPlatform(
        gpu_feature_info.disabled_extensions);
    gl::init::InitializeExtensionSettingsOneOffPlatform();
  }
  testing::TestEventListeners& listeners =
      testing::UnitTest::GetInstance()->listeners();
  listeners.Append(new TestInitializationListener);
}

}  // namespace content
