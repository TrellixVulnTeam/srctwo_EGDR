// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_ARC_DOWNLOADS_WATCHER_ARC_DOWNLOADS_WATCHER_SERVICE_H_
#define CHROME_BROWSER_CHROMEOS_ARC_DOWNLOADS_WATCHER_ARC_DOWNLOADS_WATCHER_SERVICE_H_

#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "components/arc/common/file_system.mojom.h"
#include "components/arc/instance_holder.h"
#include "components/keyed_service/core/keyed_service.h"

namespace base {
class FilePath;
class SequencedTaskRunner;
}  // namespace base

namespace content {
class BrowserContext;
}  // namespace content

namespace arc {

class ArcBridgeService;

// Returns true if the file path has a media extension supported by Android.
bool HasAndroidSupportedMediaExtension(const base::FilePath& path);

// Exposed only for testing.
extern const char* kAndroidSupportedMediaExtensions[];
extern const int kAndroidSupportedMediaExtensionsSize;

// Watches Downloads directory and registers newly created media files to
// Android MediaProvider.
class ArcDownloadsWatcherService
    : public KeyedService,
      public InstanceHolder<mojom::FileSystemInstance>::Observer {
 public:
  // Returns singleton instance for the given BrowserContext,
  // or nullptr if the browser |context| is not allowed to use ARC.
  static ArcDownloadsWatcherService* GetForBrowserContext(
      content::BrowserContext* context);

  ArcDownloadsWatcherService(content::BrowserContext* context,
                             ArcBridgeService* bridge_service);
  ~ArcDownloadsWatcherService() override;

  // InstanceHolder<mojom::FileSystemInstance>::Observer
  void OnInstanceReady() override;
  void OnInstanceClosed() override;

 private:
  class DownloadsWatcher;

  void StartWatchingDownloads();
  void StopWatchingDownloads();

  void OnDownloadsChanged(const std::vector<std::string>& paths);

  content::BrowserContext* const context_;
  ArcBridgeService* const arc_bridge_service_;  // Owned by ArcServiceManager.

  std::unique_ptr<DownloadsWatcher> watcher_;

  scoped_refptr<base::SequencedTaskRunner> file_task_runner_;

  // Note: This should remain the last member so it'll be destroyed and
  // invalidate the weak pointers before any other members are destroyed.
  base::WeakPtrFactory<ArcDownloadsWatcherService> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ArcDownloadsWatcherService);
};

}  // namespace arc

#endif  // CHROME_BROWSER_CHROMEOS_ARC_DOWNLOADS_WATCHER_ARC_DOWNLOADS_WATCHER_SERVICE_H_
