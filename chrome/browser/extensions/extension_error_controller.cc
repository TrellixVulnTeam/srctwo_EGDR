// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extension_error_controller.h"

#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/pending_extension_manager.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/extension_set.h"

namespace extensions {

namespace {

ExtensionErrorController::UICreateMethod g_create_ui =
    ExtensionErrorUI::Create;

}

ExtensionErrorController::ExtensionErrorController(
    content::BrowserContext* context,
    bool is_first_run)
    : browser_context_(context),
      is_first_run_(is_first_run) {}

ExtensionErrorController::~ExtensionErrorController() {}

void ExtensionErrorController::ShowErrorIfNeeded() {
  IdentifyAlertableExtensions();

  // Make sure there's something to show, and that there isn't currently a
  // bubble displaying.
  if (!blacklisted_extensions_.is_empty() && !error_ui_.get()) {
    if (!is_first_run_) {
      error_ui_.reset(g_create_ui(this));
      if (!error_ui_->ShowErrorInBubbleView())  // Couldn't find a browser.
        error_ui_.reset();
    } else {
      // First run. Just acknowledge all the extensions, silently, by
      // shortcutting the display of the UI and going straight to the
      // callback for pressing the Accept button.
      OnAlertClosed();
    }
  }
}

// static
void ExtensionErrorController::SetUICreateMethodForTesting(
    UICreateMethod method) {
  g_create_ui = method;
}

content::BrowserContext* ExtensionErrorController::GetContext() {
  return browser_context_;
}

const ExtensionSet& ExtensionErrorController::GetExternalExtensions() {
  return external_extensions_;
}

const ExtensionSet& ExtensionErrorController::GetBlacklistedExtensions() {
  return blacklisted_extensions_;
}

void ExtensionErrorController::OnAlertAccept() {
  error_ui_->Close();
}

void ExtensionErrorController::OnAlertDetails() {
  error_ui_->ShowExtensions();

  // ShowExtensions() may cause the error UI to close synchronously, e.g. if it
  // causes a navigation.
  if (error_ui_)
    error_ui_->Close();
}

void ExtensionErrorController::OnAlertClosed() {
  ExtensionPrefs* prefs = ExtensionPrefs::Get(browser_context_);
  for (ExtensionSet::const_iterator iter = blacklisted_extensions_.begin();
       iter != blacklisted_extensions_.end();
       ++iter) {
    prefs->AcknowledgeBlacklistedExtension((*iter)->id());
  }

  blacklisted_extensions_.Clear();
  error_ui_.reset();
}

void ExtensionErrorController::IdentifyAlertableExtensions() {
  ExtensionRegistry* registry = ExtensionRegistry::Get(browser_context_);
  ExtensionPrefs* prefs = ExtensionPrefs::Get(browser_context_);

  // This should be clear, but in case a bubble crashed somewhere along the
  // line, let's make sure we start fresh.
  blacklisted_extensions_.Clear();

  // Build up the lists of extensions that require acknowledgment. If this is
  // the first time, grandfather extensions that would have caused
  // notification.

  const ExtensionSet& blacklisted_set = registry->blacklisted_extensions();
  for (ExtensionSet::const_iterator iter = blacklisted_set.begin();
       iter != blacklisted_set.end();
       ++iter) {
    if (!prefs->IsBlacklistedExtensionAcknowledged((*iter)->id()))
      blacklisted_extensions_.Insert(*iter);
  }

  ExtensionSystem* system = ExtensionSystem::Get(browser_context_);
  ManagementPolicy* management_policy = system->management_policy();
  PendingExtensionManager* pending_extension_manager =
      system->extension_service()->pending_extension_manager();
  const ExtensionSet& enabled_set = registry->enabled_extensions();

  for (ExtensionSet::const_iterator iter = enabled_set.begin();
       iter != enabled_set.end();
       ++iter) {
    const Extension* extension = iter->get();

    // Skip for extensions that have pending updates. They will be checked again
    // once the pending update is finished.
    if (pending_extension_manager->IsIdPending(extension->id()))
      continue;

    // Extensions disabled by policy. Note: this no longer includes blacklisted
    // extensions, though we still show the same UI.
    if (!management_policy->UserMayLoad(extension, NULL /* ignore error */)) {
      if (!prefs->IsBlacklistedExtensionAcknowledged(extension->id()))
        blacklisted_extensions_.Insert(extension);
    }
  }
}

}  // namespace extensions
