// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_COMMAND_BUFFER_SERVICE_QUERY_MANAGER_H_
#define GPU_COMMAND_BUFFER_SERVICE_QUERY_MANAGER_H_

#include <stdint.h>

#include <deque>
#include <memory>
#include <vector>

#include "base/atomicops.h"
#include "base/containers/hash_tables.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "gpu/command_buffer/service/feature_info.h"
#include "gpu/gpu_export.h"

namespace gl {
class GPUTimer;
class GPUTimingClient;
}

namespace gpu {

class GLES2Decoder;

namespace gles2 {

class FeatureInfo;

// This class keeps track of the queries and their state
// As Queries are not shared there is one QueryManager per context.
class GPU_EXPORT QueryManager {
 public:
  class GPU_EXPORT Query : public base::RefCounted<Query> {
   public:
    Query(QueryManager* manager,
          GLenum target,
          scoped_refptr<gpu::Buffer> buffer,
          QuerySync* sync);

    GLenum target() const {
      return target_;
    }

    bool IsDeleted() const {
      return deleted_;
    }

    bool IsValid() const {
      return target() && !IsDeleted();
    }

    bool IsActive() const {
      return query_state_ == kQueryState_Active;
    }

    bool IsPaused() const {
      return query_state_ == kQueryState_Paused;
    }

    bool IsPending() const {
      return query_state_ == kQueryState_Pending;
    }

    bool IsFinished() const {
      return query_state_ == kQueryState_Finished;
    }

    const QuerySync* sync() const { return sync_; }

    virtual void Begin() = 0;

    virtual void End(base::subtle::Atomic32 submit_count) = 0;

    virtual void QueryCounter(base::subtle::Atomic32 submit_count) = 0;

    virtual void Process(bool did_finish) = 0;

    // Pauses active query to be resumed later.
    virtual void Pause() = 0;

    // Resume from a paused active query.
    virtual void Resume() = 0;

    virtual void Destroy(bool have_context) = 0;

    void AddCallback(base::Closure callback);

   protected:
    virtual ~Query();

    QueryManager* manager() const {
      return manager_;
    }

    void MarkAsDeleted() {
      deleted_ = true;
    }

    void MarkAsActive() {
      DCHECK(query_state_ == kQueryState_Initialize ||
             query_state_ == kQueryState_Paused ||
             query_state_ == kQueryState_Finished);
      query_state_ = kQueryState_Active;
    }

    void MarkAsPaused() {
      DCHECK(query_state_ == kQueryState_Active);
      query_state_ = kQueryState_Paused;
    }

    void MarkAsPending(base::subtle::Atomic32 submit_count) {
      DCHECK(query_state_ == kQueryState_Active);
      query_state_ = kQueryState_Pending;
      submit_count_ = submit_count;
    }

    void MarkAsCompleted(uint64_t result);

    void UnmarkAsPending() {
      DCHECK(query_state_ == kQueryState_Pending);
      query_state_ = kQueryState_Finished;
    }

    void AddToPendingQueue(base::subtle::Atomic32 submit_count) {
      manager_->AddPendingQuery(this, submit_count);
    }

    void BeginQueryHelper(GLenum target, GLuint id) {
      manager_->BeginQueryHelper(target, id);
    }

    void EndQueryHelper(GLenum target) {
      manager_->EndQueryHelper(target);
    }

    void SafelyResetDisjointValue() {
      manager_->SafelyResetDisjointValue();
    }

    void UpdateDisjointValue() {
      manager_->UpdateDisjointValue();
    }

    void BeginContinualDisjointUpdate() {
      manager_->update_disjoints_continually_ = true;
    }

    base::subtle::Atomic32 submit_count() const { return submit_count_; }

   private:
    friend class QueryManager;
    friend class QueryManagerTest;
    friend class base::RefCounted<Query>;

    void RunCallbacks();

    // The manager that owns this Query.
    QueryManager* manager_;

    // The type of query.
    GLenum target_;

    // The shared memory used with this Query. We keep a reference to the Buffer
    // to ensure it doesn't get released until we wrote results. sync_ points to
    // memory inside buffer_.
    scoped_refptr<gpu::Buffer> buffer_;
    QuerySync* sync_;

    // Count to set process count do when completed.
    base::subtle::Atomic32 submit_count_;

    // Current state of the query.
    enum QueryState {
      kQueryState_Initialize, // Has not been queried yet.
      kQueryState_Active, // Query began but has not ended.
      kQueryState_Paused, // Query was active but is now paused.
      kQueryState_Pending, // Query ended, waiting for result.
      kQueryState_Finished, // Query received result.
    } query_state_;

    // True if deleted.
    bool deleted_;

    // List of callbacks to run when result is available.
    std::vector<base::Closure> callbacks_;
  };

  QueryManager(
      GLES2Decoder* decoder,
      FeatureInfo* feature_info);
  ~QueryManager();

  // Must call before destruction.
  void Destroy(bool have_context);

  // Sets up a location to be incremented whenever a disjoint is detected.
  error::Error SetDisjointSync(int32_t shm_id, uint32_t shm_offset);

  // Creates a Query for the given query.
  Query* CreateQuery(GLenum target,
                     GLuint client_id,
                     scoped_refptr<gpu::Buffer> buffer,
                     QuerySync* sync);

  // Gets the query info for the given query.
  Query* GetQuery(GLuint client_id);

  // Gets the currently active query for a target.
  Query* GetActiveQuery(GLenum target);

  // Removes a query info for the given query.
  void RemoveQuery(GLuint client_id);

  // Returns false if any query is pointing to invalid shared memory.
  void BeginQuery(Query* query);

  // Returns false if any query is pointing to invalid shared memory.
  void EndQuery(Query* query, base::subtle::Atomic32 submit_count);

  // Returns false if any query is pointing to invalid shared memory.
  void QueryCounter(Query* query, base::subtle::Atomic32 submit_count);

  void PauseQueries();
  void ResumeQueries();

  // Processes pending queries. Returns false if any queries are pointing
  // to invalid shared memory. |did_finish| is true if this is called as
  // a result of calling glFinish().
  void ProcessPendingQueries(bool did_finish);

  // True if there are pending queries.
  bool HavePendingQueries();

  // Do any updates we need to do when the frame has begun.
  void ProcessFrameBeginUpdates();

  GLES2Decoder* decoder() const {
    return decoder_;
  }

  std::unique_ptr<gl::GPUTimer> CreateGPUTimer(bool elapsed_time);
  bool GPUTimingAvailable();

  void GenQueries(GLsizei n, const GLuint* queries);
  bool IsValidQuery(GLuint id);

 private:
  void StartTracking(Query* query);
  void StopTracking(Query* query);

  // Wrappers for BeginQueryARB and EndQueryARB to hide differences between
  // ARB_occlusion_query2 and EXT_occlusion_query_boolean.
  void BeginQueryHelper(GLenum target, GLuint id);
  void EndQueryHelper(GLenum target);

  // Adds to queue of queries waiting for completion.
  // Returns false if any query is pointing to invalid shared memory.
  void AddPendingQuery(Query* query, base::subtle::Atomic32 submit_count);

  // Removes a query from the queue of pending queries.
  // Returns false if any query is pointing to invalid shared memory.
  void RemovePendingQuery(Query* query);

  // Returns a target used for the underlying GL extension
  // used to emulate a query.
  GLenum AdjustTargetForEmulation(GLenum target);

  // Checks and notifies if a disjoint occurred.
  void UpdateDisjointValue();

  // Safely resets the disjoint value if no queries are active.
  void SafelyResetDisjointValue();

  // Used to validate shared memory and get GL errors.
  GLES2Decoder* decoder_;

  bool use_arb_occlusion_query2_for_occlusion_query_boolean_;
  bool use_arb_occlusion_query_for_occlusion_query_boolean_;

  // Whether we are tracking disjoint values every frame.
  bool update_disjoints_continually_;

  // The shared memory used for disjoint notifications.
  int32_t disjoint_notify_shm_id_;
  uint32_t disjoint_notify_shm_offset_;

  // Current number of disjoints notified.
  uint32_t disjoints_notified_;

  // Counts the number of Queries allocated with 'this' as their manager.
  // Allows checking no Query will outlive this.
  unsigned query_count_;

  // Info for each query in the system.
  typedef base::hash_map<GLuint, scoped_refptr<Query> > QueryMap;
  QueryMap queries_;

  typedef base::hash_set<GLuint> GeneratedQueryIds;
  GeneratedQueryIds generated_query_ids_;

  // A map of targets -> Query for current active queries.
  typedef std::map<GLenum, scoped_refptr<Query> > ActiveQueryMap;
  ActiveQueryMap active_queries_;

  // Queries waiting for completion.
  typedef std::deque<scoped_refptr<Query> > QueryQueue;
  QueryQueue pending_queries_;

  scoped_refptr<gl::GPUTimingClient> gpu_timing_client_;

  DISALLOW_COPY_AND_ASSIGN(QueryManager);
};

}  // namespace gles2
}  // namespace gpu

#endif  // GPU_COMMAND_BUFFER_SERVICE_QUERY_MANAGER_H_
