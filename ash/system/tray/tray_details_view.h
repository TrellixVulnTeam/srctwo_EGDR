// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_SYSTEM_TRAY_TRAY_DETAILS_VIEW_H_
#define ASH_SYSTEM_TRAY_TRAY_DETAILS_VIEW_H_

#include <memory>

#include "ash/ash_export.h"
#include "ash/system/tray/tray_constants.h"
#include "ash/system/tray/view_click_listener.h"
#include "base/macros.h"
#include "base/timer/timer.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/view.h"

namespace gfx {
struct VectorIcon;
}  // namespace gfx

namespace views {
class BoxLayout;
class Button;
class ProgressBar;
class ScrollView;
}  // namespace views

namespace ash {

class HoverHighlightView;
class ScrollBorder;
class SystemTrayItem;
class TriView;

class ASH_EXPORT TrayDetailsView : public views::View,
                                   public ViewClickListener,
                                   public views::ButtonListener {
 public:
  explicit TrayDetailsView(SystemTrayItem* owner);
  ~TrayDetailsView() override;

  // ViewClickListener:
  // Don't override this --- override HandleViewClicked.
  void OnViewClicked(views::View* sender) final;

  // views::ButtonListener:
  // Don't override this --- override HandleButtonPressed.
  void ButtonPressed(views::Button* sender, const ui::Event& event) final;

  SystemTrayItem* owner() { return owner_; }

 protected:
  // views::View:
  void Layout() override;
  int GetHeightForWidth(int width) const override;

  // Exposes the layout manager of this view to give control to subclasses.
  views::BoxLayout* box_layout() { return box_layout_; }

  // Creates the row containing the back button and title. For material design
  // this appears at the top of the view, for non-material design it appears
  // at the bottom.
  void CreateTitleRow(int string_id);

  // Creates a scrollable list. The list has a border at the bottom if there is
  // any other view between the list and the footer row at the bottom.
  void CreateScrollableList();

  // Adds a targetable row to |scroll_content_| containing |icon| and |text|.
  HoverHighlightView* AddScrollListItem(const gfx::VectorIcon& icon,
                                        const base::string16& text);

  // Adds a targetable row to |scroll_content_| containing |icon|, |text|, and a
  // checkbox. |checked| determines whether the checkbox is checked or not.
  HoverHighlightView* AddScrollListCheckableItem(const gfx::VectorIcon& icon,
                                                 const base::string16& text,
                                                 bool checked);

  // Adds a targetable row to |scroll_content_| containing |text| and a
  // checkbox. |checked| determines whether the checkbox is checked or not.
  HoverHighlightView* AddScrollListCheckableItem(const base::string16& text,
                                                 bool checked);

  // Adds connected sub label to the |view| with appropriate style.
  void SetupConnectedScrollListItem(HoverHighlightView* view);

  // Adds connecting sub label to the |view| with appropriate style.
  void SetupConnectingScrollListItem(HoverHighlightView* view);

  // Adds a sticky sub header to |scroll_content_| containing |icon| and a text
  // represented by |text_id| resource id.
  TriView* AddScrollListSubHeader(const gfx::VectorIcon& icon, int text_id);

  // Adds a sticky sub header to |scroll_content_| containing a text represented
  // by |text_id| resource id.
  TriView* AddScrollListSubHeader(int text_id);

  // Removes (and destroys) all child views.
  void Reset();

  // Shows or hides the progress bar below the title row. It occupies the same
  // space as the separator, so when shown the separator is hidden. If
  // |progress_bar_| doesn't already exist it will be created.
  void ShowProgress(double value, bool visible);

  // Helper functions which create and return the settings and help buttons,
  // respectively, used in the material design top-most header row. The caller
  // assumes ownership of the returned buttons.
  views::Button* CreateSettingsButton(int setting_accessible_name_id);
  views::Button* CreateHelpButton();

  TriView* tri_view() { return tri_view_; }
  views::ScrollView* scroller() const { return scroller_; }
  views::View* scroll_content() const { return scroll_content_; }

 private:
  friend class TrayDetailsViewTest;

  // Overridden to handle clicks on subclass-specific views.
  virtual void HandleViewClicked(views::View* view);

  // Overridden to handle button presses on subclass-specific buttons.
  virtual void HandleButtonPressed(views::Button* sender,
                                   const ui::Event& event);

  // Creates and adds subclass-specific buttons to the title row.
  virtual void CreateExtraTitleRowButtons();

  // Transition to default view from details view. If |title_row_| has focus
  // before transition, the default view should focus on the owner of this
  // details view.
  //
  // In Material Design the actual transition is intentionally delayed to allow
  // the user to perceive the ink drop animation on the clicked target.
  void TransitionToDefaultView();

  // Actually transitions to the default view.
  void DoTransitionToDefaultView();

  // Helper function which creates and returns the back button used in the
  // material design top-most header row. The caller assumes ownership of the
  // returned button.
  views::Button* CreateBackButton();

  SystemTrayItem* owner_;
  views::BoxLayout* box_layout_;
  views::ScrollView* scroller_;
  views::View* scroll_content_;
  views::ProgressBar* progress_bar_;

  ScrollBorder* scroll_border_;  // Weak reference

  // The container view for the top-most title row in material design.
  TriView* tri_view_;

  // The back button that appears in the material design title row. Not owned.
  views::Button* back_button_;

  // Used to delay the transition to the default view.
  base::OneShotTimer transition_delay_timer_;

  DISALLOW_COPY_AND_ASSIGN(TrayDetailsView);
};

}  // namespace ash

#endif  // ASH_SYSTEM_TRAY_TRAY_DETAILS_VIEW_H_
