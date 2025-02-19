// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/keyed_service/ios/browser_state_keyed_service_factory.h"

#include "base/logging.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/keyed_service/ios/browser_state_dependency_manager.h"
#include "ios/web/public/browser_state.h"

void BrowserStateKeyedServiceFactory::SetTestingFactory(
    web::BrowserState* context,
    TestingFactoryFunction testing_factory) {
  KeyedServiceFactory::SetTestingFactory(
      context, reinterpret_cast<KeyedServiceFactory::TestingFactoryFunction>(
                   testing_factory));
}

KeyedService* BrowserStateKeyedServiceFactory::SetTestingFactoryAndUse(
    web::BrowserState* context,
    TestingFactoryFunction testing_factory) {
  return KeyedServiceFactory::SetTestingFactoryAndUse(
      context, reinterpret_cast<KeyedServiceFactory::TestingFactoryFunction>(
                   testing_factory));
}

BrowserStateKeyedServiceFactory::BrowserStateKeyedServiceFactory(
    const char* name,
    BrowserStateDependencyManager* manager)
    : KeyedServiceFactory(name, manager) {
}

BrowserStateKeyedServiceFactory::~BrowserStateKeyedServiceFactory() {
}

KeyedService* BrowserStateKeyedServiceFactory::GetServiceForBrowserState(
    web::BrowserState* context,
    bool create) {
  return KeyedServiceFactory::GetServiceForContext(context, create);
}

web::BrowserState* BrowserStateKeyedServiceFactory::GetBrowserStateToUse(
    web::BrowserState* context) const {
  // TODO(crbug.com/701326): This DCHECK should be moved to GetContextToUse().
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  // Safe default for Incognito mode: no service.
  if (context->IsOffTheRecord())
    return nullptr;

  return context;
}

bool BrowserStateKeyedServiceFactory::ServiceIsCreatedWithBrowserState() const {
  return KeyedServiceBaseFactory::ServiceIsCreatedWithContext();
}

bool BrowserStateKeyedServiceFactory::ServiceIsNULLWhileTesting() const {
  return KeyedServiceBaseFactory::ServiceIsNULLWhileTesting();
}

void BrowserStateKeyedServiceFactory::BrowserStateShutdown(
    web::BrowserState* context) {
  KeyedServiceFactory::ContextShutdown(context);
}

void BrowserStateKeyedServiceFactory::BrowserStateDestroyed(
    web::BrowserState* context) {
  KeyedServiceFactory::ContextDestroyed(context);
}

std::unique_ptr<KeyedService>
BrowserStateKeyedServiceFactory::BuildServiceInstanceFor(
    base::SupportsUserData* context) const {
  return BuildServiceInstanceFor(static_cast<web::BrowserState*>(context));
}

bool BrowserStateKeyedServiceFactory::IsOffTheRecord(
    base::SupportsUserData* context) const {
  return static_cast<web::BrowserState*>(context)->IsOffTheRecord();
}

base::SupportsUserData* BrowserStateKeyedServiceFactory::GetContextToUse(
    base::SupportsUserData* context) const {
  AssertContextWasntDestroyed(context);
  return GetBrowserStateToUse(static_cast<web::BrowserState*>(context));
}

bool BrowserStateKeyedServiceFactory::ServiceIsCreatedWithContext() const {
  return ServiceIsCreatedWithBrowserState();
}

void BrowserStateKeyedServiceFactory::ContextShutdown(
    base::SupportsUserData* context) {
  BrowserStateShutdown(static_cast<web::BrowserState*>(context));
}

void BrowserStateKeyedServiceFactory::ContextDestroyed(
    base::SupportsUserData* context) {
  BrowserStateDestroyed(static_cast<web::BrowserState*>(context));
}

void BrowserStateKeyedServiceFactory::RegisterPrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  RegisterBrowserStatePrefs(registry);
}
