// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/background/background_application_list_model.h"

#include <algorithm>
#include <set>

#include "base/memory/ptr_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/background/background_contents_service.h"
#include "chrome/browser/background/background_contents_service_factory.h"
#include "chrome/browser/background/background_mode_manager.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/extensions/extension_constants.h"
#include "components/crx_file/id_util.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/image_loader.h"
#include "extensions/browser/notification_types.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_icon_set.h"
#include "extensions/common/extension_resource.h"
#include "extensions/common/extension_set.h"
#include "extensions/common/manifest_handlers/background_info.h"
#include "extensions/common/manifest_handlers/icons_handler.h"
#include "extensions/common/one_shot_event.h"
#include "extensions/common/permissions/permission_set.h"
#include "extensions/common/permissions/permissions_data.h"
#include "ui/base/l10n/l10n_util_collator.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_skia.h"

using extensions::APIPermission;
using extensions::Extension;
using extensions::ExtensionList;
using extensions::ExtensionRegistry;
using extensions::ExtensionSet;
using extensions::PermissionSet;
using extensions::UnloadedExtensionReason;
using extensions::UpdatedExtensionPermissionsInfo;

class ExtensionNameComparator {
 public:
  bool operator()(const scoped_refptr<const Extension>& x,
                  const scoped_refptr<const Extension>& y) {
    return x->name() < y->name();
  }
};

// Background application representation, private to the
// BackgroundApplicationListModel class.
class BackgroundApplicationListModel::Application
    : public base::SupportsWeakPtr<Application> {
 public:
  Application(BackgroundApplicationListModel* model,
              const Extension* an_extension);

  virtual ~Application();

  // Invoked when a request icon is available.
  void OnImageLoaded(const gfx::Image& image);

  // Uses the FILE thread to request this extension's icon, sized
  // appropriately.
  void RequestIcon(extension_misc::ExtensionIcons size);

  const Extension* extension_;
  std::unique_ptr<gfx::ImageSkia> icon_;
  BackgroundApplicationListModel* model_;
};

namespace {
void GetServiceApplications(ExtensionService* service,
                            ExtensionList* applications_result) {
  ExtensionRegistry* registry = ExtensionRegistry::Get(service->profile());
  const ExtensionSet& enabled_extensions = registry->enabled_extensions();

  for (const auto& extension : enabled_extensions) {
    if (BackgroundApplicationListModel::IsBackgroundApp(*extension,
                                                        service->profile())) {
      applications_result->push_back(extension);
    }
  }

  // Walk the list of terminated extensions also (just because an extension
  // crashed doesn't mean we should ignore it).
  const ExtensionSet& terminated_extensions = registry->terminated_extensions();
  for (const auto& extension : terminated_extensions) {
    if (BackgroundApplicationListModel::IsBackgroundApp(*extension,
                                                        service->profile())) {
      applications_result->push_back(extension);
    }
  }

  std::sort(applications_result->begin(), applications_result->end(),
            ExtensionNameComparator());
}

}  // namespace

void
BackgroundApplicationListModel::Observer::OnApplicationDataChanged(
    const Extension* extension, Profile* profile) {
}

void
BackgroundApplicationListModel::Observer::OnApplicationListChanged(
    Profile* profile) {
}

BackgroundApplicationListModel::Observer::~Observer() {
}

BackgroundApplicationListModel::Application::~Application() {
}

BackgroundApplicationListModel::Application::Application(
    BackgroundApplicationListModel* model,
    const Extension* extension)
    : extension_(extension), model_(model) {}

void BackgroundApplicationListModel::Application::OnImageLoaded(
    const gfx::Image& image) {
  if (image.IsEmpty())
    return;
  icon_.reset(image.CopyImageSkia());
  model_->SendApplicationDataChangedNotifications(extension_);
}

void BackgroundApplicationListModel::Application::RequestIcon(
    extension_misc::ExtensionIcons size) {
  extensions::ExtensionResource resource =
      extensions::IconsInfo::GetIconResource(
          extension_, size, ExtensionIconSet::MATCH_BIGGER);
  extensions::ImageLoader::Get(model_->profile_)->LoadImageAsync(
      extension_, resource, gfx::Size(size, size),
      base::Bind(&Application::OnImageLoaded, AsWeakPtr()));
}

BackgroundApplicationListModel::~BackgroundApplicationListModel() {
}

BackgroundApplicationListModel::BackgroundApplicationListModel(Profile* profile)
    : profile_(profile),
      ready_(false),
      extension_registry_observer_(this),
      weak_ptr_factory_(this) {
  DCHECK(profile_);
  registrar_.Add(this,
                 extensions::NOTIFICATION_EXTENSION_PERMISSIONS_UPDATED,
                 content::Source<Profile>(profile));
  registrar_.Add(this,
                 chrome::NOTIFICATION_BACKGROUND_CONTENTS_SERVICE_CHANGED,
                 content::Source<Profile>(profile));
  extensions::ExtensionSystem::Get(profile_)->ready().Post(
      FROM_HERE,
      base::Bind(&BackgroundApplicationListModel::OnExtensionSystemReady,
                 weak_ptr_factory_.GetWeakPtr()));
}

void BackgroundApplicationListModel::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void BackgroundApplicationListModel::AssociateApplicationData(
    const Extension* extension) {
  DCHECK(IsBackgroundApp(*extension, profile_));
  Application* application = FindApplication(extension);
  if (!application) {
    // App position is used as a dynamic command and so must be less than any
    // predefined command id.
    if (applications_.size() >= IDC_MinimumLabelValue) {
      LOG(ERROR) << "Background application limit of " << IDC_MinimumLabelValue
                 << " exceeded.  Ignoring.";
      return;
    }
    std::unique_ptr<Application> application_ptr =
        base::MakeUnique<Application>(this, extension);
    application = application_ptr.get();
    applications_[extension->id()] = std::move(application_ptr);
    application->RequestIcon(extension_misc::EXTENSION_ICON_BITTY);
  }
}

void BackgroundApplicationListModel::DissociateApplicationData(
    const Extension* extension) {
  applications_.erase(extension->id());
}

const Extension* BackgroundApplicationListModel::GetExtension(
    int position) const {
  DCHECK(position >= 0 && static_cast<size_t>(position) < extensions_.size());
  return extensions_[position].get();
}

const BackgroundApplicationListModel::Application*
BackgroundApplicationListModel::FindApplication(
    const Extension* extension) const {
  const std::string& id = extension->id();
  auto found = applications_.find(id);
  return (found == applications_.end()) ? nullptr : found->second.get();
}

BackgroundApplicationListModel::Application*
BackgroundApplicationListModel::FindApplication(
    const Extension* extension) {
  const std::string& id = extension->id();
  auto found = applications_.find(id);
  return (found == applications_.end()) ? nullptr : found->second.get();
}

const gfx::ImageSkia* BackgroundApplicationListModel::GetIcon(
    const Extension* extension) {
  const Application* application = FindApplication(extension);
  if (application)
    return application->icon_.get();
  AssociateApplicationData(extension);
  return nullptr;
}

int BackgroundApplicationListModel::GetPosition(
    const Extension* extension) const {
  int position = 0;
  const std::string& id = extension->id();
  for (const auto& it : extensions_) {
    if (id == it->id())
      return position;
    ++position;
  }
  NOTREACHED();
  return -1;
}

// static
bool BackgroundApplicationListModel::IsBackgroundApp(
    const Extension& extension, Profile* profile) {
  // An extension is a "background app" if it has the "background API"
  // permission, and meets one of the following criteria:
  // 1) It is an extension (not a hosted app).
  // 2) It is a hosted app, and has a background contents registered or in the
  //    manifest.

  // Not a background app if we don't have the background permission.
  if (!extension.permissions_data()->HasAPIPermission(
          APIPermission::kBackground)) {
    return false;
  }

  // Extensions and packaged apps with background permission are always treated
  // as background apps.
  if (!extension.is_hosted_app())
    return true;

  // Hosted apps with manifest-provided background pages are background apps.
  if (extensions::BackgroundInfo::HasBackgroundPage(&extension))
    return true;

  BackgroundContentsService* service =
      BackgroundContentsServiceFactory::GetForProfile(profile);
  base::string16 app_id = base::ASCIIToUTF16(extension.id());
  // If we have an active or registered background contents for this app, then
  // it's a background app. This covers the cases where the app has created its
  // background contents, but it hasn't navigated yet, or the background
  // contents crashed and hasn't yet been restarted - in both cases we still
  // want to treat the app as a background app.
  if (service->GetAppBackgroundContents(app_id) ||
      service->HasRegisteredBackgroundContents(app_id)) {
    return true;
  }

  // Doesn't meet our criteria, so it's not a background app.
  return false;
}

void BackgroundApplicationListModel::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  ExtensionService* service = extensions::ExtensionSystem::Get(profile_)->
      extension_service();
  if (!service || !service->is_ready())
    return;

  switch (type) {
    case extensions::NOTIFICATION_EXTENSION_PERMISSIONS_UPDATED:
      OnExtensionPermissionsUpdated(
          content::Details<UpdatedExtensionPermissionsInfo>(details)->extension,
          content::Details<UpdatedExtensionPermissionsInfo>(details)->reason,
          content::Details<UpdatedExtensionPermissionsInfo>(details)->
              permissions);
      break;
    case chrome::NOTIFICATION_BACKGROUND_CONTENTS_SERVICE_CHANGED:
      Update();
      break;
    default:
      NOTREACHED() << "Received unexpected notification";
  }
}

void BackgroundApplicationListModel::SendApplicationDataChangedNotifications(
    const Extension* extension) {
  for (auto& observer : observers_)
    observer.OnApplicationDataChanged(extension, profile_);
}

void BackgroundApplicationListModel::OnExtensionLoaded(
    content::BrowserContext* browser_context,
    const Extension* extension) {
  // We only care about extensions that are background applications.
  if (!IsBackgroundApp(*extension, profile_))
    return;
  Update();
  AssociateApplicationData(extension);
}

void BackgroundApplicationListModel::OnExtensionUnloaded(
    content::BrowserContext* browser_context,
    const Extension* extension,
    UnloadedExtensionReason reason) {
  if (!IsBackgroundApp(*extension, profile_))
    return;
  Update();
  DissociateApplicationData(extension);
}

void BackgroundApplicationListModel::OnExtensionSystemReady() {
  // All initial extensions will be loaded when extension system ready. So we
  // can get everything here.
  Update();
  for (const auto& extension : extensions_)
    AssociateApplicationData(extension.get());

  // If we register for extension loaded notifications in the ctor, we need to
  // know that this object is constructed prior to the initialization process
  // for the extension system, which isn't a guarantee. Thus, register here and
  // associate all initial extensions.
  extension_registry_observer_.Add(ExtensionRegistry::Get(profile_));
  ready_ = true;
}

void BackgroundApplicationListModel::OnShutdown(ExtensionRegistry* registry) {
  DCHECK_EQ(ExtensionRegistry::Get(profile_), registry);
  extension_registry_observer_.Remove(registry);
}

void BackgroundApplicationListModel::OnExtensionPermissionsUpdated(
    const Extension* extension,
    UpdatedExtensionPermissionsInfo::Reason reason,
    const PermissionSet& permissions) {
  if (permissions.HasAPIPermission(APIPermission::kBackground)) {
    switch (reason) {
      case UpdatedExtensionPermissionsInfo::ADDED:
        DCHECK(IsBackgroundApp(*extension, profile_));
        Update();
        AssociateApplicationData(extension);
        break;
      case UpdatedExtensionPermissionsInfo::REMOVED:
        DCHECK(!IsBackgroundApp(*extension, profile_));
        Update();
        DissociateApplicationData(extension);
        break;
      default:
        NOTREACHED();
    }
  }
}

void BackgroundApplicationListModel::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

// Update queries the extensions service of the profile with which the model was
// initialized to determine the current set of background applications.  If that
// differs from the old list, it generates OnApplicationListChanged events for
// each observer.
void BackgroundApplicationListModel::Update() {
  ExtensionService* service = extensions::ExtensionSystem::Get(profile_)->
      extension_service();

  // Discover current background applications, compare with previous list, which
  // is consistently sorted, and notify observers if they differ.
  ExtensionList extensions;
  GetServiceApplications(service, &extensions);
  ExtensionList::const_iterator old_cursor = extensions_.begin();
  ExtensionList::const_iterator new_cursor = extensions.begin();
  while (old_cursor != extensions_.end() &&
         new_cursor != extensions.end() &&
         (*old_cursor)->name() == (*new_cursor)->name() &&
         (*old_cursor)->id() == (*new_cursor)->id()) {
    ++old_cursor;
    ++new_cursor;
  }
  if (old_cursor != extensions_.end() || new_cursor != extensions.end()) {
    extensions_ = extensions;
    for (auto& observer : observers_)
      observer.OnApplicationListChanged(profile_);
  }
}
