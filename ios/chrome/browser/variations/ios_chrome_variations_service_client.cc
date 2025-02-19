// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/variations/ios_chrome_variations_service_client.h"

#include "base/bind.h"
#include "base/version.h"
#include "components/version_info/version_info.h"
#include "ios/chrome/browser/application_context.h"
#include "ios/chrome/common/channel_info.h"

namespace {

// Gets the version number to use for variations seed simulation. Must be called
// on a thread where IO is allowed.
base::Version GetVersionForSimulation() {
  // TODO(asvitkine): Get the version that will be used on restart instead of
  // the current version.
  return base::Version(version_info::GetVersionNumber());
}

}  // namespace

IOSChromeVariationsServiceClient::IOSChromeVariationsServiceClient() {}

IOSChromeVariationsServiceClient::~IOSChromeVariationsServiceClient() {}

std::string IOSChromeVariationsServiceClient::GetApplicationLocale() {
  return GetApplicationContext()->GetApplicationLocale();
}

base::Callback<base::Version()>
IOSChromeVariationsServiceClient::GetVersionForSimulationCallback() {
  return base::Bind(&GetVersionForSimulation);
}

net::URLRequestContextGetter*
IOSChromeVariationsServiceClient::GetURLRequestContext() {
  return GetApplicationContext()->GetSystemURLRequestContext();
}

network_time::NetworkTimeTracker*
IOSChromeVariationsServiceClient::GetNetworkTimeTracker() {
  return GetApplicationContext()->GetNetworkTimeTracker();
}

version_info::Channel IOSChromeVariationsServiceClient::GetChannel() {
  return ::GetChannel();
}

bool IOSChromeVariationsServiceClient::OverridesRestrictParameter(
    std::string* parameter) {
  return false;
}
