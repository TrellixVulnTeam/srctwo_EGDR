// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_COMMON_CAST_RESOURCE_DELEGATE_H_
#define CHROMECAST_COMMON_CAST_RESOURCE_DELEGATE_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "base/macros.h"
#include "base/memory/ref_counted_memory.h"
#include "ui/base/resource/resource_bundle.h"

namespace base {
class FilePath;
}

namespace gfx {
class Image;
}

namespace chromecast {

// A singleton resource bundle delegate. Primary purpose is to indicate the
// correct locale pack file to load.
class CastResourceDelegate : public ui::ResourceBundle::Delegate {
 public:
  // Returns the singleton of delegate. It doesn't create an instance.
  static CastResourceDelegate* GetInstance();

  CastResourceDelegate();
  ~CastResourceDelegate() override;

  // ui:ResourceBundle::Delegate implementation:
  base::FilePath GetPathForResourcePack(
      const base::FilePath& pack_path,
      ui::ScaleFactor scale_factor) override;
  base::FilePath GetPathForLocalePack(
      const base::FilePath& pack_path,
      const std::string& locale) override;
  gfx::Image GetImageNamed(int resource_id) override;
  gfx::Image GetNativeImageNamed(int resource_id) override;
  base::RefCountedStaticMemory* LoadDataResourceBytes(
      int resource_id,
      ui::ScaleFactor scale_factor) override;
  bool GetRawDataResource(int resource_id,
                          ui::ScaleFactor scale_factor,
                          base::StringPiece* value) override;
  bool GetLocalizedString(int message_id, base::string16* value) override;

  // Adds/removes/clears extra localized strings.
  void AddExtraLocalizedString(int resource_id,
                               const base::string16& localized);
  void RemoveExtraLocalizedString(int resource_id);
  void ClearAllExtraLocalizedStrings();

 private:
  using ExtraLocaledStringMap = std::unordered_map<int, base::string16>;

  ExtraLocaledStringMap extra_localized_strings_;

  DISALLOW_COPY_AND_ASSIGN(CastResourceDelegate);
};

}  // namespace chromecast

#endif  // CHROMECAST_COMMON_CAST_RESOURCE_DELEGATE_H_
