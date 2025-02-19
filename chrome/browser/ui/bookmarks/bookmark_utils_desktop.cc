// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/bookmarks/bookmark_utils_desktop.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/string_number_conversions.h"
#include "build/build_config.h"
#include "chrome/browser/bookmarks/bookmark_model_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/search/search.h"
#include "chrome/browser/ui/app_list/app_list_util.h"
#include "chrome/browser/ui/bookmarks/bookmark_editor.h"
#include "chrome/browser/ui/bookmarks/bookmark_utils.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_navigator.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/simple_message_box.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/url_constants.h"
#include "chrome/grit/chromium_strings.h"
#include "chrome/grit/generated_resources.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/browser/bookmark_node_data.h"
#include "components/bookmarks/browser/bookmark_utils.h"
#include "components/prefs/pref_service.h"
#include "components/search/search.h"
#include "components/url_formatter/url_formatter.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/web_contents.h"
#include "extensions/features/features.h"
#include "ui/base/dragdrop/drag_drop_types.h"
#include "ui/base/dragdrop/drop_target_event.h"
#include "ui/base/l10n/l10n_util.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "chrome/browser/extensions/api/commands/command_service.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/extension_set.h"
#endif

#if defined(TOOLKIT_VIEWS)
#include "ui/gfx/color_palette.h"
#include "ui/gfx/paint_vector_icon.h"
#endif

#if defined(OS_WIN)
#include "ui/base/material_design/material_design_controller.h"
#include "ui/base/resource/resource_bundle.h"
#endif

using bookmarks::BookmarkModel;
using bookmarks::BookmarkNode;

namespace chrome {

size_t kNumBookmarkUrlsBeforePrompting = 15;

namespace {

// Returns a vector of all URLs in |nodes| and their immediate children.  Only
// recurses one level deep, not infinitely.  TODO(pkasting): It's not clear why
// this shouldn't just recurse infinitely.
std::vector<GURL> GetURLsToOpen(
    const std::vector<const BookmarkNode*>& nodes,
    content::BrowserContext* browser_context = nullptr,
    bool incognito_urls_only = false) {
  std::vector<GURL> urls;

  const auto AddUrlIfLegal = [&](const GURL url) {
    if (!incognito_urls_only || IsURLAllowedInIncognito(url, browser_context))
      urls.push_back(url);
  };

  for (const BookmarkNode* node : nodes) {
    if (node->is_url()) {
      AddUrlIfLegal(node->url());
    } else {
      // If the node is not a URL, it is a folder. We want to add those of its
      // children which are URLs.
      for (int child_index = 0; child_index < node->child_count();
           ++child_index) {
        const BookmarkNode* child = node->GetChild(child_index);
        if (child->is_url())
          AddUrlIfLegal(child->url());
      }
    }
  }
  return urls;
}

#if !defined(OS_ANDROID)
bool ShouldOpenAll(gfx::NativeWindow parent,
                   const std::vector<const BookmarkNode*>& nodes) {
  size_t child_count = GetURLsToOpen(nodes).size();
  if (child_count < kNumBookmarkUrlsBeforePrompting)
    return true;

  return ShowQuestionMessageBox(
             parent, l10n_util::GetStringUTF16(IDS_PRODUCT_NAME),
             l10n_util::GetStringFUTF16(IDS_BOOKMARK_BAR_SHOULD_OPEN_ALL,
                                        base::SizeTToString16(child_count))) ==
         MESSAGE_BOX_RESULT_YES;
}
#endif

// Returns the total number of descendants nodes.
int ChildURLCountTotal(const BookmarkNode* node) {
  int result = 0;
  for (int i = 0; i < node->child_count(); ++i) {
    const BookmarkNode* child = node->GetChild(i);
    result++;
    if (child->is_folder())
      result += ChildURLCountTotal(child);
  }
  return result;
}

#if !defined(OS_ANDROID)
// Returns in |urls|, the url and title pairs for each open tab in browser.
void GetURLsForOpenTabs(Browser* browser,
                        std::vector<std::pair<GURL, base::string16> >* urls) {
  for (int i = 0; i < browser->tab_strip_model()->count(); ++i) {
    std::pair<GURL, base::string16> entry;
    GetURLAndTitleToBookmark(browser->tab_strip_model()->GetWebContentsAt(i),
                             &(entry.first), &(entry.second));
    urls->push_back(entry);
  }
}
#endif

}  // namespace

#if !defined(OS_ANDROID)
void OpenAll(gfx::NativeWindow parent,
             content::PageNavigator* navigator,
             const std::vector<const BookmarkNode*>& nodes,
             WindowOpenDisposition initial_disposition,
             content::BrowserContext* browser_context) {
  if (!ShouldOpenAll(parent, nodes))
    return;

  // Opens all |nodes| of type URL and any children of |nodes| that are of type
  // URL. |navigator| is the PageNavigator used to open URLs. After the first
  // url is opened |navigator| is set to the PageNavigator of the last active
  // tab. This is done to handle a window disposition of new window, in which
  // case we want subsequent tabs to open in that window.

  std::vector<GURL> urls = GetURLsToOpen(
      nodes, browser_context,
      initial_disposition == WindowOpenDisposition::OFF_THE_RECORD);

  WindowOpenDisposition disposition = initial_disposition;
  for (std::vector<GURL>::const_iterator url_it = urls.begin();
       url_it != urls.end(); ++url_it) {
    content::WebContents* opened_tab = navigator->OpenURL(
        content::OpenURLParams(*url_it, content::Referrer(), disposition,
                               ui::PAGE_TRANSITION_AUTO_BOOKMARK, false));
    if (url_it == urls.begin()) {
      // We opened the first URL which may have opened a new window or clobbered
      // the current page, reset the navigator just to be sure. |opened_tab| may
      // be null in tests.
      if (opened_tab)
        navigator = opened_tab;
      disposition = WindowOpenDisposition::NEW_BACKGROUND_TAB;
    }
  }
}

void OpenAll(gfx::NativeWindow parent,
             content::PageNavigator* navigator,
             const BookmarkNode* node,
             WindowOpenDisposition initial_disposition,
             content::BrowserContext* browser_context) {
  std::vector<const BookmarkNode*> nodes;
  nodes.push_back(node);
  OpenAll(parent, navigator, std::vector<const bookmarks::BookmarkNode*>{node},
          initial_disposition, browser_context);
}

int OpenCount(gfx::NativeWindow parent,
              const std::vector<const bookmarks::BookmarkNode*>& nodes,
              content::BrowserContext* incognito_context) {
  return GetURLsToOpen(nodes, incognito_context, incognito_context != nullptr)
      .size();
}

int OpenCount(gfx::NativeWindow parent,
              const BookmarkNode* node,
              content::BrowserContext* incognito_context) {
  std::vector<const BookmarkNode*> nodes;
  nodes.push_back(node);
  return OpenCount(parent, std::vector<const bookmarks::BookmarkNode*>{node},
                   incognito_context);
}

bool ConfirmDeleteBookmarkNode(const BookmarkNode* node,
                               gfx::NativeWindow window) {
  DCHECK(node && node->is_folder() && !node->empty());
  return ShowQuestionMessageBox(
             window, l10n_util::GetStringUTF16(IDS_PRODUCT_NAME),
             l10n_util::GetPluralStringFUTF16(
                 IDS_BOOKMARK_EDITOR_CONFIRM_DELETE,
                 ChildURLCountTotal(node))) ==
         MESSAGE_BOX_RESULT_YES;
}

void ShowBookmarkAllTabsDialog(Browser* browser) {
  Profile* profile = browser->profile();
  BookmarkModel* model = BookmarkModelFactory::GetForBrowserContext(profile);
  DCHECK(model && model->loaded());

  const BookmarkNode* parent = GetParentForNewNodes(model);
  BookmarkEditor::EditDetails details =
      BookmarkEditor::EditDetails::AddFolder(parent, parent->child_count());
  GetURLsForOpenTabs(browser, &(details.urls));
  DCHECK(!details.urls.empty());

  BookmarkEditor::Show(browser->window()->GetNativeWindow(), profile, details,
                       BookmarkEditor::SHOW_TREE);
}

bool HasBookmarkURLs(const std::vector<const BookmarkNode*>& selection) {
  return !GetURLsToOpen(selection).empty();
}

bool HasBookmarkURLsAllowedInIncognitoMode(
    const std::vector<const BookmarkNode*>& selection,
    content::BrowserContext* browser_context) {
  return !GetURLsToOpen(selection, browser_context, true).empty();
}
#endif  // !defined(OS_ANDROID)

}  // namespace chrome
