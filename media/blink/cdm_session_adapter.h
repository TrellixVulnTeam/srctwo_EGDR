// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BLINK_CDM_SESSION_ADAPTER_H_
#define MEDIA_BLINK_CDM_SESSION_ADAPTER_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "base/containers/hash_tables.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "media/base/content_decryption_module.h"
#include "third_party/WebKit/public/platform/WebContentDecryptionModuleResult.h"
#include "third_party/WebKit/public/platform/WebContentDecryptionModuleSession.h"

namespace url {
class Origin;
}

namespace media {

struct CdmConfig;
class CdmFactory;
class WebContentDecryptionModuleSessionImpl;

// Owns the CDM instance and makes calls from session objects to the CDM.
// Forwards the session ID-based callbacks of the ContentDecryptionModule
// interface to the appropriate session object. Callers should hold references
// to this class as long as they need the CDM instance.
class CdmSessionAdapter : public base::RefCounted<CdmSessionAdapter> {
 public:
  CdmSessionAdapter();

  // Creates the CDM for |key_system| using |cdm_factory| and returns the result
  // via |result|.
  void CreateCdm(
      CdmFactory* cdm_factory,
      const std::string& key_system,
      const url::Origin& security_origin,
      const CdmConfig& cdm_config,
      std::unique_ptr<blink::WebContentDecryptionModuleResult> result);

  // Provides a server certificate to be used to encrypt messages to the
  // license server.
  void SetServerCertificate(const std::vector<uint8_t>& certificate,
                            std::unique_ptr<SimpleCdmPromise> promise);

  // Gets the key status for a hypothetical key with |min_hdcp_version|
  // requirement.
  void GetStatusForPolicy(HdcpVersion min_hdcp_version,
                          std::unique_ptr<KeyStatusCdmPromise> promise);

  // Creates a new session and adds it to the internal map. RemoveSession()
  // must be called when destroying it, if RegisterSession() was called.
  std::unique_ptr<WebContentDecryptionModuleSessionImpl> CreateSession();

  // Adds a session to the internal map. Called once the session is successfully
  // initialized. Returns true if the session was registered, false if there is
  // already an existing session with the same |session_id|.
  bool RegisterSession(
      const std::string& session_id,
      base::WeakPtr<WebContentDecryptionModuleSessionImpl> session);

  // Removes a session from the internal map.
  void UnregisterSession(const std::string& session_id);

  // Initializes a session with the |init_data_type|, |init_data| and
  // |session_type| provided.
  void InitializeNewSession(EmeInitDataType init_data_type,
                            const std::vector<uint8_t>& init_data,
                            CdmSessionType session_type,
                            std::unique_ptr<NewSessionCdmPromise> promise);

  // Loads the session specified by |session_id|.
  void LoadSession(CdmSessionType session_type,
                   const std::string& session_id,
                   std::unique_ptr<NewSessionCdmPromise> promise);

  // Updates the session specified by |session_id| with |response|.
  void UpdateSession(const std::string& session_id,
                     const std::vector<uint8_t>& response,
                     std::unique_ptr<SimpleCdmPromise> promise);

  // Closes the session specified by |session_id|.
  void CloseSession(const std::string& session_id,
                    std::unique_ptr<SimpleCdmPromise> promise);

  // Removes stored session data associated with the session specified by
  // |session_id|.
  void RemoveSession(const std::string& session_id,
                     std::unique_ptr<SimpleCdmPromise> promise);

  // Returns a reference to the CDM.
  scoped_refptr<ContentDecryptionModule> GetCdm();

  // Returns the key system name.
  const std::string& GetKeySystem() const;

  // Returns a prefix to use for UMAs.
  const std::string& GetKeySystemUMAPrefix() const;

 private:
  friend class base::RefCounted<CdmSessionAdapter>;

  // Session ID to WebContentDecryptionModuleSessionImpl mapping.
  typedef base::hash_map<std::string,
                         base::WeakPtr<WebContentDecryptionModuleSessionImpl> >
      SessionMap;

  ~CdmSessionAdapter();

  // Callback for CreateCdm().
  void OnCdmCreated(const std::string& key_system,
                    base::TimeTicks start_time,
                    const scoped_refptr<ContentDecryptionModule>& cdm,
                    const std::string& error_message);

  // Callbacks for firing session events.
  void OnSessionMessage(const std::string& session_id,
                        CdmMessageType message_type,
                        const std::vector<uint8_t>& message);
  void OnSessionKeysChange(const std::string& session_id,
                           bool has_additional_usable_key,
                           CdmKeysInfo keys_info);
  void OnSessionExpirationUpdate(const std::string& session_id,
                                 base::Time new_expiry_time);
  void OnSessionClosed(const std::string& session_id);

  // Helper function of the callbacks.
  WebContentDecryptionModuleSessionImpl* GetSession(
      const std::string& session_id);

  scoped_refptr<ContentDecryptionModule> cdm_;

  SessionMap sessions_;

  std::string key_system_;
  std::string key_system_uma_prefix_;

  // A unique ID to trace CdmSessionAdapter::CreateCdm() call and the matching
  // OnCdmCreated() call.
  uint32_t trace_id_;

  std::unique_ptr<blink::WebContentDecryptionModuleResult> cdm_created_result_;

  // NOTE: Weak pointers must be invalidated before all other member variables.
  base::WeakPtrFactory<CdmSessionAdapter> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(CdmSessionAdapter);
};

}  // namespace media

#endif  // MEDIA_BLINK_CDM_SESSION_ADAPTER_H_
