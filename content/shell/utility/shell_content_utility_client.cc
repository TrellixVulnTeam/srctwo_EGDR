// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/utility/shell_content_utility_client.h"

#include <memory>
#include <utility>

#include "base/bind.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/ptr_util.h"
#include "base/process/process.h"
#include "content/public/child/child_thread.h"
#include "content/public/common/service_manager_connection.h"
#include "content/public/common/simple_connection_filter.h"
#include "content/public/test/test_service.h"
#include "content/public/test/test_service.mojom.h"
#include "content/shell/common/power_monitor_test_impl.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "mojo/public/cpp/system/buffer.h"
#include "services/service_manager/public/cpp/binder_registry.h"

namespace content {

namespace {

class TestServiceImpl : public mojom::TestService {
 public:
  static void Create(mojom::TestServiceRequest request) {
    mojo::MakeStrongBinding(base::WrapUnique(new TestServiceImpl),
                            std::move(request));
  }

  // mojom::TestService implementation:
  void DoSomething(DoSomethingCallback callback) override {
    std::move(callback).Run();
  }

  void DoTerminateProcess(DoTerminateProcessCallback callback) override {
    base::Process::Current().Terminate(0, false);
  }

  void CreateFolder(CreateFolderCallback callback) override {
    // Note: This is used to check if the sandbox is disabled or not since
    //       creating a folder is forbidden when it is enabled.
    std::move(callback).Run(base::ScopedTempDir().CreateUniqueTempDir());
  }

  void GetRequestorName(GetRequestorNameCallback callback) override {
    NOTREACHED();
  }

  void CreateSharedBuffer(const std::string& message,
                          CreateSharedBufferCallback callback) override {
    mojo::ScopedSharedBufferHandle buffer =
        mojo::SharedBufferHandle::Create(message.size());
    CHECK(buffer.is_valid());

    mojo::ScopedSharedBufferMapping mapping = buffer->Map(message.size());
    CHECK(mapping);
    std::copy(message.begin(), message.end(),
              reinterpret_cast<char*>(mapping.get()));

    std::move(callback).Run(std::move(buffer));
  }

 private:
  explicit TestServiceImpl() {}

  DISALLOW_COPY_AND_ASSIGN(TestServiceImpl);
};

std::unique_ptr<service_manager::Service> CreateTestService() {
  return std::unique_ptr<service_manager::Service>(new TestService);
}

}  // namespace

ShellContentUtilityClient::ShellContentUtilityClient() {}

ShellContentUtilityClient::~ShellContentUtilityClient() {
}

void ShellContentUtilityClient::UtilityThreadStarted() {
  auto registry = base::MakeUnique<service_manager::BinderRegistry>();
  registry->AddInterface(base::Bind(&TestServiceImpl::Create),
                         base::ThreadTaskRunnerHandle::Get());
  registry->AddInterface<mojom::PowerMonitorTest>(
      base::Bind(&PowerMonitorTestImpl::MakeStrongBinding,
                 base::Passed(base::MakeUnique<PowerMonitorTestImpl>())),
      base::ThreadTaskRunnerHandle::Get());
  content::ChildThread::Get()
      ->GetServiceManagerConnection()
      ->AddConnectionFilter(
          base::MakeUnique<SimpleConnectionFilter>(std::move(registry)));
}

void ShellContentUtilityClient::RegisterServices(StaticServiceMap* services) {
  service_manager::EmbeddedServiceInfo info;
  info.factory = base::Bind(&CreateTestService);
  services->insert(std::make_pair(kTestServiceUrl, info));
}

void ShellContentUtilityClient::RegisterNetworkBinders(
    service_manager::BinderRegistry* registry) {
  network_service_test_helper_.RegisterNetworkBinders(registry);
}

}  // namespace content
