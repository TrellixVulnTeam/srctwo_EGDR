// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stddef.h>

#include "base/command_line.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "chrome/browser/prefs/chrome_command_line_pref_store.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/pref_names.h"
#include "components/proxy_config/proxy_config_dictionary.h"
#include "components/proxy_config/proxy_config_pref_names.h"
#include "components/ssl_config/ssl_config_prefs.h"
#include "content/public/common/content_switches.h"
#include "ui/base/ui_base_switches.h"

namespace {

const char unknown_bool[] = "unknown_switch";
const char unknown_string[] = "unknown_other_switch";

}  // namespace

class TestCommandLinePrefStore : public ChromeCommandLinePrefStore {
 public:
  explicit TestCommandLinePrefStore(base::CommandLine* cl)
      : ChromeCommandLinePrefStore(cl) {}

  bool ProxySwitchesAreValid() {
    return ValidateProxySwitches();
  }

  void VerifyProxyMode(ProxyPrefs::ProxyMode expected_mode) {
    const base::Value* value = NULL;
    ASSERT_TRUE(GetValue(proxy_config::prefs::kProxy, &value));
    ASSERT_EQ(base::Value::Type::DICTIONARY, value->GetType());
    ProxyConfigDictionary dict(
        static_cast<const base::DictionaryValue*>(value)->CreateDeepCopy());
    ProxyPrefs::ProxyMode actual_mode;
    ASSERT_TRUE(dict.GetMode(&actual_mode));
    EXPECT_EQ(expected_mode, actual_mode);
  }

  void VerifySSLCipherSuites(const char* const* ciphers,
                             size_t cipher_count) {
    const base::Value* value = NULL;
    ASSERT_TRUE(GetValue(ssl_config::prefs::kCipherSuiteBlacklist, &value));
    ASSERT_EQ(base::Value::Type::LIST, value->GetType());
    const base::ListValue* list_value =
        static_cast<const base::ListValue*>(value);
    ASSERT_EQ(cipher_count, list_value->GetSize());

    std::string cipher_string;
    for (base::ListValue::const_iterator it = list_value->begin();
         it != list_value->end(); ++it, ++ciphers) {
      ASSERT_TRUE(it->GetAsString(&cipher_string));
      EXPECT_EQ(*ciphers, cipher_string);
    }
  }

 private:
  ~TestCommandLinePrefStore() override {}
};

// Tests a simple string pref on the command line.
TEST(ChromeCommandLinePrefStoreTest, SimpleStringPref) {
  base::CommandLine cl(base::CommandLine::NO_PROGRAM);
  cl.AppendSwitchASCII(switches::kLang, "hi-MOM");
  scoped_refptr<ChromeCommandLinePrefStore> store =
      new ChromeCommandLinePrefStore(&cl);

  const base::Value* actual = NULL;
  EXPECT_TRUE(store->GetValue(prefs::kApplicationLocale, &actual));
  std::string result;
  EXPECT_TRUE(actual->GetAsString(&result));
  EXPECT_EQ("hi-MOM", result);
}

// Tests a simple boolean pref on the command line.
TEST(ChromeCommandLinePrefStoreTest, SimpleBooleanPref) {
  base::CommandLine cl(base::CommandLine::NO_PROGRAM);
  cl.AppendSwitch(switches::kNoProxyServer);
  scoped_refptr<TestCommandLinePrefStore> store =
      new TestCommandLinePrefStore(&cl);

  store->VerifyProxyMode(ProxyPrefs::MODE_DIRECT);
}

// Tests a command line with no recognized prefs.
TEST(ChromeCommandLinePrefStoreTest, NoPrefs) {
  base::CommandLine cl(base::CommandLine::NO_PROGRAM);
  cl.AppendSwitch(unknown_string);
  cl.AppendSwitchASCII(unknown_bool, "a value");
  scoped_refptr<ChromeCommandLinePrefStore> store =
      new ChromeCommandLinePrefStore(&cl);

  const base::Value* actual = NULL;
  EXPECT_FALSE(store->GetValue(unknown_bool, &actual));
  EXPECT_FALSE(store->GetValue(unknown_string, &actual));
}

// Tests a complex command line with multiple known and unknown switches.
TEST(ChromeCommandLinePrefStoreTest, MultipleSwitches) {
  base::CommandLine cl(base::CommandLine::NO_PROGRAM);
  cl.AppendSwitch(unknown_string);
  cl.AppendSwitchASCII(switches::kProxyServer, "proxy");
  cl.AppendSwitchASCII(switches::kProxyBypassList, "list");
  cl.AppendSwitchASCII(unknown_bool, "a value");
  scoped_refptr<TestCommandLinePrefStore> store =
      new TestCommandLinePrefStore(&cl);

  const base::Value* actual = NULL;
  EXPECT_FALSE(store->GetValue(unknown_bool, &actual));
  EXPECT_FALSE(store->GetValue(unknown_string, &actual));

  store->VerifyProxyMode(ProxyPrefs::MODE_FIXED_SERVERS);

  const base::Value* value = NULL;
  ASSERT_TRUE(store->GetValue(proxy_config::prefs::kProxy, &value));
  ASSERT_EQ(base::Value::Type::DICTIONARY, value->GetType());
  ProxyConfigDictionary dict(
      static_cast<const base::DictionaryValue*>(value)->CreateDeepCopy());

  std::string string_result;

  ASSERT_TRUE(dict.GetProxyServer(&string_result));
  EXPECT_EQ("proxy", string_result);

  ASSERT_TRUE(dict.GetBypassList(&string_result));
  EXPECT_EQ("list", string_result);
}

// Tests proxy switch validation.
TEST(ChromeCommandLinePrefStoreTest, ProxySwitchValidation) {
  base::CommandLine cl(base::CommandLine::NO_PROGRAM);

  // No switches.
  scoped_refptr<TestCommandLinePrefStore> store =
      new TestCommandLinePrefStore(&cl);
  EXPECT_TRUE(store->ProxySwitchesAreValid());

  // Only no-proxy.
  cl.AppendSwitch(switches::kNoProxyServer);
  scoped_refptr<TestCommandLinePrefStore> store2 =
      new TestCommandLinePrefStore(&cl);
  EXPECT_TRUE(store2->ProxySwitchesAreValid());

  // Another proxy switch too.
  cl.AppendSwitch(switches::kProxyAutoDetect);
  scoped_refptr<TestCommandLinePrefStore> store3 =
      new TestCommandLinePrefStore(&cl);
  EXPECT_FALSE(store3->ProxySwitchesAreValid());

  // All proxy switches except no-proxy.
  base::CommandLine cl2(base::CommandLine::NO_PROGRAM);
  cl2.AppendSwitch(switches::kProxyAutoDetect);
  cl2.AppendSwitchASCII(switches::kProxyServer, "server");
  cl2.AppendSwitchASCII(switches::kProxyPacUrl, "url");
  cl2.AppendSwitchASCII(switches::kProxyBypassList, "list");
  scoped_refptr<TestCommandLinePrefStore> store4 =
      new TestCommandLinePrefStore(&cl2);
  EXPECT_TRUE(store4->ProxySwitchesAreValid());
}

TEST(ChromeCommandLinePrefStoreTest, ManualProxyModeInference) {
  base::CommandLine cl1(base::CommandLine::NO_PROGRAM);
  cl1.AppendSwitch(unknown_string);
  cl1.AppendSwitchASCII(switches::kProxyServer, "proxy");
  scoped_refptr<TestCommandLinePrefStore> store1 =
      new TestCommandLinePrefStore(&cl1);
  store1->VerifyProxyMode(ProxyPrefs::MODE_FIXED_SERVERS);

  base::CommandLine cl2(base::CommandLine::NO_PROGRAM);
  cl2.AppendSwitchASCII(switches::kProxyPacUrl, "proxy");
  scoped_refptr<TestCommandLinePrefStore> store2 =
        new TestCommandLinePrefStore(&cl2);
  store2->VerifyProxyMode(ProxyPrefs::MODE_PAC_SCRIPT);

  base::CommandLine cl3(base::CommandLine::NO_PROGRAM);
  cl3.AppendSwitchASCII(switches::kProxyServer, std::string());
  scoped_refptr<TestCommandLinePrefStore> store3 =
      new TestCommandLinePrefStore(&cl3);
  store3->VerifyProxyMode(ProxyPrefs::MODE_DIRECT);
}

TEST(ChromeCommandLinePrefStoreTest, DisableSSLCipherSuites) {
  base::CommandLine cl1(base::CommandLine::NO_PROGRAM);
  cl1.AppendSwitchASCII(switches::kCipherSuiteBlacklist,
                        "0x0004,0x0005");
  scoped_refptr<TestCommandLinePrefStore> store1 =
      new TestCommandLinePrefStore(&cl1);
  const char* const expected_ciphers1[] = {
    "0x0004",
    "0x0005",
  };
  store1->VerifySSLCipherSuites(expected_ciphers1,
                                arraysize(expected_ciphers1));

  base::CommandLine cl2(base::CommandLine::NO_PROGRAM);
  cl2.AppendSwitchASCII(switches::kCipherSuiteBlacklist,
                        "0x0004, WHITESPACE_IGNORED TEST , 0x0005");
  scoped_refptr<TestCommandLinePrefStore> store2 =
      new TestCommandLinePrefStore(&cl2);
  const char* const expected_ciphers2[] = {
    "0x0004",
    "WHITESPACE_IGNORED TEST",
    "0x0005",
  };
  store2->VerifySSLCipherSuites(expected_ciphers2,
                                arraysize(expected_ciphers2));

  base::CommandLine cl3(base::CommandLine::NO_PROGRAM);
  cl3.AppendSwitchASCII(switches::kCipherSuiteBlacklist,
                        "0x0004;MOAR;0x0005");
  scoped_refptr<TestCommandLinePrefStore> store3 =
      new TestCommandLinePrefStore(&cl3);
  const char* const expected_ciphers3[] = {
    "0x0004;MOAR;0x0005"
  };
  store3->VerifySSLCipherSuites(expected_ciphers3,
                                arraysize(expected_ciphers3));
}
