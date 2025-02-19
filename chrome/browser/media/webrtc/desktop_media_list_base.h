// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_MEDIA_WEBRTC_DESKTOP_MEDIA_LIST_BASE_H_
#define CHROME_BROWSER_MEDIA_WEBRTC_DESKTOP_MEDIA_LIST_BASE_H_

#include "chrome/browser/media/webrtc/desktop_media_list.h"
#include "content/public/browser/desktop_media_id.h"

namespace gfx {
class Image;
}

// Thumbnail size is 100*100 pixels
static const int kDefaultThumbnailSize = 100;

// Base class for DesktopMediaList implementations. Implements logic shared
// between implementations. Specifically it's responsible for keeping current
// list of sources and calling the observer when the list changes.
class DesktopMediaListBase : public DesktopMediaList {
 public:
  explicit DesktopMediaListBase(base::TimeDelta update_period);
  ~DesktopMediaListBase() override;

  // DesktopMediaList interface.
  void SetUpdatePeriod(base::TimeDelta period) override;
  void SetThumbnailSize(const gfx::Size& thumbnail_size) override;
  void SetViewDialogWindowId(content::DesktopMediaID dialog_id) override;
  void StartUpdating(DesktopMediaListObserver* observer) override;
  int GetSourceCount() const override;
  const Source& GetSource(int index) const override;
  content::DesktopMediaID::Type GetMediaListType() const override;

  static uint32_t GetImageHash(const gfx::Image& image);

 protected:
  struct SourceDescription {
    SourceDescription(content::DesktopMediaID id, const base::string16& name);

    content::DesktopMediaID id;
    base::string16 name;
  };

  virtual void Refresh() = 0;

  // Update source media list to observer.
  void UpdateSourcesList(const std::vector<SourceDescription>& new_sources);

  // Update a thumbnail to observer.
  void UpdateSourceThumbnail(content::DesktopMediaID id,
                             const gfx::ImageSkia& image);

  // Post a task for next list update.
  void ScheduleNextRefresh();

  // Size of thumbnails generated by the model.
  gfx::Size thumbnail_size_ =
      gfx::Size(kDefaultThumbnailSize, kDefaultThumbnailSize);

  // ID of the hosting dialog.
  content::DesktopMediaID view_dialog_id_ =
      content::DesktopMediaID(content::DesktopMediaID::TYPE_NONE, -1);

  // Desktop media type of the list.
  content::DesktopMediaID::Type type_ = content::DesktopMediaID::TYPE_NONE;

 private:
  // Time interval between mode updates.
  base::TimeDelta update_period_;

  // Current list of sources.
  std::vector<Source> sources_;

  // The observer passed to StartUpdating().
  DesktopMediaListObserver* observer_ = nullptr;

  base::WeakPtrFactory<DesktopMediaListBase> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(DesktopMediaListBase);
};

#endif  // CHROME_BROWSER_MEDIA_WEBRTC_DESKTOP_MEDIA_LIST_BASE_H_
