// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/bookmarks/managed/managed_bookmark_service.h"

#include <stdint.h>
#include <stdlib.h>

#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string16.h"
#include "base/values.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/browser/bookmark_utils.h"
#include "components/bookmarks/managed/managed_bookmarks_tracker.h"
#include "components/strings/grit/components_strings.h"
#include "ui/base/l10n/l10n_util.h"

namespace bookmarks {
namespace {

// BookmarkPermanentNodeLoader initializes a BookmarkPermanentNode from a JSON
// representation, title id and starting node id.
class BookmarkPermanentNodeLoader {
 public:
  BookmarkPermanentNodeLoader(
      std::unique_ptr<BookmarkPermanentNode> node,
      std::unique_ptr<base::ListValue> initial_bookmarks,
      int title_id)
      : node_(std::move(node)),
        initial_bookmarks_(std::move(initial_bookmarks)),
        title_id_(title_id) {
    DCHECK(node_);
  }

  ~BookmarkPermanentNodeLoader() {}

  // Initializes |node_| from |initial_bookmarks_| and |title_id_| and returns
  // it. The ids are assigned starting at |next_node_id| and the value is
  // updated as a side-effect.
  std::unique_ptr<BookmarkPermanentNode> Load(int64_t* next_node_id) {
    node_->set_id(*next_node_id);
    *next_node_id = ManagedBookmarksTracker::LoadInitial(
        node_.get(), initial_bookmarks_.get(), node_->id() + 1);
    node_->set_visible(!node_->empty());
    node_->SetTitle(l10n_util::GetStringUTF16(title_id_));
    return std::move(node_);
  }

 private:
  std::unique_ptr<BookmarkPermanentNode> node_;
  std::unique_ptr<base::ListValue> initial_bookmarks_;
  int title_id_;

  DISALLOW_COPY_AND_ASSIGN(BookmarkPermanentNodeLoader);
};

// Returns a list of initialized BookmarkPermanentNodes using |next_node_id| to
// start assigning id. |next_node_id| is updated as a side effect of calling
// this method.
BookmarkPermanentNodeList LoadExtraNodes(
    std::vector<std::unique_ptr<BookmarkPermanentNodeLoader>> loaders,
    int64_t* next_node_id) {
  BookmarkPermanentNodeList extra_nodes;
  for (auto& loader : loaders)
    extra_nodes.push_back(loader->Load(next_node_id));
  return extra_nodes;
}

}  // namespace

ManagedBookmarkService::ManagedBookmarkService(
    PrefService* prefs,
    const GetManagementDomainCallback& callback)
    : prefs_(prefs),
      bookmark_model_(nullptr),
      managed_domain_callback_(callback),
      managed_node_(nullptr),
      supervised_node_(nullptr) {}

ManagedBookmarkService::~ManagedBookmarkService() {
  DCHECK(!bookmark_model_);
}

void ManagedBookmarkService::BookmarkModelCreated(
    BookmarkModel* bookmark_model) {
  DCHECK(bookmark_model);
  DCHECK(!bookmark_model_);
  bookmark_model_ = bookmark_model;
  bookmark_model_->AddObserver(this);

  managed_bookmarks_tracker_.reset(new ManagedBookmarksTracker(
      bookmark_model_, prefs_, false /* is_supervised */,
      managed_domain_callback_));

  GetManagementDomainCallback unbound_callback;
  supervised_bookmarks_tracker_.reset(new ManagedBookmarksTracker(
      bookmark_model_, prefs_, true /* is_supervised */, unbound_callback));
}

LoadExtraCallback ManagedBookmarkService::GetLoadExtraNodesCallback() {
  // Create two BookmarkPermanentNode with a temporary id of 0. They will be
  // populated and assigned proper ids in the LoadExtraNodes callback. Until
  // then, they are owned by the returned closure.
  std::unique_ptr<BookmarkPermanentNode> managed(new BookmarkPermanentNode(0));
  std::unique_ptr<BookmarkPermanentNode> supervised(
      new BookmarkPermanentNode(0));

  managed_node_ = managed.get();
  supervised_node_ = supervised.get();

  std::vector<std::unique_ptr<BookmarkPermanentNodeLoader>> loaders;
  loaders.push_back(base::MakeUnique<BookmarkPermanentNodeLoader>(
      std::move(managed),
      managed_bookmarks_tracker_->GetInitialManagedBookmarks(),
      IDS_BOOKMARK_BAR_MANAGED_FOLDER_DEFAULT_NAME));
  loaders.push_back(base::MakeUnique<BookmarkPermanentNodeLoader>(
      std::move(supervised),
      supervised_bookmarks_tracker_->GetInitialManagedBookmarks(),
      IDS_BOOKMARK_BAR_SUPERVISED_FOLDER_DEFAULT_NAME));

  return base::Bind(&LoadExtraNodes, base::Passed(&loaders));
}

bool ManagedBookmarkService::CanSetPermanentNodeTitle(
    const BookmarkNode* node) {
  // |managed_node_| can have its title updated if the user signs in or out,
  // since the name of the managed domain can appear in it. Also both
  // |managed_node_| and |supervised_node_| can have their title updated on
  // locale changes (http://crbug.com/459448).
  if (node == managed_node_ || node == supervised_node_)
    return true;
  return !IsDescendantOf(node, managed_node_) &&
         !IsDescendantOf(node, supervised_node_);
}

bool ManagedBookmarkService::CanSyncNode(const BookmarkNode* node) {
  return !IsDescendantOf(node, managed_node_) &&
         !IsDescendantOf(node, supervised_node_);
}

bool ManagedBookmarkService::CanBeEditedByUser(const BookmarkNode* node) {
  return !IsDescendantOf(node, managed_node_) &&
         !IsDescendantOf(node, supervised_node_);
}

void ManagedBookmarkService::Shutdown() {
  Cleanup();
}

void ManagedBookmarkService::BookmarkModelChanged() {}

void ManagedBookmarkService::BookmarkModelLoaded(BookmarkModel* bookmark_model,
                                                 bool ids_reassigned) {
  BaseBookmarkModelObserver::BookmarkModelLoaded(bookmark_model,
                                                 ids_reassigned);
  // Start tracking the managed and supervised bookmarks. This will detect any
  // changes that may have occurred while the initial managed and supervised
  // bookmarks were being loaded on the background.
  managed_bookmarks_tracker_->Init(managed_node_);
  supervised_bookmarks_tracker_->Init(supervised_node_);
}

void ManagedBookmarkService::BookmarkModelBeingDeleted(
    BookmarkModel* bookmark_model) {
  Cleanup();
}

void ManagedBookmarkService::Cleanup() {
  if (bookmark_model_) {
    bookmark_model_->RemoveObserver(this);
    bookmark_model_ = nullptr;
  }

  managed_bookmarks_tracker_.reset();
  supervised_bookmarks_tracker_.reset();

  managed_node_ = nullptr;
  supervised_node_ = nullptr;
}

}  // namespace bookmarks
