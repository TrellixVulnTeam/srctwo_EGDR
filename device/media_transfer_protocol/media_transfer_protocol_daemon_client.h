// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Client code to talk to the Media Transfer Protocol daemon. The MTP daemon is
// responsible for communicating with PTP / MTP capable devices like cameras
// and smartphones.

#ifndef DEVICE_MEDIA_TRANSFER_PROTOCOL_MEDIA_TRANSFER_PROTOCOL_DAEMON_CLIENT_H_
#define DEVICE_MEDIA_TRANSFER_PROTOCOL_MEDIA_TRANSFER_PROTOCOL_DAEMON_CLIENT_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "build/build_config.h"

#if !defined(OS_LINUX)
#error "Only used on Linux and ChromeOS"
#endif

class MtpFileEntry;
class MtpStorageInfo;

namespace dbus {
class Bus;
}

namespace device {

// A class to make the actual DBus calls for mtpd service.
// This class only makes calls, result/error handling should be done
// by callbacks.
class MediaTransferProtocolDaemonClient {
 public:
  // A callback to be called when DBus method call fails.
  typedef base::Closure ErrorCallback;

  // A callback to handle the result of EnumerateAutoMountableDevices.
  // The argument is the enumerated storage names.
  typedef base::Callback<void(const std::vector<std::string>& storage_names)
                         > EnumerateStoragesCallback;

  // A callback to handle the result of GetStorageInfo.
  // The argument is the information about the specified storage.
  typedef base::Callback<void(const MtpStorageInfo& storage_info)
                         > GetStorageInfoCallback;

  // A callback to handle the result of OpenStorage.
  // The argument is the returned handle.
  typedef base::Callback<void(const std::string& handle)> OpenStorageCallback;

  // A callback to handle the result of CloseStorage.
  typedef base::Closure CloseStorageCallback;

  // A callback to handle the result of CreateDirectory.
  typedef base::Closure CreateDirectoryCallback;

  // A callback to handle the result of ReadDirectoryEntryIds.
  // The argument is a vector of file ids.
  typedef base::Callback<void(const std::vector<uint32_t>& file_ids)>
      ReadDirectoryEntryIdsCallback;

  // A callback to handle the result of GetFileInfo.
  // The argument is a vector of file entries.
  typedef base::Callback<void(const std::vector<MtpFileEntry>& file_entries)
                         > GetFileInfoCallback;

  // A callback to handle the result of ReadFileChunkById.
  // The argument is a string containing the file data.
  typedef base::Callback<void(const std::string& data)> ReadFileCallback;

  // A callback to handle the result of RenameObject.
  typedef base::Closure RenameObjectCallback;

  // A callback to handle the result of CopyFileFromLocal.
  typedef base::Closure CopyFileFromLocalCallback;

  // A callback to handle the result of DeleteObject.
  typedef base::Closure DeleteObjectCallback;

  // A callback to handle storage attach/detach events.
  // The first argument is true for attach, false for detach.
  // The second argument is the storage name.
  typedef base::Callback<void(bool is_attach,
                              const std::string& storage_name)
                         > MTPStorageEventHandler;

  virtual ~MediaTransferProtocolDaemonClient();

  // Calls EnumerateStorages method. |callback| is called after the
  // method call succeeds, otherwise, |error_callback| is called.
  virtual void EnumerateStorages(
      const EnumerateStoragesCallback& callback,
      const ErrorCallback& error_callback) = 0;

  // Calls GetStorageInfo method. |callback| is called after the method call
  // succeeds, otherwise, |error_callback| is called.
  virtual void GetStorageInfo(const std::string& storage_name,
                              const GetStorageInfoCallback& callback,
                              const ErrorCallback& error_callback) = 0;

  // Calls GetStorageInfoFromDevice method. |callback| is called after the
  // method call succeeds, otherwise, |error_callback| is called.
  virtual void GetStorageInfoFromDevice(
      const std::string& storage_name,
      const GetStorageInfoCallback& callback,
      const ErrorCallback& error_callback) = 0;

  // Calls OpenStorage method. |callback| is called after the method call
  // succeeds, otherwise, |error_callback| is called.
  // OpenStorage returns a handle in |callback|.
  virtual void OpenStorage(const std::string& storage_name,
                           const std::string& mode,
                           const OpenStorageCallback& callback,
                           const ErrorCallback& error_callback) = 0;

  // Calls CloseStorage method. |callback| is called after the method call
  // succeeds, otherwise, |error_callback| is called.
  // |handle| comes from a OpenStorageCallback.
  virtual void CloseStorage(const std::string& handle,
                            const CloseStorageCallback& callback,
                            const ErrorCallback& error_callback) = 0;

  // Calls CreateDirectory method. |callback| is called after the method call
  // succeeds, otherwise, |error_callback| is called.
  // |parent_id| is an id of the parent directory.
  // |directory_name| is name of new directory.
  virtual void CreateDirectory(const std::string& handle,
                               const uint32_t parent_id,
                               const std::string& directory_name,
                               const CreateDirectoryCallback& callback,
                               const ErrorCallback& error_callback) = 0;

  // Calls ReadDirectoryEntryIds method. |callback| is called after the method
  // call succeeds, otherwise, |error_callback| is called.
  // |file_id| is a MTP-device specific id for a file.
  virtual void ReadDirectoryEntryIds(
      const std::string& handle,
      uint32_t file_id,
      const ReadDirectoryEntryIdsCallback& callback,
      const ErrorCallback& error_callback) = 0;

  // Calls GetFileInfo method. |callback| is called after the method
  // call succeeds, otherwise, |error_callback| is called.
  // |file_ids| is a list of MTP-device specific file ids.
  // |offset| is the index into |file_ids| to read from.
  // |entries_to_read| is the maximum number of file entries to read.
  virtual void GetFileInfo(const std::string& handle,
                           const std::vector<uint32_t>& file_ids,
                           size_t offset,
                           size_t entries_to_read,
                           const GetFileInfoCallback& callback,
                           const ErrorCallback& error_callback) = 0;

  // Calls ReadFileChunk method. |callback| is called after the method call
  // succeeds, otherwise, |error_callback| is called.
  // |file_id| is a MTP-device specific id for a file.
  // |offset| is the offset into the file.
  // |bytes_to_read| cannot exceed 1 MiB.
  virtual void ReadFileChunk(const std::string& handle,
                             uint32_t file_id,
                             uint32_t offset,
                             uint32_t bytes_to_read,
                             const ReadFileCallback& callback,
                             const ErrorCallback& error_callback) = 0;

  // Calls RenameObject method. |callback| is called after the method call
  // succeeds, otherwise, |error_callback| is called.
  // |object_is| is an id of object to be renamed.
  // |new_name| is new name of the object.
  virtual void RenameObject(const std::string& handle,
                            const uint32_t object_id,
                            const std::string& new_name,
                            const RenameObjectCallback& callback,
                            const ErrorCallback& error_callback) = 0;

  // Calls CopyFileFromLocal method. |callback| is called after the method call
  // succeeds, otherwise, |error_callback| is called.
  // |source_file_descriptor| is a file descriptor of source file.
  // |parent_id| is a object id of a target directory.
  // |file_name| is a file name of a target file.
  virtual void CopyFileFromLocal(const std::string& handle,
                                 const int source_file_descriptor,
                                 const uint32_t parent_id,
                                 const std::string& file_name,
                                 const CopyFileFromLocalCallback& callback,
                                 const ErrorCallback& error_callback) = 0;

  // Calls DeleteObject method. |callback| is called after the method call
  // succeeds, otherwise, |error_callback| is called.
  // |object_id| is an object id of a file or directory which is deleted.
  virtual void DeleteObject(const std::string& handle,
                            const uint32_t object_id,
                            const DeleteObjectCallback& callback,
                            const ErrorCallback& error_callback) = 0;

  // Registers given callback for events. Should only be called once.
  // |storage_event_handler| is called when a mtp storage attach or detach
  // signal is received.
  virtual void ListenForChanges(const MTPStorageEventHandler& handler) = 0;

  // Factory function, creates a new instance and returns ownership.
  static MediaTransferProtocolDaemonClient* Create(dbus::Bus* bus);

 protected:
  // Create() should be used instead.
  MediaTransferProtocolDaemonClient();

 private:
  DISALLOW_COPY_AND_ASSIGN(MediaTransferProtocolDaemonClient);
};

}  // namespace device

#endif  // DEVICE_MEDIA_TRANSFER_PROTOCOL_MEDIA_TRANSFER_PROTOCOL_DAEMON_CLIENT_H_
