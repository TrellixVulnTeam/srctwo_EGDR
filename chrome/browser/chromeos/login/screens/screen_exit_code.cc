// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/login/screens/screen_exit_code.h"

#include "base/logging.h"

namespace chromeos {

std::string ExitCodeToString(ScreenExitCode code) {
  switch (code) {
    case ScreenExitCode::NETWORK_CONNECTED:
      return "NETWORK_CONNECTED";
    case ScreenExitCode::HID_DETECTION_COMPLETED:
      return "HID_DETECTION_COMPLETED";
    case ScreenExitCode::CONNECTION_FAILED:
      return "CONNECTION_FAILED";
    case ScreenExitCode::UPDATE_INSTALLED:
      return "UPDATE_INSTALLED";
    case ScreenExitCode::UPDATE_NOUPDATE:
      return "UPDATE_NOUPDATE";
    case ScreenExitCode::UPDATE_ERROR_CHECKING_FOR_UPDATE:
      return "UPDATE_ERROR_CHECKING_FOR_UPDATE";
    case ScreenExitCode::UPDATE_ERROR_UPDATING:
      return "UPDATE_ERROR_UPDATING";
    case ScreenExitCode::USER_IMAGE_SELECTED:
      return "USER_IMAGE_SELECTED";
    case ScreenExitCode::EULA_ACCEPTED:
      return "EULA_ACCEPTED";
    case ScreenExitCode::EULA_BACK:
      return "EULA_BACK";
    case ScreenExitCode::ENTERPRISE_AUTO_ENROLLMENT_CHECK_COMPLETED:
      return "ENTERPRISE_AUTO_ENROLLMENT_CHECK_COMPLETED";
    case ScreenExitCode::ENTERPRISE_ENROLLMENT_COMPLETED:
      return "ENTERPRISE_ENROLLMENT_COMPLETED";
    case ScreenExitCode::ENTERPRISE_ENROLLMENT_BACK:
      return "ENTERPRISE_ENROLLMENT_BACK";
    case ScreenExitCode::RESET_CANCELED:
      return "RESET_CANCELED";
    case ScreenExitCode::KIOSK_AUTOLAUNCH_CANCELED:
      return "KIOSK_AUTOLAUNCH_CANCELED";
    case ScreenExitCode::KIOSK_AUTOLAUNCH_CONFIRMED:
      return "KIOSK_AUTOLAUNCH_CONFIRMED";
    case ScreenExitCode::KIOSK_ENABLE_COMPLETED:
      return "KIOSK_ENABLE_COMPLETED";
    case ScreenExitCode::TERMS_OF_SERVICE_DECLINED:
      return "TERMS_OF_SERVICE_DECLINED";
    case ScreenExitCode::TERMS_OF_SERVICE_ACCEPTED:
      return "TERMS_OF_SERVICE_ACCEPTED";
    case ScreenExitCode::WRONG_HWID_WARNING_SKIPPED:
      return "WRONG_HWID_WARNING_SKIPPED";
    case ScreenExitCode::CONTROLLER_PAIRING_FINISHED:
      return "CONTROLLER_PAIRING_FINISHED";
    case ScreenExitCode::ENABLE_DEBUGGING_FINISHED:
      return "ENABLE_DEBUGGING_FINISHED";
    case ScreenExitCode::ENABLE_DEBUGGING_CANCELED:
      return "ENABLE_DEBUGGING_CANCELED";
    case ScreenExitCode::ARC_TERMS_OF_SERVICE_SKIPPED:
      return "ARC_TERMS_OF_SERVICE_SKIPPED";
    case ScreenExitCode::ARC_TERMS_OF_SERVICE_ACCEPTED:
      return "ARC_TERMS_OF_SERVICE_ACCEPTED";
    case ScreenExitCode::UPDATE_ERROR_UPDATING_CRITICAL_UPDATE:
      return "UPDATE_ERROR_UPDATING_CRITICAL_UPDATE";
    case ScreenExitCode::VOICE_INTERACTION_VALUE_PROP_SKIPPED:
      return "VOICE_INTERACTION_VALUE_PROP_SKIPPED";
    case ScreenExitCode::VOICE_INTERACTION_VALUE_PROP_ACCEPTED:
      return "VOICE_INTERACTION_VALUE_PROP_ACCEPTED";
    case ScreenExitCode::WAIT_FOR_CONTAINER_READY_FINISHED:
      return "WAIT_FOR_CONTAINER_READY_FINISHED";
    case ScreenExitCode::EXIT_CODES_COUNT:
    default:
      NOTREACHED();
      return "";
  }
}

}  // namespace chromeos
