// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_SAVED_FILES_SERVICE_H_
#define APPS_SAVED_FILES_SERVICE_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/gtest_prod_util.h"
#include "components/keyed_service/core/keyed_service.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "extensions/browser/api/file_system/saved_files_service_interface.h"

namespace content {
class BrowserContext;
}

class SavedFilesServiceUnitTest;
FORWARD_DECLARE_TEST(SavedFilesServiceUnitTest, RetainTwoFilesTest);
FORWARD_DECLARE_TEST(SavedFilesServiceUnitTest, EvictionTest);
FORWARD_DECLARE_TEST(SavedFilesServiceUnitTest, SequenceNumberCompactionTest);

namespace extensions {
class Extension;
struct SavedFileEntry;
}

namespace apps {

// Tracks the files that apps have retained access to both while running and
// when suspended.
class SavedFilesService : public extensions::SavedFilesServiceInterface,
                          public KeyedService,
                          public content::NotificationObserver {
 public:
  explicit SavedFilesService(content::BrowserContext* context);
  ~SavedFilesService() override;

  static SavedFilesService* Get(content::BrowserContext* context);

  // extensions::SavedFilesServiceInterface:
  void RegisterFileEntry(const std::string& extension_id,
                         const std::string& id,
                         const base::FilePath& file_path,
                         bool is_directory) override;
  void EnqueueFileEntry(const std::string& extension_id,
                        const std::string& id) override;
  bool IsRegistered(const std::string& extension_id,
                    const std::string& id) override;
  const extensions::SavedFileEntry* GetFileEntry(
      const std::string& extension_id,
      const std::string& id) override;

  // Returns all registered file entries.
  std::vector<extensions::SavedFileEntry> GetAllFileEntries(
      const std::string& extension_id);

  // Clears all retained files if the app does not have the
  // fileSystem.retainEntries permission.
  void ClearQueueIfNoRetainPermission(const extensions::Extension* extension);

  // Clears all retained files.
  void ClearQueue(const extensions::Extension* extension);

  // Called to notify that the application has begun to exit.
  void OnApplicationTerminating();

 private:
  FRIEND_TEST_ALL_PREFIXES(::SavedFilesServiceUnitTest, RetainTwoFilesTest);
  FRIEND_TEST_ALL_PREFIXES(::SavedFilesServiceUnitTest, EvictionTest);
  FRIEND_TEST_ALL_PREFIXES(::SavedFilesServiceUnitTest,
                           SequenceNumberCompactionTest);
  friend class ::SavedFilesServiceUnitTest;

  // A container for the registered files for an app.
  class SavedFiles;

  // content::NotificationObserver.
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  // Returns the SavedFiles for |extension_id| or NULL if one does not exist.
  SavedFiles* Get(const std::string& extension_id) const;

  // Returns the SavedFiles for |extension_id|, creating it if necessary.
  SavedFiles* GetOrInsert(const std::string& extension_id);

  // Clears the SavedFiles for |extension_id|.
  void Clear(const std::string& extension_id);

  static void SetMaxSequenceNumberForTest(int max_value);
  static void ClearMaxSequenceNumberForTest();
  static void SetLruSizeForTest(int size);
  static void ClearLruSizeForTest();

  std::map<std::string, std::unique_ptr<SavedFiles>>
      extension_id_to_saved_files_;
  content::NotificationRegistrar registrar_;
  content::BrowserContext* context_;

  DISALLOW_COPY_AND_ASSIGN(SavedFilesService);
};

}  // namespace apps

#endif  // APPS_SAVED_FILES_SERVICE_H_
