// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/extensions/gfx_utils.h"

#include "base/lazy_instance.h"
#include "chrome/browser/chromeos/profiles/profile_helper.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/app_list/arc/arc_app_list_prefs.h"
#include "chrome/browser/ui/ash/multi_user/multi_user_util.h"
#include "chrome/common/extensions/extension_constants.h"
#include "chrome/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "extensions/browser/extension_registry.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/chromeos/resources/grit/ui_chromeos_resources.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_skia_operations.h"

namespace extensions {

namespace {

// The badge map between |arc_package_name| and |extension_id|. Note the mapping
// from |extension_id| to |arc_package_name| is unique, but the mapping from
// |arc_package_name| to |extension_id| is not.
const struct {
  const char* arc_package_name;
  const char* extension_id;
} kDualBadgeMap[] = {
    // Google Keep
    {"com.google.android.keep", "hmjkmjkepdijhoojdojkdfohbdgmmhki"},
    {"com.google.android.keep", "dondgdlndnpianbklfnehgdhkickdjck"},
    // GMail
    {"com.google.android.gm", extension_misc::kGmailAppId},
    {"com.google.android.gm", "bjdhhokmhgelphffoafoejjmlfblpdha"},
    // Google Drive
    {"com.google.android.apps.docs", extension_misc::kDriveHostedAppId},
    {"com.google.android.apps.docs", "mdhnphfgagkpdhndljccoackjjhghlif"},
    // Google Maps
    {"com.google.android.apps.maps", "lneaknkopdijkpnocmklfnjbeapigfbh"},
    // Calculator
    {"com.google.android.calculator", "joodangkbfjnajiiifokapkpmhfnpleo"},
    // Chrome Remote Desktop
    {"com.google.chromeremotedesktop", "gbchcmhmhahfdphkhkmpfmihenigjmpp"},
    {"com.google.chromeremotedesktop", "cdjikkcakjcdjemakobkmijmikhkegcj"},
    // Google Calender
    {"com.google.android.calendar", "ejjicmeblgpmajnghnpcppodonldlgfn"},
    {"com.google.android.calendar", "fpgfohogebplgnamlafljlcidjedbdeb"},
    // Google Docs
    {"com.google.android.apps.docs.editors.docs",
     extension_misc::kGoogleDocAppId},
    {"com.google.android.apps.docs.editors.docs",
     "npnjdccdffhdndcbeappiamcehbhjibf"},
    // Google Slides
    {"com.google.android.apps.docs.editors.slides",
     extension_misc::kGoogleSlidesAppId},
    {"com.google.android.apps.docs.editors.slides",
     "hdmobeajeoanbanmdlabnbnlopepchip"},
    // Google Sheets
    {"com.google.android.apps.docs.editors.sheets",
     extension_misc::kGoogleSheetsAppId},
    {"com.google.android.apps.docs.editors.sheets",
     "nifkmgcdokhkjghdlgflonppnefddien"},
    // YouTube
    {"com.google.android.youtube", extension_misc::kYoutubeAppId},
    {"com.google.android.youtube", "pbdihpaifchmclcmkfdgffnnpfbobefh"},
    // Google Play Books
    {"com.google.android.apps.books", extension_misc::kGooglePlayBooksAppId},
    // Google+
    {"com.google.android.apps.plus", "dlppkpafhbajpcmmoheippocdidnckmm"},
    {"com.google.android.apps.plus", "fgjnkhlabjcaajddbaenilcmpcidahll"},
    // Google Play Movies & TV
    {"com.google.android.videos", extension_misc::kGooglePlayMoviesAppId},
    {"com.google.android.videos", "amfoiggnkefambnaaphodjdmdooiinna"},
    // Google Play Music
    {"com.google.android.music", extension_misc::kGooglePlayMusicAppId},
    {"com.google.android.music", "onbhgdmifjebcabplolilidlpgeknifi"},
    // Google Now
    {"com.google.android.launcher", "mnfadmojomeniojkkikjpgjaegolkbpb"},
    // Google Photos
    {"com.google.android.apps.photos", "hcglmfcclpfgljeaiahehebeoaiicbko"},
    {"com.google.android.apps.photos", "efjnaogkjbogokcnohkmnjdojkikgobo"},
    {"com.google.android.apps.photos", "ifpkhncdnjfipfjlhfidljjffdgklanh"},
    // Google Classroom
    {"com.google.android.apps.classroom", "mfhehppjhmmnlfbbopchdfldgimhfhfk"},
    // Google Hangouts
    {"com.google.android.talk", "knipolnnllmklapflnccelgolnpehhpl"},
    {"com.google.android.talk", "cgmlfbhkckbedohgdepgbkflommbfkep"},
    {"com.google.android.talk", "gldgpnmcpaogjlojhhpebkbbanacoglc"},
    // Google Play Music
    {"com.google.android.music", "fahmaaghhglfmonjliepjlchgpgfmobi"},
    // Google News
    {"com.google.android.apps.genie.geniewidget",
     "dllkocilcinkggkchnjgegijklcililc"},
    // Used in unit tests.
    {"fake.package.name1", "emfkafnhnpcmabnnkckkchdilgeoekbo"},
};

// This class maintains the maps between the extension id and its equivalent
// ARC package name.
class AppDualBadgeMap {
 public:
  using ArcAppToExtensionsMap =
      std::unordered_map<std::string, std::vector<std::string>>;
  using ExtensionToArcAppMap = std::unordered_map<std::string, std::string>;

  AppDualBadgeMap() {
    for (size_t i = 0; i < arraysize(kDualBadgeMap); ++i) {
      arc_app_to_extensions_map_[kDualBadgeMap[i].arc_package_name].push_back(
          kDualBadgeMap[i].extension_id);
      extension_to_arc_app_map_[kDualBadgeMap[i].extension_id] =
          kDualBadgeMap[i].arc_package_name;
    }
  }

  std::vector<std::string> GetExtensionIdsForArcPackageName(
      std::string arc_package_name) {
    const auto iter = arc_app_to_extensions_map_.find(arc_package_name);
    if (iter == arc_app_to_extensions_map_.end())
      return std::vector<std::string>();
    return iter->second;
  }

  std::string GetArcPackageNameFromExtensionId(std::string extension_id) {
    const auto iter = extension_to_arc_app_map_.find(extension_id);
    if (iter == extension_to_arc_app_map_.end())
      return std::string();
    return iter->second;
  }

 private:
  ArcAppToExtensionsMap arc_app_to_extensions_map_;
  ExtensionToArcAppMap extension_to_arc_app_map_;

  DISALLOW_COPY_AND_ASSIGN(AppDualBadgeMap);
};

base::LazyInstance<AppDualBadgeMap>::DestructorAtExit g_dual_badge_map =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace

namespace util {

bool HasEquivalentInstalledArcApp(content::BrowserContext* context,
                                  const std::string& extension_id) {
  std::unordered_set<std::string> arc_apps;
  return GetEquivalentInstalledArcApps(context, extension_id, &arc_apps);
}

bool GetEquivalentInstalledArcApps(content::BrowserContext* context,
                                   const std::string& extension_id,
                                   std::unordered_set<std::string>* arc_apps) {
  const std::string arc_package_name =
      g_dual_badge_map.Get().GetArcPackageNameFromExtensionId(extension_id);
  if (arc_package_name.empty())
    return false;

  const ArcAppListPrefs* const prefs = ArcAppListPrefs::Get(context);
  if (!prefs)
    return false;

  // TODO(hidehiko): The icon is per launcher, so we should have more precise
  // check here.
  DCHECK(arc_apps);
  prefs->GetAppsForPackage(arc_package_name).swap(*arc_apps);
  return !arc_apps->empty();
}

const std::vector<std::string> GetEquivalentInstalledExtensions(
    content::BrowserContext* context,
    const std::string& arc_package_name) {
  const ExtensionRegistry* registry = ExtensionRegistry::Get(context);
  if (!registry)
    return std::vector<std::string>();

  std::vector<std::string> extension_ids =
      g_dual_badge_map.Get().GetExtensionIdsForArcPackageName(arc_package_name);
  if (extension_ids.empty())
    return std::vector<std::string>();

  extension_ids.erase(
      std::remove_if(extension_ids.begin(), extension_ids.end(),
                     [registry](std::string extension_id) {
                       return !registry->GetInstalledExtension(extension_id);
                     }),
      extension_ids.end());
  return extension_ids;
}

void MaybeApplyChromeBadge(content::BrowserContext* context,
                           const std::string& extension_id,
                           gfx::ImageSkia* icon_out) {
  DCHECK(context);
  DCHECK(icon_out);

  Profile* profile = Profile::FromBrowserContext(context);
  // Only apply Chrome badge for the primary profile.
  if (!chromeos::ProfileHelper::IsPrimaryProfile(profile) ||
      !multi_user_util::IsProfileFromActiveUser(profile)) {
    return;
  }

  const ExtensionRegistry* registry = ExtensionRegistry::Get(context);
  if (!registry || !registry->GetInstalledExtension(extension_id))
    return;

  if (!HasEquivalentInstalledArcApp(context, extension_id))
    return;

  const gfx::ImageSkia* badge_image =
      ResourceBundle::GetSharedInstance().GetImageSkiaNamed(
          IDR_ARC_DUAL_ICON_BADGE);
  DCHECK(badge_image);

  gfx::ImageSkia resized_badge_image = *badge_image;
  if (badge_image->size() != icon_out->size()) {
    resized_badge_image = gfx::ImageSkiaOperations::CreateResizedImage(
        *badge_image, skia::ImageOperations::RESIZE_BEST, icon_out->size());
  }
  *icon_out = gfx::ImageSkiaOperations::CreateSuperimposedImage(
      *icon_out, resized_badge_image);
  return;
}

}  // namespace util
}  // namespace extensions
