// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DBUS_UTIL_H_
#define DBUS_UTIL_H_

#include <string>

#include "dbus/dbus_export.h"

namespace dbus {

// Returns the absolute name of a member by concatanating |interface_name| and
// |member_name|. e.g.:
//    GetAbsoluteMemberName(
//        "org.freedesktop.DBus.Properties",
//        "PropertiesChanged")
//
//    => "org.freedesktop.DBus.Properties.PropertiesChanged"
//
CHROME_DBUS_EXPORT std::string GetAbsoluteMemberName(
    const std::string& interface_name,
    const std::string& member_name);

// Similar to base::DeletePointer, but takes void* as an argument.
// Used as DBusFreeFunction.
template<typename T>
void DeleteVoidPointer(void* memory) {
  delete static_cast<T*>(memory);
}

}  // namespace dbus

#endif  // DBUS_UTIL_H_
