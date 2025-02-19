// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_CERTIFICATE_TRANSPARENCY_SINGLE_TREE_TRACKER_H_
#define COMPONENTS_CERTIFICATE_TRANSPARENCY_SINGLE_TREE_TRACKER_H_

#include <map>
#include <memory>
#include <string>

#include "base/containers/mru_cache.h"
#include "base/memory/memory_pressure_monitor.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "net/base/hash_value.h"
#include "net/base/network_change_notifier.h"
#include "net/cert/ct_verifier.h"
#include "net/cert/signed_tree_head.h"
#include "net/cert/sth_observer.h"
#include "net/log/net_log_with_source.h"

namespace net {

class CTLogVerifier;
class X509Certificate;

namespace ct {

struct MerkleAuditProof;
struct SignedCertificateTimestamp;

}  // namespace ct

}  // namespace net

namespace certificate_transparency {

class LogDnsClient;

// Tracks the state of an individual Certificate Transparency Log's Merkle Tree.
// A CT Log constantly issues Signed Tree Heads, for which every older STH must
// be incorporated into the current/newer STH. As new certificates are logged,
// new SCTs are produced, and eventually, those SCTs are incorporated into the
// log and a new STH is produced, with there being an inclusion proof between
// the SCTs and the new STH, and a consistency proof between the old STH and the
// new STH.
// This class receives STHs provided by/observed by the embedder, with the
// assumption that STHs have been checked for consistency already. As SCTs are
// observed, their status is checked against the latest STH to ensure they were
// properly logged. If an SCT is newer than the latest STH, then this class
// verifies that when an STH is observed that should have incorporated those
// SCTs, the SCTs (and their corresponding entries) are present in the log.
//
// To accomplish this, this class needs to be notified of when new SCTs are
// observed (which it does by implementing net::CTVerifier::Observer) and when
// new STHs are observed (which it does by implementing net::ct::STHObserver).
// Once connected to sources providing that data, the status for a given SCT
// can be queried by calling GetLogEntryInclusionCheck.
class SingleTreeTracker : public net::CTVerifier::Observer,
                          public net::ct::STHObserver {
 public:
  enum SCTInclusionStatus {
    // SCT was not observed by this class and is not currently pending
    // inclusion check. As there's no evidence the SCT this status relates
    // to is verified (it was never observed via OnSCTVerified), nothing
    // is done with it.
    SCT_NOT_OBSERVED,

    // SCT was observed but the STH known to this class is not old
    // enough to check for inclusion, so a newer STH is needed first.
    SCT_PENDING_NEWER_STH,

    // SCT is known and there's a new-enough STH to check inclusion against.
    // It's in the process of being checked for inclusion.
    SCT_PENDING_INCLUSION_CHECK,

    // Inclusion check succeeded.
    SCT_INCLUDED_IN_LOG,
  };

  SingleTreeTracker(scoped_refptr<const net::CTLogVerifier> ct_log,
                    LogDnsClient* dns_client,
                    net::NetLog* net_log);
  ~SingleTreeTracker() override;

  // net::ct::CTVerifier::Observer implementation.

  // TODO(eranm): Extract CTVerifier::Observer to SCTObserver
  // Performs an inclusion check for the given certificate if the latest
  // STH known for this log is older than sct.timestamp + Maximum Merge Delay,
  // enqueues the SCT for future checking later on.
  // Should only be called with SCTs issued by the log this instance tracks.
  // TODO(eranm): Make sure not to perform any synchronous, blocking operation
  // here as this callback is invoked during certificate validation.
  void OnSCTVerified(net::X509Certificate* cert,
                     const net::ct::SignedCertificateTimestamp* sct) override;

  // net::ct::STHObserver implementation.
  // After verification of the signature over the |sth|, uses this
  // STH for future inclusion checks.
  // Must only be called for STHs issued by the log this instance tracks.
  void NewSTHObserved(const net::ct::SignedTreeHead& sth) override;

  // Returns the status of a given log entry that is assembled from
  // |cert| and |sct|. If |cert| and |sct| were not previously observed,
  // |sct| is not an SCT for |cert| or |sct| is not for this log,
  // SCT_NOT_OBSERVED will be returned.
  SCTInclusionStatus GetLogEntryInclusionStatus(
      net::X509Certificate* cert,
      const net::ct::SignedCertificateTimestamp* sct);

 private:
  struct EntryToAudit;
  struct EntryAuditState;
  struct EntryAuditResult {};
  class NetworkObserver;
  friend class NetworkObserver;

  // Less-than comparator that sorts EntryToAudits based on the SCT timestamp,
  // with smaller (older) SCTs appearing less than larger (newer) SCTs.
  struct OrderByTimestamp {
    bool operator()(const EntryToAudit& lhs, const EntryToAudit& rhs) const;
  };

  // Requests an inclusion proof for each of the entries in |pending_entries_|
  // until throttled by the LogDnsClient.
  void ProcessPendingEntries();

  // Returns the inclusion status of the given |entry|, similar to
  // GetLogEntryInclusionStatus(). The |entry| is an internal representation of
  // a certificate + SCT combination.
  SCTInclusionStatus GetAuditedEntryInclusionStatus(const EntryToAudit& entry);

  // Processes the result of obtaining an audit proof for |entry|.
  // * If an audit proof was successfully obtained and validated,
  //   updates |checked_entries_| so that future calls to
  //   GetLogEntryInclusionStatus() will indicate the entry's
  //   inclusion.
  // * If there was a failure to obtain or validate an inclusion
  //   proof, removes |entry| from the queue of entries to validate.
  //   Future calls to GetLogEntryInclusionStatus() will indicate the entry
  //   has not been observed.
  void OnAuditProofObtained(const EntryToAudit& entry, int net_error);

  // Discards all entries pending inclusion check on network change.
  // That is done to prevent the client looking up inclusion proofs for
  // certificates received from one network, on another network, thus
  // leaking state between networks.
  void ResetPendingQueue();

  // Clears entries to reduce memory overhead.
  void OnMemoryPressure(
      base::MemoryPressureListener::MemoryPressureLevel memory_pressure_level);

  void LogAuditResultToNetLog(const EntryToAudit& entry, bool success);

  // Holds the latest STH fetched and verified for this log.
  net::ct::SignedTreeHead verified_sth_;

  // The log being tracked.
  scoped_refptr<const net::CTLogVerifier> ct_log_;

  // Log entries waiting to be checked for inclusion, or being checked for
  // inclusion, and their state.
  std::map<EntryToAudit, EntryAuditState, OrderByTimestamp> pending_entries_;

  // A cache of leaf hashes identifying entries which were checked for
  // inclusion (the key is the Leaf Hash of the log entry).
  // NOTE: The current implementation does not cache failures, so the presence
  // of an entry in |checked_entries_| indicates success.
  // To extend support for caching failures, a success indicator should be
  // added to the EntryAuditResult struct.
  base::MRUCache<net::SHA256HashValue,
                 EntryAuditResult,
                 net::SHA256HashValueLessThan>
      checked_entries_;

  LogDnsClient* dns_client_;

  std::unique_ptr<base::MemoryPressureListener> memory_pressure_listener_;

  net::NetLogWithSource net_log_;

  std::unique_ptr<NetworkObserver> network_observer_;

  base::WeakPtrFactory<SingleTreeTracker> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(SingleTreeTracker);
};

}  // namespace certificate_transparency

#endif  // COMPONENTS_CERTIFICATE_TRANSPARENCY_SINGLE_TREE_TRACKER_H_
