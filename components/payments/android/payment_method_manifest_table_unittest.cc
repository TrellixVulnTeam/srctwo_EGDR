// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/payments/android/payment_method_manifest_table.h"

#include "base/files/scoped_temp_dir.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace payments {
namespace {

class PaymentMethodManifestTableTest : public testing::Test {
 public:
  PaymentMethodManifestTableTest() {}
  ~PaymentMethodManifestTableTest() override {}

 protected:
  void SetUp() override {
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
    file_ = temp_dir_.GetPath().AppendASCII("TestWebDatabase");

    table_.reset(new PaymentMethodManifestTable);
    db_.reset(new WebDatabase);
    db_->AddTable(table_.get());
    ASSERT_EQ(sql::INIT_OK, db_->Init(file_));
  }

  void TearDown() override {}

  base::FilePath file_;
  base::ScopedTempDir temp_dir_;
  std::unique_ptr<PaymentMethodManifestTable> table_;
  std::unique_ptr<WebDatabase> db_;

 private:
  DISALLOW_COPY_AND_ASSIGN(PaymentMethodManifestTableTest);
};

TEST_F(PaymentMethodManifestTableTest, GetNonExistManifest) {
  PaymentMethodManifestTable* payment_method_manifest_table =
      PaymentMethodManifestTable::FromWebDatabase(db_.get());
  std::vector<std::string> web_app_ids =
      payment_method_manifest_table->GetManifest("https://bobpay.com");
  ASSERT_TRUE(web_app_ids.empty());
}

TEST_F(PaymentMethodManifestTableTest, AddAndGetSingleManifest) {
  PaymentMethodManifestTable* payment_method_manifest_table =
      PaymentMethodManifestTable::FromWebDatabase(db_.get());

  std::string method_name("https://bobpay.com");
  std::vector<std::string> web_app_ids = {"com.bobpay"};
  ASSERT_TRUE(
      payment_method_manifest_table->AddManifest(method_name, web_app_ids));

  std::vector<std::string> retrieved_web_app_ids =
      payment_method_manifest_table->GetManifest(method_name);
  ASSERT_EQ(web_app_ids.size(), retrieved_web_app_ids.size());
  ASSERT_EQ(web_app_ids[0], retrieved_web_app_ids[0]);

  web_app_ids.emplace_back("com.alicepay");
  ASSERT_TRUE(
      payment_method_manifest_table->AddManifest(method_name, web_app_ids));

  retrieved_web_app_ids =
      payment_method_manifest_table->GetManifest("https://bobpay.com");
  ASSERT_EQ(web_app_ids.size(), retrieved_web_app_ids.size());
  ASSERT_TRUE(std::find(retrieved_web_app_ids.begin(),
                        retrieved_web_app_ids.end(),
                        web_app_ids[0]) != retrieved_web_app_ids.end());
  ASSERT_TRUE(std::find(retrieved_web_app_ids.begin(),
                        retrieved_web_app_ids.end(),
                        web_app_ids[1]) != retrieved_web_app_ids.end());
}

TEST_F(PaymentMethodManifestTableTest, AddAndGetMultipleManifest) {
  PaymentMethodManifestTable* payment_method_manifest_table =
      PaymentMethodManifestTable::FromWebDatabase(db_.get());

  std::string method_name_1("https://bobpay.com");
  std::string method_name_2("https://alicepay.com");
  std::vector<std::string> web_app_ids = {"com.bobpay"};
  ASSERT_TRUE(
      payment_method_manifest_table->AddManifest(method_name_1, web_app_ids));
  ASSERT_TRUE(
      payment_method_manifest_table->AddManifest(method_name_2, web_app_ids));

  std::vector<std::string> bobpay_web_app_ids =
      payment_method_manifest_table->GetManifest(method_name_1);
  ASSERT_EQ(web_app_ids.size(), bobpay_web_app_ids.size());
  ASSERT_EQ(web_app_ids[0], bobpay_web_app_ids[0]);

  std::vector<std::string> alicepay_web_app_ids =
      payment_method_manifest_table->GetManifest(method_name_2);
  ASSERT_EQ(web_app_ids.size(), alicepay_web_app_ids.size());
  ASSERT_EQ(web_app_ids[0], alicepay_web_app_ids[0]);

  web_app_ids.emplace_back("com.alicepay");
  ASSERT_TRUE(
      payment_method_manifest_table->AddManifest(method_name_1, web_app_ids));
  ASSERT_TRUE(
      payment_method_manifest_table->AddManifest(method_name_2, web_app_ids));

  bobpay_web_app_ids =
      payment_method_manifest_table->GetManifest(method_name_1);
  ASSERT_EQ(web_app_ids.size(), bobpay_web_app_ids.size());
  ASSERT_TRUE(std::find(bobpay_web_app_ids.begin(), bobpay_web_app_ids.end(),
                        web_app_ids[0]) != bobpay_web_app_ids.end());
  ASSERT_TRUE(std::find(bobpay_web_app_ids.begin(), bobpay_web_app_ids.end(),
                        web_app_ids[1]) != bobpay_web_app_ids.end());

  alicepay_web_app_ids =
      payment_method_manifest_table->GetManifest(method_name_1);
  ASSERT_EQ(web_app_ids.size(), alicepay_web_app_ids.size());
  ASSERT_TRUE(std::find(alicepay_web_app_ids.begin(),
                        alicepay_web_app_ids.end(),
                        web_app_ids[0]) != alicepay_web_app_ids.end());
  ASSERT_TRUE(std::find(alicepay_web_app_ids.begin(),
                        alicepay_web_app_ids.end(),
                        web_app_ids[1]) != alicepay_web_app_ids.end());
}

}  // namespace

}  // namespace payments
