// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PRINTING_CLOUD_PRINT_PRIVET_NOTIFICATIONS_H_
#define CHROME_BROWSER_PRINTING_CLOUD_PRINT_PRIVET_NOTIFICATIONS_H_

#include <map>
#include <memory>
#include <string>

#include "chrome/browser/notifications/notification_delegate.h"
#include "chrome/browser/printing/cloud_print/privet_device_lister.h"
#include "chrome/browser/printing/cloud_print/privet_http.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/prefs/pref_member.h"
#include "net/net_features.h"

class Profile;

namespace content {
class BrowserContext;
}

namespace local_discovery {
class ServiceDiscoverySharedClient;
}

namespace cloud_print {

class PrivetDeviceLister;
class PrivetHTTPAsynchronousFactory;
class PrivetHTTPResolution;
class PrivetNotificationDelegate;
struct DeviceDescription;

#if BUILDFLAG(ENABLE_MDNS)
class PrivetTrafficDetector;
#endif  // ENABLE_MDNS

// Contains logic related to notifications not tied actually displaying them.
class PrivetNotificationsListener  {
 public:
  class Delegate {
   public:
    virtual ~Delegate() {}

    // Notify user that printer(s) have been added or removed.
    virtual void PrivetNotify(int devices_active, bool added) = 0;

    // Notify user that all printers have been removed.
    virtual void PrivetRemoveNotification() = 0;
  };

  PrivetNotificationsListener(
      std::unique_ptr<PrivetHTTPAsynchronousFactory> privet_http_factory,
      Delegate* delegate);
  virtual ~PrivetNotificationsListener();

  // These two methods are akin to those of PrivetDeviceLister::Delegate. The
  // user of PrivetNotificationListener should create a PrivetDeviceLister and
  // forward device notifications to the PrivetNotificationLister.
  void DeviceChanged(const std::string& name,
                     const DeviceDescription& description);
  void DeviceRemoved(const std::string& name);
  virtual void DeviceCacheFlushed();

 private:
  struct DeviceContext {
    DeviceContext();
    ~DeviceContext();

    bool notification_may_be_active;
    bool registered;
    std::unique_ptr<PrivetJSONOperation> info_operation;
    std::unique_ptr<PrivetHTTPResolution> privet_http_resolution;
    std::unique_ptr<PrivetHTTPClient> privet_http;
  };

  using DeviceContextMap =
      std::map<std::string, std::unique_ptr<DeviceContext>>;

  void CreateInfoOperation(std::unique_ptr<PrivetHTTPClient> http_client);
  void OnPrivetInfoDone(DeviceContext* device,
                        const base::DictionaryValue* json_value);


  void NotifyDeviceRemoved();

  Delegate* const delegate_;
  std::unique_ptr<PrivetDeviceLister> device_lister_;
  std::unique_ptr<PrivetHTTPAsynchronousFactory> privet_http_factory_;
  DeviceContextMap devices_seen_;
  int devices_active_;
};

class PrivetNotificationService
    : public KeyedService,
      public PrivetDeviceLister::Delegate,
      public PrivetNotificationsListener::Delegate,
      public base::SupportsWeakPtr<PrivetNotificationService> {
 public:
  explicit PrivetNotificationService(content::BrowserContext* profile);
  ~PrivetNotificationService() override;

  // PrivetDeviceLister::Delegate implementation:
  void DeviceChanged(const std::string& name,
                     const DeviceDescription& description) override;
  void DeviceRemoved(const std::string& name) override;
  void DeviceCacheFlushed() override;

  // PrivetNotificationListener::Delegate implementation:
  void PrivetNotify(int devices_active, bool added) override;
  void PrivetRemoveNotification() override;

  static bool IsEnabled();
  static bool IsForced();

 private:
  void Start();
  void OnNotificationsEnabledChanged();
  void StartLister();

  // Virtual for testing. The returned delegate is refcounted.
  virtual PrivetNotificationDelegate* CreateNotificationDelegate(
      Profile* profile);

  content::BrowserContext* const profile_;
  std::unique_ptr<PrivetDeviceLister> device_lister_;
  scoped_refptr<local_discovery::ServiceDiscoverySharedClient>
      service_discovery_client_;
  std::unique_ptr<PrivetNotificationsListener> privet_notifications_listener_;
  BooleanPrefMember enable_privet_notification_member_;

#if BUILDFLAG(ENABLE_MDNS)
  scoped_refptr<PrivetTrafficDetector> traffic_detector_;
#endif  // ENABLE_MDNS
};

class PrivetNotificationDelegate : public NotificationDelegate {
 public:
  explicit PrivetNotificationDelegate(Profile* profile);

  // NotificationDelegate implementation.
  std::string id() const override;
  void ButtonClick(int button_index) override;

 protected:
  // Refcounted.
  ~PrivetNotificationDelegate() override;

 private:
  // ButtonClick() response handlers. Virtual for testing.
  virtual void OpenTab(const GURL& url);
  virtual void DisableNotifications();

  void CloseNotification();

  Profile* const profile_;
};

}  // namespace cloud_print

#endif  // CHROME_BROWSER_PRINTING_CLOUD_PRINT_PRIVET_NOTIFICATIONS_H_
