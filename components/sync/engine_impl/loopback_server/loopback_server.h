// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SYNC_ENGINE_IMPL_LOOPBACK_SERVER_LOOPBACK_SERVER_H_
#define COMPONENTS_SYNC_ENGINE_IMPL_LOOPBACK_SERVER_LOOPBACK_SERVER_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/threading/thread_checker.h"
#include "base/values.h"
#include "components/sync/base/model_type.h"
#include "components/sync/engine_impl/loopback_server/loopback_server_entity.h"
#include "components/sync/engine_impl/net/server_connection_manager.h"
#include "components/sync/protocol/loopback_server.pb.h"
#include "components/sync/protocol/sync.pb.h"

namespace fake_server {
class FakeServer;
}

namespace syncer {

// A loopback version of the Sync server used for local profile serialization.
class LoopbackServer {
 public:
  class ObserverForTests {
   public:
    virtual ~ObserverForTests() {}

    // Called after the server has processed a successful commit. The types
    // updated as part of the commit are passed in |committed_model_types|.
    virtual void OnCommit(const std::string& committer_id,
                          syncer::ModelTypeSet committed_model_types) = 0;
  };

  explicit LoopbackServer(const base::FilePath& persistent_file);
  virtual ~LoopbackServer();

  // Handles a /command POST (with the given |request|) to the server. Three
  // output arguments, |server_status|, |response_code|, and |response|, are
  // used to pass data back to the caller. The command has failed if the value
  // pointed to by |error_code| is nonzero.
  void HandleCommand(const std::string& request,
                     HttpResponse::ServerConnectionCode* server_status,
                     int64_t* response_code,
                     std::string* response);

 private:
  // Allow the FakeServer decorator to inspect the internals of this class.
  friend class fake_server::FakeServer;

  using EntityMap =
      std::map<std::string, std::unique_ptr<LoopbackServerEntity>>;

  using ResponseTypeProvider =
      base::RepeatingCallback<sync_pb::CommitResponse::ResponseType(
          const LoopbackServerEntity& entity)>;

  // Gets LoopbackServer ready for syncing.
  void Init();

  // Processes a GetUpdates call.
  bool HandleGetUpdatesRequest(const sync_pb::GetUpdatesMessage& get_updates,
                               sync_pb::GetUpdatesResponse* response);

  // Processes a Commit call.
  bool HandleCommitRequest(const sync_pb::CommitMessage& message,
                           const std::string& invalidator_client_id,
                           sync_pb::CommitResponse* response);

  void ClearServerData();

  // Creates and saves a permanent folder for Bookmarks (e.g., Bookmark Bar).
  bool CreatePermanentBookmarkFolder(const std::string& server_tag,
                                     const std::string& name);

  // Inserts the default permanent items in |entities_|.
  bool CreateDefaultPermanentItems();

  std::string GenerateNewKeystoreKey() const;

  // Saves a |entity| to |entities_|.
  void SaveEntity(std::unique_ptr<LoopbackServerEntity> entity);

  // Commits a client-side SyncEntity to the server as a LoopbackServerEntity.
  // The client that sent the commit is identified via |client_guid|. The
  // parent ID string present in |client_entity| should be ignored in favor
  // of |parent_id|. If the commit is successful, the entity's server ID string
  // is returned and a new LoopbackServerEntity is saved in |entities_|.
  std::string CommitEntity(
      const sync_pb::SyncEntity& client_entity,
      sync_pb::CommitResponse_EntryResponse* entry_response,
      const std::string& client_guid,
      const std::string& parent_id);

  // Populates |entry_response| based on the stored entity identified by
  // |entity_id|. It is assumed that the entity identified by |entity_id| has
  // already been stored using SaveEntity.
  void BuildEntryResponseForSuccessfulCommit(
      const std::string& entity_id,
      sync_pb::CommitResponse_EntryResponse* entry_response);

  // Determines whether the SyncEntity with id_string |id| is a child of an
  // entity with id_string |potential_parent_id|.
  bool IsChild(const std::string& id, const std::string& potential_parent_id);

  // Creates and saves tombstones for all children of the entity with the given
  // |id|. A tombstone is not created for the entity itself.
  void DeleteChildren(const std::string& id);

  // Updates the |entity| to a new version and increments the version counter
  // that the server uses to assign versions.
  void UpdateEntityVersion(LoopbackServerEntity* entity);

  // Returns the store birthday.
  std::string GetStoreBirthday() const;

  // Returns all entities stored by the server of the given |model_type|.
  // This method is only used in tests.
  std::vector<sync_pb::SyncEntity> GetSyncEntitiesByModelType(
      syncer::ModelType model_type);

  // Creates a DicionaryValue representation of all entities present in the
  // server. The dictionary keys are the strings generated by ModelTypeToString
  // and the values are ListValues containing StringValue versions of entity
  // names. Used by test to verify the contents of the server state.
  std::unique_ptr<base::DictionaryValue> GetEntitiesAsDictionaryValue();

  // Modifies the entity on the server with the given |id|. The entity's
  // EntitySpecifics are replaced with |updated_specifics| and its version is
  // updated to n+1. If the given |id| does not exist or the ModelType of
  // |updated_specifics| does not match the entity, false is returned.
  // Otherwise, true is returned to represent a successful modification.
  //
  // This method sometimes updates entity data beyond EntitySpecifics. For
  // example, in the case of a bookmark, changing the BookmarkSpecifics title
  // field will modify the top-level entity's name field.
  // This method is only used in tests.
  bool ModifyEntitySpecifics(const std::string& id,
                             const sync_pb::EntitySpecifics& updated_specifics);

  // This method is only used in tests.
  bool ModifyBookmarkEntity(const std::string& id,
                            const std::string& parent_id,
                            const sync_pb::EntitySpecifics& updated_specifics);

  // Use this callback to generate response types for entities. They will still
  // be "committed" and stored as normal, this only affects the response type
  // the client sees. This allows tests to still inspect what the client has
  // done, although not as useful of a mechanism for multi client tests. Care
  // should be taken when failing responses, as the client will go into
  // exponential backoff, which can cause tests to be slow or time out.
  // This method is only used in tests.
  void OverrideResponseType(ResponseTypeProvider response_type_override);

  // Serializes the server state to |proto|.
  void SerializeState(sync_pb::LoopbackServerProto* proto) const;

  // Populates the server state from |proto|. Returns true iff successful.
  bool DeSerializeState(const sync_pb::LoopbackServerProto& proto);

  // Saves all entities and server state to a protobuf file in |filename|.
  bool SaveStateToFile(const base::FilePath& filename) const;

  // Loads all entities and server state from a protobuf file in |filename|.
  bool LoadStateFromFile(const base::FilePath& filename);

  void set_observer_for_tests(ObserverForTests* observer) {
    observer_for_tests_ = observer;
  }

  // This is the last version number assigned to an entity. The next entity will
  // have a version number of version_ + 1.
  int64_t version_;

  int64_t store_birthday_;

  EntityMap entities_;
  std::vector<std::string> keystore_keys_;

  // The file used to store the local sync data.
  base::FilePath persistent_file_;

  // Used to verify that LoopbackServer is only used from one thread.
  base::ThreadChecker thread_checker_;

  // Used to observe the completion of commit messages for the sake of testing.
  ObserverForTests* observer_for_tests_;

  // Response type override callback used in tests.
  ResponseTypeProvider response_type_override_;
};

}  // namespace syncer

#endif  // COMPONENTS_SYNC_ENGINE_IMPL_LOOPBACK_SERVER_LOOPBACK_SERVER_H_
