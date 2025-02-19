// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/local_discovery/service_discovery_client_mdns.h"

#include <stddef.h>

#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/metrics/histogram_macros.h"
#include "base/task_scheduler/post_task.h"
#include "base/threading/thread_task_runner_handle.h"
#include "chrome/browser/local_discovery/service_discovery_client_impl.h"
#include "content/public/browser/browser_thread.h"
#include "net/dns/mdns_client.h"
#include "net/socket/datagram_server_socket.h"

namespace net {
class IPAddress;
}

namespace local_discovery {

using content::BrowserThread;

// Base class for objects returned by ServiceDiscoveryClient implementation.
// Handles interaction of client code on UI thread end net code on mdns thread.
class ServiceDiscoveryClientMdns::Proxy {
 public:
  using WeakPtr = base::WeakPtr<Proxy>;

  explicit Proxy(ServiceDiscoveryClientMdns* client)
      : client_(client),
        weak_ptr_factory_(this) {
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    client_->proxies_.AddObserver(this);
  }

  virtual ~Proxy() {
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    client_->proxies_.RemoveObserver(this);
  }

  // Returns true if object is not yet shutdown.
  virtual bool IsValid() = 0;

  // Notifies proxies that mDNS layer is going to be destroyed.
  virtual void OnMdnsDestroy() = 0;

  // Notifies proxies that new mDNS instance is ready.
  virtual void OnNewMdnsReady() {
    DCHECK(!client_->need_delay_mdns_tasks_);
    if (IsValid()) {
      for (const auto& task : delayed_tasks_)
        client_->mdns_runner_->PostTask(FROM_HERE, task);
    }
    delayed_tasks_.clear();
  }

  // Runs callback using this method to abort callback if instance of |Proxy|
  // is deleted.
  void RunCallback(const base::Closure& callback) {
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    callback.Run();
  }

 protected:
  void PostToMdnsThread(const base::Closure& task) {
    DCHECK(IsValid());
    // The first task on the IO thread for each |mdns_| instance must be
    // InitMdns(). OnInterfaceListReady() could be delayed by
    // GetMDnsInterfacesToBind() running on a background task runner, so
    // PostToMdnsThread() could be called to post task for |mdns_| that is not
    // initialized yet.
    if (!client_->need_delay_mdns_tasks_) {
      client_->mdns_runner_->PostTask(FROM_HERE, task);
      return;
    }
    delayed_tasks_.push_back(task);
  }

  static bool PostToUIThread(const base::Closure& task) {
    return BrowserThread::PostTask(BrowserThread::UI, FROM_HERE, task);
  }

  ServiceDiscoveryClient* client() {
    return client_->client_.get();
  }

  WeakPtr GetWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

  template<class T>
  void DeleteOnMdnsThread(T* t) {
    if (!t)
      return;
    if (!client_->mdns_runner_->DeleteSoon(FROM_HERE, t))
      delete t;
  }

 private:
  scoped_refptr<ServiceDiscoveryClientMdns> client_;
  // Delayed |mdns_runner_| tasks.
  std::vector<base::Closure> delayed_tasks_;
  base::WeakPtrFactory<Proxy> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(Proxy);
};

namespace {

const int kMaxRestartAttempts = 10;
const int kRestartDelayOnNetworkChangeSeconds = 3;

using MdnsInitCallback = base::Callback<void(bool)>;

class SocketFactory : public net::MDnsSocketFactory {
 public:
  explicit SocketFactory(const net::InterfaceIndexFamilyList& interfaces)
      : interfaces_(interfaces) {}

  // net::MDnsSocketFactory implementation:
  void CreateSockets(std::vector<std::unique_ptr<net::DatagramServerSocket>>*
                         sockets) override {
    for (size_t i = 0; i < interfaces_.size(); ++i) {
      DCHECK(interfaces_[i].second == net::ADDRESS_FAMILY_IPV4 ||
             interfaces_[i].second == net::ADDRESS_FAMILY_IPV6);
      std::unique_ptr<net::DatagramServerSocket> socket(
          CreateAndBindMDnsSocket(interfaces_[i].second, interfaces_[i].first));
      if (socket)
        sockets->push_back(std::move(socket));
    }
  }

 private:
  net::InterfaceIndexFamilyList interfaces_;
};

void InitMdns(const MdnsInitCallback& on_initialized,
              const net::InterfaceIndexFamilyList& interfaces,
              net::MDnsClient* mdns) {
  SocketFactory socket_factory(interfaces);
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::BindOnce(on_initialized, mdns->StartListening(&socket_factory)));
}

template<class T>
class ProxyBase : public ServiceDiscoveryClientMdns::Proxy, public T {
 public:
  using Base = ProxyBase<T>;

  explicit ProxyBase(ServiceDiscoveryClientMdns* client)
      : Proxy(client) {
  }

  ~ProxyBase() override {
    DeleteOnMdnsThread(implementation_.release());
  }

  bool IsValid() override {
    return !!implementation();
  }

  void OnMdnsDestroy() override {
    DeleteOnMdnsThread(implementation_.release());
  };

 protected:
  void set_implementation(std::unique_ptr<T> implementation) {
    implementation_ = std::move(implementation);
  }

  T* implementation()  const {
    return implementation_.get();
  }

 private:
  std::unique_ptr<T> implementation_;

  DISALLOW_COPY_AND_ASSIGN(ProxyBase);
};

class ServiceWatcherProxy : public ProxyBase<ServiceWatcher> {
 public:
  ServiceWatcherProxy(ServiceDiscoveryClientMdns* client_mdns,
                      const std::string& service_type,
                      const ServiceWatcher::UpdatedCallback& callback)
      : ProxyBase(client_mdns),
        service_type_(service_type),
        callback_(callback) {
    // It's safe to call |CreateServiceWatcher| on UI thread, because
    // |MDnsClient| is not used there. It's simplify implementation.
    set_implementation(client()->CreateServiceWatcher(
        service_type,
        base::Bind(&ServiceWatcherProxy::OnCallback, GetWeakPtr(), callback)));
  }

  // ServiceWatcher methods.
  void Start() override {
    if (implementation()) {
      PostToMdnsThread(base::Bind(&ServiceWatcher::Start,
                                  base::Unretained(implementation())));
    }
  }

  void DiscoverNewServices() override {
    if (implementation()) {
      PostToMdnsThread(base::Bind(&ServiceWatcher::DiscoverNewServices,
                                  base::Unretained(implementation())));
    }
  }

  void SetActivelyRefreshServices(bool actively_refresh_services) override {
    if (implementation()) {
      PostToMdnsThread(base::Bind(&ServiceWatcher::SetActivelyRefreshServices,
                                  base::Unretained(implementation()),
                                  actively_refresh_services));
    }
  }

  std::string GetServiceType() const override { return service_type_; }

  void OnNewMdnsReady() override {
    ProxyBase<ServiceWatcher>::OnNewMdnsReady();
    if (!implementation())
      callback_.Run(ServiceWatcher::UPDATE_INVALIDATED, "");
  }

 private:
  static void OnCallback(const WeakPtr& proxy,
                         const ServiceWatcher::UpdatedCallback& callback,
                         UpdateType a1,
                         const std::string& a2) {
    DCHECK(!BrowserThread::CurrentlyOn(BrowserThread::UI));
    PostToUIThread(base::Bind(&Base::RunCallback, proxy,
                              base::Bind(callback, a1, a2)));
  }
  std::string service_type_;
  ServiceWatcher::UpdatedCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(ServiceWatcherProxy);
};

class ServiceResolverProxy : public ProxyBase<ServiceResolver> {
 public:
  ServiceResolverProxy(ServiceDiscoveryClientMdns* client_mdns,
                       const std::string& service_name,
                       const ServiceResolver::ResolveCompleteCallback& callback)
      : ProxyBase(client_mdns),
        service_name_(service_name) {
    // It's safe to call |CreateServiceResolver| on UI thread, because
    // |MDnsClient| is not used there. It's simplify implementation.
    set_implementation(client()->CreateServiceResolver(
        service_name,
        base::Bind(&ServiceResolverProxy::OnCallback, GetWeakPtr(), callback)));
  }

  // ServiceResolver methods.
  void StartResolving() override {
    if (implementation()) {
      PostToMdnsThread(base::Bind(&ServiceResolver::StartResolving,
                                  base::Unretained(implementation())));
    }
  };

  std::string GetName() const override { return service_name_; }

 private:
  static void OnCallback(
      const WeakPtr& proxy,
      const ServiceResolver::ResolveCompleteCallback& callback,
      RequestStatus a1,
      const ServiceDescription& a2) {
    DCHECK(!BrowserThread::CurrentlyOn(BrowserThread::UI));
    PostToUIThread(base::Bind(&Base::RunCallback, proxy,
                              base::Bind(callback, a1, a2)));
  }

  std::string service_name_;

  DISALLOW_COPY_AND_ASSIGN(ServiceResolverProxy);
};

class LocalDomainResolverProxy : public ProxyBase<LocalDomainResolver> {
 public:
  LocalDomainResolverProxy(
      ServiceDiscoveryClientMdns* client_mdns,
      const std::string& domain,
      net::AddressFamily address_family,
      const LocalDomainResolver::IPAddressCallback& callback)
      : ProxyBase(client_mdns) {
    // It's safe to call |CreateLocalDomainResolver| on UI thread, because
    // |MDnsClient| is not used there. It's simplify implementation.
    set_implementation(client()->CreateLocalDomainResolver(
        domain,
        address_family,
        base::Bind(
            &LocalDomainResolverProxy::OnCallback, GetWeakPtr(), callback)));
  }

  // LocalDomainResolver methods.
  void Start() override {
    if (implementation()) {
      PostToMdnsThread(base::Bind(&LocalDomainResolver::Start,
                                  base::Unretained(implementation())));
    }
  };

 private:
  static void OnCallback(const WeakPtr& proxy,
                         const LocalDomainResolver::IPAddressCallback& callback,
                         bool a1,
                         const net::IPAddress& a2,
                         const net::IPAddress& a3) {
    DCHECK(!BrowserThread::CurrentlyOn(BrowserThread::UI));
    PostToUIThread(base::Bind(&Base::RunCallback, proxy,
                              base::Bind(callback, a1, a2, a3)));
  }

  DISALLOW_COPY_AND_ASSIGN(LocalDomainResolverProxy);
};

}  // namespace

ServiceDiscoveryClientMdns::ServiceDiscoveryClientMdns()
    : mdns_runner_(BrowserThread::GetTaskRunnerForThread(BrowserThread::IO)),
      restart_attempts_(0),
      need_delay_mdns_tasks_(true),
      weak_ptr_factory_(this) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  net::NetworkChangeNotifier::AddNetworkChangeObserver(this);
  StartNewClient();
}

std::unique_ptr<ServiceWatcher>
ServiceDiscoveryClientMdns::CreateServiceWatcher(
    const std::string& service_type,
    const ServiceWatcher::UpdatedCallback& callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  auto watcher =
      base::MakeUnique<ServiceWatcherProxy>(this, service_type, callback);
  return std::move(watcher);
}

std::unique_ptr<ServiceResolver>
ServiceDiscoveryClientMdns::CreateServiceResolver(
    const std::string& service_name,
    const ServiceResolver::ResolveCompleteCallback& callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  auto resolver =
      base::MakeUnique<ServiceResolverProxy>(this, service_name, callback);
  return std::move(resolver);
}

std::unique_ptr<LocalDomainResolver>
ServiceDiscoveryClientMdns::CreateLocalDomainResolver(
    const std::string& domain,
    net::AddressFamily address_family,
    const LocalDomainResolver::IPAddressCallback& callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  auto resolver = base::MakeUnique<LocalDomainResolverProxy>(
      this, domain, address_family, callback);
  return std::move(resolver);
}

ServiceDiscoveryClientMdns::~ServiceDiscoveryClientMdns() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  net::NetworkChangeNotifier::RemoveNetworkChangeObserver(this);
  DestroyMdns();
}

void ServiceDiscoveryClientMdns::OnNetworkChanged(
    net::NetworkChangeNotifier::ConnectionType type) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  // Only network changes resets counter.
  restart_attempts_ = 0;
  ScheduleStartNewClient();
}

void ServiceDiscoveryClientMdns::ScheduleStartNewClient() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  OnBeforeMdnsDestroy();
  if (restart_attempts_ >= kMaxRestartAttempts) {
    ReportSuccess();
    return;
  }

  base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&ServiceDiscoveryClientMdns::StartNewClient,
                     weak_ptr_factory_.GetWeakPtr()),
      base::TimeDelta::FromSeconds(kRestartDelayOnNetworkChangeSeconds *
                                   (1 << restart_attempts_)));
}

void ServiceDiscoveryClientMdns::StartNewClient() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  ++restart_attempts_;
  DestroyMdns();
  mdns_ = net::MDnsClient::CreateDefault();
  client_ = base::MakeUnique<ServiceDiscoveryClientImpl>(mdns_.get());
  base::PostTaskWithTraitsAndReplyWithResult(
      FROM_HERE, {base::TaskPriority::BACKGROUND, base::MayBlock()},
      base::BindOnce(&net::GetMDnsInterfacesToBind),
      base::BindOnce(&ServiceDiscoveryClientMdns::OnInterfaceListReady,
                     weak_ptr_factory_.GetWeakPtr()));
}

void ServiceDiscoveryClientMdns::OnInterfaceListReady(
    const net::InterfaceIndexFamilyList& interfaces) {
  mdns_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&InitMdns,
                     base::Bind(&ServiceDiscoveryClientMdns::OnMdnsInitialized,
                                weak_ptr_factory_.GetWeakPtr()),
                     interfaces, base::Unretained(mdns_.get())));
}

void ServiceDiscoveryClientMdns::OnMdnsInitialized(bool success) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  if (!success) {
    ScheduleStartNewClient();
    return;
  }
  ReportSuccess();

  // Initialization is done, no need to delay tasks.
  need_delay_mdns_tasks_ = false;
  for (Proxy& observer : proxies_)
    observer.OnNewMdnsReady();
}

void ServiceDiscoveryClientMdns::ReportSuccess() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  UMA_HISTOGRAM_COUNTS_100("LocalDiscovery.ClientRestartAttempts",
                           restart_attempts_);
}

void ServiceDiscoveryClientMdns::OnBeforeMdnsDestroy() {
  need_delay_mdns_tasks_ = true;
  weak_ptr_factory_.InvalidateWeakPtrs();
  for (Proxy& observer : proxies_)
    observer.OnMdnsDestroy();
}

void ServiceDiscoveryClientMdns::DestroyMdns() {
  OnBeforeMdnsDestroy();
  // After calling Proxy::OnMdnsDestroy(), all references to |client_| and
  // |mdns_| should be destroyed.
  if (client_)
    mdns_runner_->DeleteSoon(FROM_HERE, client_.release());
  if (mdns_)
    mdns_runner_->DeleteSoon(FROM_HERE, mdns_.release());
}

}  // namespace local_discovery
