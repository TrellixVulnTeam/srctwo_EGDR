// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PREFS_PREF_REGISTRY_H_
#define COMPONENTS_PREFS_PREF_REGISTRY_H_

#include <stdint.h>

#include <memory>
#include <set>

#include "base/containers/hash_tables.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "components/prefs/base_prefs_export.h"
#include "components/prefs/pref_value_map.h"

namespace base {
class Value;
}

class DefaultPrefStore;
class PrefStore;

// Preferences need to be registered with a type and default value
// before they are used.
//
// The way you use a PrefRegistry is that you register all required
// preferences on it (via one of its subclasses), then pass it as a
// construction parameter to PrefService.
//
// Currently, registrations after constructing the PrefService will
// also work, but this is being deprecated.
class COMPONENTS_PREFS_EXPORT PrefRegistry
    : public base::RefCounted<PrefRegistry> {
 public:
  // Registration flags that can be specified which impact how the pref will
  // behave or be stored. This will be passed in a bitmask when the pref is
  // registered. Subclasses of PrefRegistry can specify their own flags. Care
  // must be taken to ensure none of these overlap with the flags below.
  enum PrefRegistrationFlags : uint32_t {
    // No flags are specified.
    NO_REGISTRATION_FLAGS = 0,

    // The first 8 bits are reserved for subclasses of PrefRegistry to use.

    // This marks the pref as "lossy". There is no strict time guarantee on when
    // a lossy pref will be persisted to permanent storage when it is modified.
    LOSSY_PREF = 1 << 8,

    // Registering a pref as public allows other services to access it.
    PUBLIC = 1 << 9,
  };

  typedef PrefValueMap::const_iterator const_iterator;
  typedef base::hash_map<std::string, uint32_t> PrefRegistrationFlagsMap;

  PrefRegistry();

  // Retrieve the set of registration flags for the given preference. The return
  // value is a bitmask of PrefRegistrationFlags.
  uint32_t GetRegistrationFlags(const std::string& pref_name) const;

  // Gets the registered defaults.
  scoped_refptr<PrefStore> defaults();

  // Allows iteration over defaults.
  const_iterator begin() const;
  const_iterator end() const;

  // Changes the default value for a preference. Takes ownership of |value|.
  //
  // |pref_name| must be a previously registered preference.
  void SetDefaultPrefValue(const std::string& pref_name, base::Value* value);

  // Registers a pref owned by another service for use with the current service.
  // The owning service must register that pref with the |PUBLIC| flag.
  void RegisterForeignPref(const std::string& path);

  // Sets the default value and flags of a previously-registered foreign pref
  // value.
  void SetDefaultForeignPrefValue(const std::string& path,
                                  std::unique_ptr<base::Value> default_value,
                                  uint32_t flags);

  const std::set<std::string>& foreign_pref_keys() const {
    return foreign_pref_keys_;
  }

 protected:
  friend class base::RefCounted<PrefRegistry>;
  virtual ~PrefRegistry();

  // Used by subclasses to register a default value and registration flags for
  // a preference. |flags| is a bitmask of |PrefRegistrationFlags|.
  void RegisterPreference(const std::string& path,
                          std::unique_ptr<base::Value> default_value,
                          uint32_t flags);

  scoped_refptr<DefaultPrefStore> defaults_;

  // A map of pref name to a bitmask of PrefRegistrationFlags.
  PrefRegistrationFlagsMap registration_flags_;

  std::set<std::string> foreign_pref_keys_;

 private:
  DISALLOW_COPY_AND_ASSIGN(PrefRegistry);
};

#endif  // COMPONENTS_PREFS_PREF_REGISTRY_H_
