// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/network/network_state.h"

#include <stdint.h>

#include <memory>
#include <utility>

#include "base/i18n/streaming_utf8_validator.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "chromeos/network/tether_constants.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/cros_system_api/dbus/service_constants.h"

namespace chromeos {

namespace {

class NetworkStateTest : public testing::Test {
 public:
  NetworkStateTest() : network_state_("test_path") {
  }

 protected:
  bool SetProperty(const std::string& key, std::unique_ptr<base::Value> value) {
    const bool result = network_state_.PropertyChanged(key, *value);
    properties_.SetWithoutPathExpansion(key, std::move(value));
    return result;
  }

  bool SetStringProperty(const std::string& key, const std::string& value) {
    return SetProperty(key, base::MakeUnique<base::Value>(value));
  }

  bool SignalInitialPropertiesReceived() {
    return network_state_.InitialPropertiesReceived(properties_);
  }

  NetworkState network_state_;

 private:
  base::DictionaryValue properties_;

  DISALLOW_COPY_AND_ASSIGN(NetworkStateTest);
};

}  // namespace

// Setting kNameProperty should set network name after call to
// InitialPropertiesReceived().
TEST_F(NetworkStateTest, NameAscii) {
  EXPECT_TRUE(SetStringProperty(shill::kTypeProperty, shill::kTypeVPN));

  std::string network_setname = "Name TEST";
  EXPECT_TRUE(SetStringProperty(shill::kNameProperty, network_setname));
  EXPECT_FALSE(SignalInitialPropertiesReceived());
  EXPECT_EQ(network_state_.name(), network_setname);
}

TEST_F(NetworkStateTest, NameAsciiWithNull) {
  EXPECT_TRUE(SetStringProperty(shill::kTypeProperty, shill::kTypeVPN));

  std::string network_setname = "Name TEST\x00xxx";
  std::string network_setname_result = "Name TEST";
  EXPECT_TRUE(SetStringProperty(shill::kNameProperty, network_setname));
  EXPECT_FALSE(SignalInitialPropertiesReceived());
  EXPECT_EQ(network_state_.name(), network_setname_result);
}

// Truncates invalid UTF-8. base::Value has a DCHECK against invalid UTF-8
// strings, which is why this is a release mode only test.
#if defined(NDEBUG) && !defined(DCHECK_ALWAYS_ON)
TEST_F(NetworkStateTest, NameTruncateInvalid) {
  EXPECT_TRUE(SetStringProperty(shill::kTypeProperty, shill::kTypeVPN));

  std::string network_setname = "SSID TEST \x01\xff!";
  std::string network_setname_result = "SSID TEST \xEF\xBF\xBD\xEF\xBF\xBD!";
  EXPECT_TRUE(SetStringProperty(shill::kNameProperty, network_setname));
  EXPECT_TRUE(SignalInitialPropertiesReceived());
  EXPECT_EQ(network_state_.name(), network_setname_result);
}
#endif

// If HexSSID doesn't exist, fallback to NameProperty.
TEST_F(NetworkStateTest, SsidFromName) {
  EXPECT_TRUE(SetStringProperty(shill::kTypeProperty, shill::kTypeWifi));

  std::string wifi_utf8 = "UTF-8 \u3042\u3044\u3046";
  std::string wifi_utf8_result = "UTF-8 \xE3\x81\x82\xE3\x81\x84\xE3\x81\x86";
  EXPECT_TRUE(SetStringProperty(shill::kNameProperty, wifi_utf8));
  EXPECT_FALSE(SignalInitialPropertiesReceived());
  EXPECT_EQ(network_state_.name(), wifi_utf8_result);
}

// latin1 SSID -> UTF8 SSID (Hex)
TEST_F(NetworkStateTest, SsidLatin) {
  EXPECT_TRUE(SetStringProperty(shill::kTypeProperty, shill::kTypeWifi));

  std::string wifi_latin1 = "latin-1 \x54\xe9\x6c\xe9\x63\x6f\x6d";  // Télécom
  std::string wifi_latin1_hex =
      base::HexEncode(wifi_latin1.c_str(), wifi_latin1.length());
  std::string wifi_latin1_result = "latin-1 T\xc3\xa9\x6c\xc3\xa9\x63om";
  EXPECT_TRUE(SetStringProperty(shill::kWifiHexSsid, wifi_latin1_hex));
  EXPECT_TRUE(SignalInitialPropertiesReceived());
  EXPECT_EQ(network_state_.name(), wifi_latin1_result);
}

// Hex SSID
TEST_F(NetworkStateTest, SsidHex) {
  EXPECT_TRUE(SetStringProperty(shill::kTypeProperty, shill::kTypeWifi));

  std::string wifi_hex_result = "This is HEX SSID!";
  std::string wifi_hex =
      base::HexEncode(wifi_hex_result.c_str(), wifi_hex_result.length());
  EXPECT_TRUE(SetStringProperty(shill::kWifiHexSsid, wifi_hex));
  EXPECT_TRUE(SignalInitialPropertiesReceived());
  EXPECT_EQ(wifi_hex_result, network_state_.name());

  // Check HexSSID via network state dictionary.
  base::DictionaryValue dictionary;
  network_state_.GetStateProperties(&dictionary);
  std::string value;
  EXPECT_TRUE(
      dictionary.GetStringWithoutPathExpansion(shill::kWifiHexSsid, &value));
  EXPECT_EQ(wifi_hex, value);
}

// Non-UTF-8 SSID should be preserved in |raw_ssid_| field.
TEST_F(NetworkStateTest, SsidNonUtf8) {
  EXPECT_TRUE(SetStringProperty(shill::kTypeProperty, shill::kTypeWifi));

  std::string non_utf8_ssid = "\xc0";
  ASSERT_FALSE(base::StreamingUtf8Validator::Validate(non_utf8_ssid));

  std::vector<uint8_t> non_utf8_ssid_bytes;
  non_utf8_ssid_bytes.push_back(static_cast<uint8_t>(non_utf8_ssid.data()[0]));

  std::string wifi_hex =
      base::HexEncode(non_utf8_ssid.data(), non_utf8_ssid.size());
  EXPECT_TRUE(SetStringProperty(shill::kWifiHexSsid, wifi_hex));
  EXPECT_TRUE(SignalInitialPropertiesReceived());
  EXPECT_EQ(network_state_.raw_ssid(), non_utf8_ssid_bytes);
}

// Multiple updates for Hex SSID should work fine.
TEST_F(NetworkStateTest, SsidHexMultipleUpdates) {
  EXPECT_TRUE(SetStringProperty(shill::kTypeProperty, shill::kTypeWifi));

  std::string wifi_hex_result = "This is HEX SSID!";
  std::string wifi_hex =
      base::HexEncode(wifi_hex_result.c_str(), wifi_hex_result.length());
  EXPECT_TRUE(SetStringProperty(shill::kWifiHexSsid, wifi_hex));
  EXPECT_TRUE(SetStringProperty(shill::kWifiHexSsid, wifi_hex));
}

TEST_F(NetworkStateTest, CaptivePortalState) {
  std::string network_name = "test";
  EXPECT_TRUE(SetStringProperty(shill::kTypeProperty, shill::kTypeWifi));
  EXPECT_TRUE(SetStringProperty(shill::kNameProperty, network_name));
  std::string hex_ssid =
      base::HexEncode(network_name.c_str(), network_name.length());
  EXPECT_TRUE(SetStringProperty(shill::kWifiHexSsid, hex_ssid));

  // State != portal -> is_captive_portal == false
  EXPECT_TRUE(SetStringProperty(shill::kStateProperty, shill::kStateReady));
  SignalInitialPropertiesReceived();
  EXPECT_FALSE(network_state_.is_captive_portal());

  // State == portal, kPortalDetection* not set -> is_captive_portal = true
  EXPECT_TRUE(SetStringProperty(shill::kStateProperty, shill::kStatePortal));
  SignalInitialPropertiesReceived();
  EXPECT_TRUE(network_state_.is_captive_portal());

  // Set kPortalDetectionFailed* properties to states that should not trigger
  // is_captive_portal.
  SetStringProperty(shill::kPortalDetectionFailedPhaseProperty,
                    shill::kPortalDetectionPhaseUnknown);
  SetStringProperty(shill::kPortalDetectionFailedStatusProperty,
                    shill::kPortalDetectionStatusTimeout);
  SignalInitialPropertiesReceived();
  EXPECT_FALSE(network_state_.is_captive_portal());

  // Set just the phase property to the expected captive portal state.
  // is_captive_portal should still be false.
  SetStringProperty(shill::kPortalDetectionFailedPhaseProperty,
                    shill::kPortalDetectionPhaseContent);
  SignalInitialPropertiesReceived();
  EXPECT_FALSE(network_state_.is_captive_portal());

  // Set the status property to the expected captive portal state property.
  // is_captive_portal should now be true.
  SetStringProperty(shill::kPortalDetectionFailedStatusProperty,
                    shill::kPortalDetectionStatusFailure);
  SignalInitialPropertiesReceived();
  EXPECT_TRUE(network_state_.is_captive_portal());
}

// Third-party VPN provider.
TEST_F(NetworkStateTest, VPNThirdPartyProvider) {
  EXPECT_TRUE(SetStringProperty(shill::kTypeProperty, shill::kTypeVPN));
  EXPECT_TRUE(SetStringProperty(shill::kNameProperty, "VPN"));

  std::unique_ptr<base::DictionaryValue> provider(new base::DictionaryValue);
  provider->SetKey(shill::kTypeProperty,
                   base::Value(shill::kProviderThirdPartyVpn));
  provider->SetKey(shill::kHostProperty,
                   base::Value("third-party-vpn-provider-extension-id"));
  EXPECT_TRUE(SetProperty(shill::kProviderProperty, std::move(provider)));
  SignalInitialPropertiesReceived();
  EXPECT_EQ(network_state_.vpn_provider_type(), shill::kProviderThirdPartyVpn);
  EXPECT_EQ(network_state_.third_party_vpn_provider_extension_id(),
            "third-party-vpn-provider-extension-id");
}

TEST_F(NetworkStateTest, Visible) {
  EXPECT_FALSE(network_state_.visible());

  network_state_.set_visible(true);
  EXPECT_TRUE(network_state_.visible());

  network_state_.set_visible(false);
  EXPECT_FALSE(network_state_.visible());
}

TEST_F(NetworkStateTest, ConnectionState) {
  network_state_.set_visible(true);

  network_state_.set_connection_state(shill::kStateConfiguration);
  EXPECT_EQ(network_state_.connection_state(), shill::kStateConfiguration);
  EXPECT_TRUE(network_state_.IsConnectingState());
  EXPECT_FALSE(network_state_.IsReconnecting());

  network_state_.set_connection_state(shill::kStateOnline);
  EXPECT_EQ(network_state_.connection_state(), shill::kStateOnline);
  EXPECT_TRUE(network_state_.IsConnectedState());
  EXPECT_FALSE(network_state_.IsReconnecting());

  network_state_.set_connection_state(shill::kStateConfiguration);
  EXPECT_EQ(network_state_.connection_state(), shill::kStateConfiguration);
  EXPECT_TRUE(network_state_.IsConnectingState());
  EXPECT_TRUE(network_state_.IsReconnecting());
}

TEST_F(NetworkStateTest, ConnectionStateNotVisible) {
  network_state_.set_visible(false);

  network_state_.set_connection_state(shill::kStateConfiguration);
  EXPECT_EQ(network_state_.connection_state(), shill::kStateDisconnect);
  EXPECT_FALSE(network_state_.IsConnectingState());
  EXPECT_FALSE(network_state_.IsReconnecting());

  network_state_.set_connection_state(shill::kStateOnline);
  EXPECT_EQ(network_state_.connection_state(), shill::kStateDisconnect);
  EXPECT_FALSE(network_state_.IsConnectedState());
  EXPECT_FALSE(network_state_.IsReconnecting());

  network_state_.set_connection_state(shill::kStateConfiguration);
  EXPECT_EQ(network_state_.connection_state(), shill::kStateDisconnect);
  EXPECT_FALSE(network_state_.IsConnectingState());
  EXPECT_FALSE(network_state_.IsReconnecting());
}

TEST_F(NetworkStateTest, TetherProperties) {
  network_state_.set_type(kTypeTether);
  network_state_.set_carrier("Project Fi");
  network_state_.set_battery_percentage(85);
  network_state_.set_tether_has_connected_to_host(true);
  network_state_.set_signal_strength(75);

  base::DictionaryValue dictionary;
  network_state_.GetStateProperties(&dictionary);

  int signal_strength;
  EXPECT_TRUE(dictionary.GetIntegerWithoutPathExpansion(kTetherSignalStrength,
                                                        &signal_strength));
  EXPECT_EQ(75, signal_strength);

  int battery_percentage;
  EXPECT_TRUE(dictionary.GetIntegerWithoutPathExpansion(
      kTetherBatteryPercentage, &battery_percentage));
  EXPECT_EQ(85, battery_percentage);

  bool tether_has_connected_to_host;
  EXPECT_TRUE(dictionary.GetBooleanWithoutPathExpansion(
      kTetherHasConnectedToHost, &tether_has_connected_to_host));
  EXPECT_TRUE(tether_has_connected_to_host);

  std::string carrier;
  EXPECT_TRUE(
      dictionary.GetStringWithoutPathExpansion(kTetherCarrier, &carrier));
  EXPECT_EQ("Project Fi", carrier);
}

}  // namespace chromeos
