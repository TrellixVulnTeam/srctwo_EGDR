// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/engine_impl/syncer.h"

#include <memory>

#include "base/auto_reset.h"
#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "build/build_config.h"
#include "components/sync/base/cancelation_signal.h"
#include "components/sync/engine_impl/apply_control_data_updates.h"
#include "components/sync/engine_impl/clear_server_data.h"
#include "components/sync/engine_impl/commit.h"
#include "components/sync/engine_impl/commit_processor.h"
#include "components/sync/engine_impl/cycle/nudge_tracker.h"
#include "components/sync/engine_impl/cycle/sync_cycle.h"
#include "components/sync/engine_impl/get_updates_delegate.h"
#include "components/sync/engine_impl/get_updates_processor.h"
#include "components/sync/engine_impl/net/server_connection_manager.h"
#include "components/sync/syncable/directory.h"

namespace syncer {

namespace {

// TODO(akalin): We may want to propagate this switch up eventually.
#if defined(OS_ANDROID) || defined(OS_IOS)
static const bool kCreateMobileBookmarksFolder = true;
#else
static const bool kCreateMobileBookmarksFolder = false;
#endif

void HandleCycleBegin(SyncCycle* cycle) {
  cycle->mutable_status_controller()->UpdateStartTime();
  cycle->SendEventNotification(SyncCycleEvent::SYNC_CYCLE_BEGIN);
}

}  // namespace

Syncer::Syncer(CancelationSignal* cancelation_signal)
    : cancelation_signal_(cancelation_signal), is_syncing_(false) {}

Syncer::~Syncer() {}

bool Syncer::IsSyncing() const {
  return is_syncing_;
}

bool Syncer::NormalSyncShare(ModelTypeSet request_types,
                             NudgeTracker* nudge_tracker,
                             SyncCycle* cycle) {
  base::AutoReset<bool> is_syncing(&is_syncing_, true);
  HandleCycleBegin(cycle);
  if (nudge_tracker->IsGetUpdatesRequired() ||
      cycle->context()->ShouldFetchUpdatesBeforeCommit()) {
    VLOG(1) << "Downloading types " << ModelTypeSetToString(request_types);
    if (!DownloadAndApplyUpdates(&request_types, cycle,
                                 NormalGetUpdatesDelegate(*nudge_tracker),
                                 kCreateMobileBookmarksFolder)) {
      return HandleCycleEnd(cycle, nudge_tracker->GetLegacySource());
    }
  }

  CommitProcessor commit_processor(
      cycle->context()->model_type_registry()->commit_contributor_map());
  SyncerError commit_result = BuildAndPostCommits(request_types, nudge_tracker,
                                                  cycle, &commit_processor);
  cycle->mutable_status_controller()->set_commit_result(commit_result);

  return HandleCycleEnd(cycle, nudge_tracker->GetLegacySource());
}

bool Syncer::ConfigureSyncShare(
    const ModelTypeSet& request_types,
    sync_pb::GetUpdatesCallerInfo::GetUpdatesSource source,
    SyncCycle* cycle) {
  base::AutoReset<bool> is_syncing(&is_syncing_, true);

  // It is possible during configuration that datatypes get unregistered from
  // ModelTypeRegistry before scheduled configure sync cycle gets executed.
  // This happens either because DataTypeController::LoadModels fail and type
  // need to be stopped or during shutdown when all datatypes are stopped. When
  // it happens we should adjust set of types to download to only include
  // registered types.
  ModelTypeSet still_enabled_types =
      Intersection(request_types, cycle->context()->GetEnabledTypes());
  VLOG(1) << "Configuring types " << ModelTypeSetToString(still_enabled_types);
  HandleCycleBegin(cycle);
  DownloadAndApplyUpdates(&still_enabled_types, cycle,
                          ConfigureGetUpdatesDelegate(source),
                          kCreateMobileBookmarksFolder);
  return HandleCycleEnd(cycle, source);
}

bool Syncer::PollSyncShare(ModelTypeSet request_types, SyncCycle* cycle) {
  base::AutoReset<bool> is_syncing(&is_syncing_, true);
  VLOG(1) << "Polling types " << ModelTypeSetToString(request_types);
  HandleCycleBegin(cycle);
  DownloadAndApplyUpdates(&request_types, cycle, PollGetUpdatesDelegate(),
                          kCreateMobileBookmarksFolder);
  return HandleCycleEnd(cycle, sync_pb::GetUpdatesCallerInfo::PERIODIC);
}

bool Syncer::PostClearServerData(SyncCycle* cycle) {
  DCHECK(cycle);
  ClearServerData clear_server_data(cycle->context()->account_name());
  return clear_server_data.SendRequest(cycle) == SYNCER_OK;
}

bool Syncer::DownloadAndApplyUpdates(ModelTypeSet* request_types,
                                     SyncCycle* cycle,
                                     const GetUpdatesDelegate& delegate,
                                     bool create_mobile_bookmarks_folder) {
  // CommitOnlyTypes() should not be included in the GetUpdates, but should be
  // included in the Commit. We are given a set of types for our SyncShare,
  // and we must do this filtering. Note that |request_types| is also an out
  // param, see below where we update it.
  ModelTypeSet requested_commit_only_types =
      Intersection(*request_types, CommitOnlyTypes());
  ModelTypeSet download_types =
      Difference(*request_types, requested_commit_only_types);
  GetUpdatesProcessor get_updates_processor(
      cycle->context()->model_type_registry()->update_handler_map(), delegate);
  SyncerError download_result = UNSET;
  do {
    download_result = get_updates_processor.DownloadUpdates(
        &download_types, cycle, create_mobile_bookmarks_folder);
  } while (download_result == SERVER_MORE_TO_DOWNLOAD);

  // It is our responsibility to propagate the removal of types that occurred in
  // GetUpdatesProcessor::DownloadUpdates().
  *request_types = Union(download_types, requested_commit_only_types);

  // Exit without applying if we're shutting down or an error was detected.
  if (download_result != SYNCER_OK || ExitRequested())
    return false;

  {
    TRACE_EVENT0("sync", "ApplyUpdates");

    // Control type updates always get applied first.
    ApplyControlDataUpdates(cycle->context()->directory());

    // Apply updates to the other types. May or may not involve cross-thread
    // traffic, depending on the underlying update handlers and the GU type's
    // delegate.
    get_updates_processor.ApplyUpdates(download_types,
                                       cycle->mutable_status_controller());

    cycle->context()->set_hierarchy_conflict_detected(
        cycle->status_controller().num_hierarchy_conflicts() > 0);
    cycle->SendEventNotification(SyncCycleEvent::STATUS_CHANGED);
  }

  return !ExitRequested();
}

SyncerError Syncer::BuildAndPostCommits(const ModelTypeSet& request_types,
                                        NudgeTracker* nudge_tracker,
                                        SyncCycle* cycle,
                                        CommitProcessor* commit_processor) {
  VLOG(1) << "Committing from types " << ModelTypeSetToString(request_types);

  // The ExitRequested() check is unnecessary, since we should start getting
  // errors from the ServerConnectionManager if an exist has been requested.
  // However, it doesn't hurt to check it anyway.
  while (!ExitRequested()) {
    std::unique_ptr<Commit> commit(
        Commit::Init(request_types, cycle->context()->GetEnabledTypes(),
                     cycle->context()->max_commit_batch_size(),
                     cycle->context()->account_name(),
                     cycle->context()->directory()->cache_guid(),
                     cycle->context()->cookie_jar_mismatch(),
                     cycle->context()->cookie_jar_empty(), commit_processor,
                     cycle->context()->extensions_activity()));
    if (!commit) {
      break;
    }

    SyncerError error = commit->PostAndProcessResponse(
        nudge_tracker, cycle, cycle->mutable_status_controller(),
        cycle->context()->extensions_activity());
    commit->CleanUp();
    if (error != SYNCER_OK) {
      return error;
    }
  }

  return SYNCER_OK;
}

bool Syncer::ExitRequested() {
  return cancelation_signal_->IsSignalled();
}

bool Syncer::HandleCycleEnd(
    SyncCycle* cycle,
    sync_pb::GetUpdatesCallerInfo::GetUpdatesSource source) {
  if (ExitRequested())
    return false;

  cycle->SendSyncCycleEndEventNotification(source);
  bool success =
      !HasSyncerError(cycle->status_controller().model_neutral_state());
  if (success && source == sync_pb::GetUpdatesCallerInfo::PERIODIC) {
    cycle->mutable_status_controller()->UpdatePollTime();
  }

  return success;
}

}  // namespace syncer
