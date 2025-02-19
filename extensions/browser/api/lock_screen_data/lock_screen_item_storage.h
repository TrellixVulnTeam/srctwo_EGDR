// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_API_LOCK_SCREEN_DATA_LOCK_SCREEN_ITEM_STORAGE_H_
#define EXTENSIONS_BROWSER_API_LOCK_SCREEN_DATA_LOCK_SCREEN_ITEM_STORAGE_H_

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observer.h"
#include "base/sequenced_task_runner.h"
#include "extensions/browser/api/lock_screen_data/data_item.h"
#include "extensions/browser/extension_registry_observer.h"

class PrefRegistrySimple;
class PrefService;

namespace base {
class TickClock;
}

namespace content {
class BrowserContext;
}

namespace extensions {

class Extension;
class ExtensionRegistry;
class LocalValueStoreCache;

namespace lock_screen_data {

class DataItem;
enum class OperationResult;

// Keeps track of lock screen data items created by extensions.
// Note that only a single instance is allowed per process - once created, the
// instance can be retrieved using LockScreenItemStorage::GetIfAllowed().
class LockScreenItemStorage : public ExtensionRegistryObserver {
 public:
  using CreateCallback =
      base::Callback<void(OperationResult result, const DataItem* item)>;
  using DataItemListCallback =
      base::Callback<void(const std::vector<const DataItem*>& items)>;
  using WriteCallback = DataItem::WriteCallback;
  using ReadCallback = DataItem::ReadCallback;

  static void RegisterLocalState(PrefRegistrySimple* registry);

  // Gets the global LockScreenItemStorage instance if lock screen item storage
  // can be used from the browser context.
  // |context| - Context from which the item storage is needed, if the context
  // is not allowed to use LockScreenItemStorage, this will return nullptr.
  // LockScreenItemStorage can be only used from:
  //  *  lock screen browser context while session state is set to locked
  //  *  from the browser process passed to the LockScreenItemStorage ctor when
  //     the session state is set to unlocked.
  static LockScreenItemStorage* GetIfAllowed(content::BrowserContext* context);

  // Creates a LockScreenItemStorage instance. Note that only one
  // LockScreenItemStorage can be present at a time - the constructor will set a
  // global reference to the created object that can be retrieved using
  // LockScreenItemStorage::GetIfAllowed (the reference will be reset when the
  // instance goes out of scoped, i.e. in ~LockScreenItemStorage).
  //
  // |context| - primary user context, only apps from that context should be
  //     able to use the storage.
  // |local_state| - Local state preference.
  // |crypto_key| - Symmetric key that should be used to encrypt data content
  //     when it's persisted on the disk.
  // |storage_root| - Directory on the disk to which data item content should be
  //     persisted.
  LockScreenItemStorage(content::BrowserContext* context,
                        PrefService* local_state,
                        const std::string& crypto_key,
                        const base::FilePath& storage_root);
  ~LockScreenItemStorage() override;

  // Updates the LockScreenItemStorage's view of whether the user session is
  // locked.
  void SetSessionLocked(bool session_locked);

  // Creates a new data item for the extension.
  void CreateItem(const std::string& extension_id,
                  const CreateCallback& callback);

  // Returns all existing data items associated with the extension.
  void GetAllForExtension(const std::string& extension_id,
                          const DataItemListCallback& callback);

  // Updates the content of the item identified by |item_id| that is associated
  // with the provided extension.
  void SetItemContent(const std::string& extension_id,
                      const std::string& item_id,
                      const std::vector<char>& data,
                      const WriteCallback& callback);

  // Retrieves the content of the item identified by |item_id| that is
  // associated with the provided extension.
  void GetItemContent(const std::string& extension_id,
                      const std::string& item_id,
                      const ReadCallback& callback);

  // Deletes the data item associated with the extension.
  void DeleteItem(const std::string& extension_id,
                  const std::string& item_id,
                  const WriteCallback& callback);

  // extensions::ExtensionRegistryObserver:
  void OnExtensionUninstalled(content::BrowserContext* browser_context,
                              const Extension* extension,
                              UninstallReason reason) override;

  // Used in tests to inject fake data items implementations.
  using ItemFactoryCallback =
      base::Callback<std::unique_ptr<DataItem>(const std::string& id,
                                               const std::string& extension_id,
                                               const std::string& crypto_key)>;
  using RegisteredItemsGetter =
      base::Callback<void(const std::string& extension_id,
                          const DataItem::RegisteredValuesCallback& callback)>;
  using ItemStoreDeleter = base::Callback<void(const std::string& extension_id,
                                               const base::Closure& callback)>;
  static void SetItemProvidersForTesting(
      RegisteredItemsGetter* item_fetch_callback,
      ItemFactoryCallback* factory_callback,
      ItemStoreDeleter* deleter_callback);

  const std::string& crypto_key_for_testing() const { return crypto_key_; }

 private:
  // Maps a data item ID to the data item instance.
  using DataItemMap =
      std::unordered_map<std::string, std::unique_ptr<DataItem>>;

  // Contains information about cached data item information for an extension -
  // in particular, the list of existing data items.
  struct CachedExtensionData {
    enum class State {
      // The set of registered data items has not been requested.
      kInitial,
      // The initial set of registered data items is being loaded.
      kLoading,
      // The set of registered data items has been loaded; the cached list of
      // items should be up to date.
      kLoaded,
    };

    CachedExtensionData();
    ~CachedExtensionData();

    State state = State::kInitial;
    // The set of registered data items.
    DataItemMap data_items;
    // When the initial data item list is being loaded, contains the callback
    // waiting for the load to finish. When the initial data item set is loaded,
    // all of the callbacks in this list will be run.
    std::vector<base::Closure> load_callbacks;
  };

  // Maps an extension ID to data items associated with the extension.
  using ExtensionDataMap = std::unordered_map<std::string, CachedExtensionData>;

  // The session state as seen by the LockScreenItemStorage.
  enum class SessionLockedState { kUnknown, kLocked, kNotLocked };

  // Whether the context is allowed to use LockScreenItemStorage.
  // Result depends on the current LockScreenItemStorage state - see
  // |LockScreenItemStorage::GetIfAllowed|.
  bool IsContextAllowed(content::BrowserContext* context);

  // Implementations for data item management methods - called when the data
  // cache for the associated extension was initialized:
  void CreateItemImpl(const std::string& extension_id,
                      const CreateCallback& callback);
  void GetAllForExtensionImpl(const std::string& extension_id,
                              const DataItemListCallback& callback);
  void SetItemContentImpl(const std::string& extension_id,
                          const std::string& item_id,
                          const std::vector<char>& data,
                          const WriteCallback& callback);
  void GetItemContentImpl(const std::string& extension_id,
                          const std::string& item_id,
                          const ReadCallback& callback);
  void DeleteItemImpl(const std::string& extension_id,
                      const std::string& item_id,
                      const WriteCallback& callback);

  // Defers |callback| until the cached data item set is loaded for the
  // extension. If the data is already loaded, |callback| will be run
  // immediately.
  void EnsureCacheForExtensionLoaded(const std::string& extension_id,
                                     const base::Closure& callback);

  // Callback for loading initial set of known data items for an extension.
  void OnGotExtensionItems(const std::string& extension_id,
                           const base::TimeTicks& start_time,
                           OperationResult result,
                           std::unique_ptr<base::DictionaryValue> items);

  // Callback for registration operation of a newly created data item - it adds
  // the item to the cached data item state, and invoked the callback.
  void OnItemRegistered(std::unique_ptr<DataItem> item,
                        const std::string& extension_id,
                        const base::TimeTicks& start_time,
                        const CreateCallback& callback,
                        OperationResult result);

  // Callback for data item write operation - it invokes the callback with the
  // operation result.
  void OnItemWritten(const base::TimeTicks& start_time,
                     const WriteCallback& callback,
                     OperationResult result);

  // Callback for data item read operation - it invokes the callback with the
  // operation result.
  void OnItemRead(const base::TimeTicks& start_time,
                  const ReadCallback& callback,
                  OperationResult result,
                  std::unique_ptr<std::vector<char>> data);

  // Callback for item deletion operation. It removes the item from the cached
  // state and invokes the callback with the operation result.
  void OnItemDeleted(const std::string& extension_id,
                     const std::string& item_id,
                     const base::TimeTicks& start_time,
                     const WriteCallback& callback,
                     OperationResult result);

  // Finds a data item associated with the extension.
  DataItem* FindItem(const std::string& extension_id,
                     const std::string& item_id);

  // Gets the set of extensions that are known to have registered data items -
  // this information is kept in the local state so it can be used to
  // synchronously determine the set of extensions that should be sent a
  // DataItemsAvailable event, and to determine the set of extensions whose data
  // should be deleted (for example if they are not installed anymore).
  std::set<std::string> GetExtensionsWithDataItems(bool include_empty);

  // Deletes data item data for all extensions that have existing data items,
  // but are not currently installed.
  void ClearUninstalledAppData();

  // Clears all data items associated with the extension.
  void ClearExtensionData(const std::string& id);

  // Removes local state entry for the extension.
  void RemoveExtensionFromLocalState(const std::string& id);

  content::BrowserContext* context_;

  // The user associated with the primary profile.
  const std::string user_id_;
  const std::string crypto_key_;
  PrefService* local_state_;
  const base::FilePath storage_root_;

  std::unique_ptr<base::TickClock> tick_clock_;

  SessionLockedState session_locked_state_ = SessionLockedState::kUnknown;

  ScopedObserver<ExtensionRegistry, ExtensionRegistryObserver>
      extension_registry_observer_;

  // Expected to be used only on FILE thread.
  std::unique_ptr<LocalValueStoreCache> value_store_cache_;

  // Mapping containing all known data items for extensions.
  ExtensionDataMap data_item_cache_;

  scoped_refptr<base::SequencedTaskRunner> task_runner_;

  base::WeakPtrFactory<LockScreenItemStorage> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(LockScreenItemStorage);
};

}  // namespace lock_screen_data
}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_LOCK_SCREEN_DATA_LOCK_SCREEN_ITEM_STORAGE_H_
