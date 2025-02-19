// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_POLICY_CORE_BROWSER_BROWSER_POLICY_CONNECTOR_BASE_H_
#define COMPONENTS_POLICY_CORE_BROWSER_BROWSER_POLICY_CONNECTOR_BASE_H_

#include <memory>

#include "base/macros.h"
#include "components/policy/core/browser/configuration_policy_handler_list.h"
#include "components/policy/core/common/schema.h"
#include "components/policy/core/common/schema_registry.h"
#include "components/policy/policy_export.h"

namespace policy {

class ConfigurationPolicyProvider;
class PolicyService;

// The BrowserPolicyConnectorBase keeps and initializes some core elements of
// the policy component, mainly the PolicyProviders and the PolicyService.
class POLICY_EXPORT BrowserPolicyConnectorBase {
 public:
  // Invoke Shutdown() before deleting, see below.
  virtual ~BrowserPolicyConnectorBase();

  // Stops the policy providers and cleans up the connector before it can be
  // safely deleted. This must be invoked before the destructor and while the
  // threads are still running. The policy providers are still valid but won't
  // update anymore after this call. Subclasses can override this for cleanup
  // and should call the parent method.
  virtual void Shutdown();

  // Returns true if InitPolicyProviders() has been called but Shutdown() hasn't
  // been yet.
  bool is_initialized() const { return is_initialized_; }

  // Returns a handle to the Chrome schema.
  const Schema& GetChromeSchema() const;

  // Returns the global CombinedSchemaRegistry. SchemaRegistries from Profiles
  // should be tracked by the global registry, so that the global policy
  // providers also load policies for the components of each Profile.
  CombinedSchemaRegistry* GetSchemaRegistry();

  // Returns the browser-global PolicyService, that contains policies for the
  // whole browser.
  PolicyService* GetPolicyService();

  // Returns the platform-specific policy provider, if there is one.
  ConfigurationPolicyProvider* GetPlatformProvider();

  const ConfigurationPolicyHandlerList* GetHandlerList() const;

  // Sets a |provider| that will be included in PolicyServices returned by
  // GetPolicyService. This is a static method because local state is
  // created immediately after the connector, and tests don't have a chance to
  // inject the provider otherwise. |provider| must outlive the connector, and
  // its ownership is not taken though the connector will initialize and shut it
  // down.
  static void SetPolicyProviderForTesting(
      ConfigurationPolicyProvider* provider);

 protected:
  // Builds an uninitialized BrowserPolicyConnectorBase. InitPolicyProviders()
  // should be called to create and start the policy components.
  explicit BrowserPolicyConnectorBase(
      const HandlerListFactory& handler_list_factory);

  // Finalizes the initialization of the connector. Must be called by
  // subclasses. This call can be skipped on tests that don't require the full
  // policy system running.
  void InitPolicyProviders();

  // Adds |provider| to the list of |policy_providers_|. Providers should
  // be added in decreasing order of priority.
  void AddPolicyProvider(std::unique_ptr<ConfigurationPolicyProvider> provider);

  // Same as AddPolicyProvider(), but |provider| becomes the platform provider
  // which can be retrieved by GetPlatformProvider(). This can be called at
  // most once, and uses the same priority order as AddPolicyProvider().
  void SetPlatformPolicyProvider(
      std::unique_ptr<ConfigurationPolicyProvider> provider);

 private:
  // Whether InitPolicyProviders() but not Shutdown() has been invoked.
  bool is_initialized_;

  // Used to convert policies to preferences. The providers declared below
  // may trigger policy updates during shutdown, which will result in
  // |handler_list_| being consulted for policy translation.
  // Therefore, it's important to destroy |handler_list_| after the providers.
  std::unique_ptr<ConfigurationPolicyHandlerList> handler_list_;

  // The Chrome schema. This wraps the structure generated by
  // generate_policy_source.py at compile time.
  Schema chrome_schema_;

  // The global SchemaRegistry, which will track all the other registries.
  CombinedSchemaRegistry schema_registry_;

  // The browser-global policy providers, in decreasing order of priority.
  std::vector<std::unique_ptr<ConfigurationPolicyProvider>> policy_providers_;
  ConfigurationPolicyProvider* platform_policy_provider_;

  // Must be deleted before all the policy providers.
  std::unique_ptr<PolicyService> policy_service_;

  DISALLOW_COPY_AND_ASSIGN(BrowserPolicyConnectorBase);
};

}  // namespace policy

#endif  // COMPONENTS_POLICY_CORE_BROWSER_BROWSER_POLICY_CONNECTOR_BASE_H_
