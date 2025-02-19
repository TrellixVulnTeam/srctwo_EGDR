// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/test_extension_registry_observer.h"

#include "base/macros.h"
#include "base/run_loop.h"
#include "extensions/browser/extension_registry.h"

namespace extensions {

class TestExtensionRegistryObserver::Waiter {
 public:
  Waiter() : observed_(false), extension_(nullptr) {}

  void Wait() {
    if (!observed_)
      run_loop_.Run();
  }

  void OnObserved(const Extension* extension) {
    observed_ = true;
    run_loop_.Quit();
    extension_ = extension;
  }

  const Extension* extension() const { return extension_; }

 private:
  bool observed_;
  base::RunLoop run_loop_;
  const Extension* extension_;

  DISALLOW_COPY_AND_ASSIGN(Waiter);
};

TestExtensionRegistryObserver::TestExtensionRegistryObserver(
    ExtensionRegistry* registry)
    : TestExtensionRegistryObserver(registry, std::string()) {
}

TestExtensionRegistryObserver::TestExtensionRegistryObserver(
    ExtensionRegistry* registry,
    const std::string& extension_id)
    : will_be_installed_waiter_(new Waiter()),
      uninstalled_waiter_(new Waiter()),
      loaded_waiter_(new Waiter()),
      unloaded_waiter_(new Waiter()),
      extension_registry_observer_(this),
      extension_id_(extension_id) {
  extension_registry_observer_.Add(registry);
}

TestExtensionRegistryObserver::~TestExtensionRegistryObserver() {
}

const Extension* TestExtensionRegistryObserver::WaitForExtensionUninstalled() {
  return Wait(&uninstalled_waiter_);
}

const Extension*
TestExtensionRegistryObserver::WaitForExtensionWillBeInstalled() {
  return Wait(&will_be_installed_waiter_);
}

const Extension* TestExtensionRegistryObserver::WaitForExtensionLoaded() {
  return Wait(&loaded_waiter_);
}

const Extension* TestExtensionRegistryObserver::WaitForExtensionUnloaded() {
  return Wait(&unloaded_waiter_);
}

void TestExtensionRegistryObserver::OnExtensionWillBeInstalled(
    content::BrowserContext* browser_context,
    const Extension* extension,
    bool is_update,
    const std::string& old_name) {
  if (extension_id_.empty() || extension->id() == extension_id_)
    will_be_installed_waiter_->OnObserved(extension);
}

void TestExtensionRegistryObserver::OnExtensionUninstalled(
    content::BrowserContext* browser_context,
    const Extension* extension,
    extensions::UninstallReason reason) {
  if (extension_id_.empty() || extension->id() == extension_id_)
    uninstalled_waiter_->OnObserved(extension);
}

void TestExtensionRegistryObserver::OnExtensionLoaded(
    content::BrowserContext* browser_context,
    const Extension* extension) {
  if (extension_id_.empty() || extension->id() == extension_id_)
    loaded_waiter_->OnObserved(extension);
}

void TestExtensionRegistryObserver::OnExtensionUnloaded(
    content::BrowserContext* browser_context,
    const Extension* extension,
    UnloadedExtensionReason reason) {
  if (extension_id_.empty() || extension->id() == extension_id_)
    unloaded_waiter_->OnObserved(extension);
}

const Extension* TestExtensionRegistryObserver::Wait(
    std::unique_ptr<Waiter>* waiter) {
  waiter->get()->Wait();
  const Extension* extension = waiter->get()->extension();
  // Reset the waiter for future uses.
  // We could have a Waiter::Reset method, but it would reset every field in the
  // class, so let's just reset the pointer.
  waiter->reset(new Waiter());
  return extension;
}

}  // namespace extensions
