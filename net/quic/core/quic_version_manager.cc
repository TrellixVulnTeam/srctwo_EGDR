// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/quic/core/quic_version_manager.h"

#include "net/quic/core/quic_versions.h"
#include "net/quic/platform/api/quic_flags.h"

namespace net {

QuicVersionManager::QuicVersionManager(QuicVersionVector supported_versions)
    : enable_version_41_(GetQuicFlag(FLAGS_quic_enable_version_41)),
      enable_version_40_(FLAGS_quic_reloadable_flag_quic_enable_version_40),
      enable_version_39_(FLAGS_quic_reloadable_flag_quic_enable_version_39),
      enable_version_38_(FLAGS_quic_reloadable_flag_quic_enable_version_38),
      disable_version_36_(FLAGS_quic_reloadable_flag_quic_disable_version_36),
      allowed_supported_versions_(supported_versions),
      filtered_supported_versions_(
          FilterSupportedVersions(supported_versions)) {}

QuicVersionManager::~QuicVersionManager() {}

const QuicVersionVector& QuicVersionManager::GetSupportedVersions() {
  MaybeRefilterSupportedVersions();
  return filtered_supported_versions_;
}

void QuicVersionManager::MaybeRefilterSupportedVersions() {
  if (enable_version_41_ != GetQuicFlag(FLAGS_quic_enable_version_41) ||
      enable_version_40_ != FLAGS_quic_reloadable_flag_quic_enable_version_40 ||
      enable_version_39_ != FLAGS_quic_reloadable_flag_quic_enable_version_39 ||
      enable_version_38_ != FLAGS_quic_reloadable_flag_quic_enable_version_38 ||
      disable_version_36_ !=
          FLAGS_quic_reloadable_flag_quic_disable_version_36) {
    enable_version_41_ = GetQuicFlag(FLAGS_quic_enable_version_41);
    enable_version_40_ = FLAGS_quic_reloadable_flag_quic_enable_version_40;
    enable_version_39_ = FLAGS_quic_reloadable_flag_quic_enable_version_39;
    enable_version_38_ = FLAGS_quic_reloadable_flag_quic_enable_version_38;
    disable_version_36_ = FLAGS_quic_reloadable_flag_quic_disable_version_36;
    RefilterSupportedVersions();
  }
}

void QuicVersionManager::RefilterSupportedVersions() {
  filtered_supported_versions_ =
      FilterSupportedVersions(allowed_supported_versions_);
}

}  // namespace net
