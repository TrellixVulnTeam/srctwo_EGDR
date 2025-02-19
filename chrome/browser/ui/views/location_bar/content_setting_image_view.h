// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_LOCATION_BAR_CONTENT_SETTING_IMAGE_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_LOCATION_BAR_CONTENT_SETTING_IMAGE_VIEW_H_

#include <memory>

#include "base/macros.h"
#include "chrome/browser/ui/content_settings/content_setting_image_model.h"
#include "chrome/browser/ui/views/location_bar/icon_label_bubble_view.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "ui/gfx/animation/animation_delegate.h"
#include "ui/gfx/animation/slide_animation.h"
#include "ui/views/painter.h"
#include "ui/views/view.h"

class ContentSettingImageModel;
class LocationBarView;

namespace content {
class WebContents;
}

namespace gfx {
class FontList;
}

namespace views {
class BubbleDialogDelegateView;
}

// The ContentSettingImageView displays an icon and optional text label for
// various content settings affordances in the location bar (i.e. plugin
// blocking, geolocation).
class ContentSettingImageView : public IconLabelBubbleView {
 public:
  ContentSettingImageView(std::unique_ptr<ContentSettingImageModel> image_model,
                          LocationBarView* parent,
                          const gfx::FontList& font_list);
  ~ContentSettingImageView() override;

  // Updates the decoration from the shown WebContents.
  void Update(content::WebContents* web_contents);

 private:
  // The total animation time, including open and close as well as an
  // intervening "stay open" period.
  static const int kAnimationDurationMS;

  // IconLabelBubbleView:
  const char* GetClassName() const override;
  void OnBoundsChanged(const gfx::Rect& previous_bounds) override;
  bool GetTooltipText(const gfx::Point& p,
                      base::string16* tooltip) const override;
  void OnNativeThemeChanged(const ui::NativeTheme* native_theme) override;
  SkColor GetTextColor() const override;
  bool ShouldShowLabel() const override;
  double WidthMultiplier() const override;
  bool IsShrinking() const override;
  bool ShowBubble(const ui::Event& event) override;
  bool IsBubbleShowing() const override;

  // gfx::AnimationDelegate:
  void AnimationEnded(const gfx::Animation* animation) override;
  void AnimationProgressed(const gfx::Animation* animation) override;
  void AnimationCanceled(const gfx::Animation* animation) override;

  // views::WidgetObserver:
  void OnWidgetDestroying(views::Widget* widget) override;
  void OnWidgetVisibilityChanged(views::Widget* widget, bool visible) override;

  // Updates the image and tooltip to match the current model state.
  void UpdateImage();

  // Animates the view in and disables highlighting for hover and focus.
  // TODO(bruthig): See crbug.com/669253. Since the ink drop highlight currently
  // cannot handle host resizes, the highlight needs to be disabled when the
  // animation is running.
  void AnimateIn();

  LocationBarView* parent_;  // Weak, owns us.
  std::unique_ptr<ContentSettingImageModel> content_setting_image_model_;
  gfx::SlideAnimation slide_animator_;
  bool pause_animation_;
  double pause_animation_state_;
  views::BubbleDialogDelegateView* bubble_view_;

  DISALLOW_COPY_AND_ASSIGN(ContentSettingImageView);
};

#endif  // CHROME_BROWSER_UI_VIEWS_LOCATION_BAR_CONTENT_SETTING_IMAGE_VIEW_H_
