// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_PUBLIC_CAST_SYS_INFO_H_
#define CHROMECAST_PUBLIC_CAST_SYS_INFO_H_

#include <string>
#include <vector>

namespace chromecast {

// Pure abstract interface for system information which is accessed by other
// processes as well as cast_shell browser process. All information should be
// immutable.
// It should be possible to instantiate multiple instances of CastSysInfo and
// should be able to be instantiated at any point in the startup process. Other
// processes must be able to create an instance of CastSysInfo.
class CastSysInfo {
 public:
  enum BuildType {
    BUILD_ENG,
    BUILD_BETA,
    BUILD_PRODUCTION,
  };

  virtual ~CastSysInfo() {}

  // Returns the system build type.
  virtual BuildType GetBuildType() = 0;
  // Returns release channel of system.
  virtual std::string GetSystemReleaseChannel() = 0;
  // Returns serial number of the device.
  virtual std::string GetSerialNumber() = 0;
  // Returns product code name of the device.
  virtual std::string GetProductName() = 0;
  // Returns model name of device (eg: Chromecast, Nexus Player, ...).
  virtual std::string GetDeviceModel() = 0;
  // Returns the board's name.
  virtual std::string GetBoardName() = 0;
  // Returns the revision of board (eg: 514, ...).
  virtual std::string GetBoardRevision() = 0;
  // Returns device manufacturer (eg: Google, ...).
  virtual std::string GetManufacturer() = 0;
  // Returns the system's build number (eg: 100, 20000 ...).
  // This describes system version which may be different with
  // CAST_BUILD_NUMBER.
  virtual std::string GetSystemBuildNumber() = 0;

  // Returns default country and locale baked from the factory.
  virtual std::string GetFactoryCountry() = 0;
  virtual std::string GetFactoryLocale(std::string* second_locale) = 0;

  // Returns the name of the wifi interface used to connect to the internet.
  virtual std::string GetWifiInterface() = 0;
  // Returns the name of the software AP interface.
  virtual std::string GetApInterface() = 0;

  // Returns the GL_VENDOR string of GPU driver. Must match the value that would
  // be obtained from a GL context, but implementations must *not* create a GL
  // context.
  virtual std::string GetGlVendor() = 0;
  // Returns the GL_RENDERER string of GPU driver. Must match the value that
  // would be obtained from a GL context, but implementations must *not* create
  // a GL context.
  virtual std::string GetGlRenderer() = 0;
  // Returns the GL_VERSION string of GPU driver. Must match the value that
  // would be obtained from a GL context, but implementations must *not* create
  // a GL context.
  virtual std::string GetGlVersion() = 0;
};

}  // namespace chromecast

#endif  // CHROMECAST_PUBLIC_CAST_SYS_INFO_H_
