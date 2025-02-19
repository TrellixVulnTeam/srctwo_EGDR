// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/payments/payments_client.h"

#include <memory>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "build/build_config.h"
#include "components/autofill/core/browser/autofill_data_model.h"
#include "components/autofill/core/browser/autofill_type.h"
#include "components/autofill/core/browser/credit_card.h"
#include "components/autofill/core/browser/payments/payments_request.h"
#include "components/autofill/core/browser/payments/payments_service_url.h"
#include "components/data_use_measurement/core/data_use_user_data.h"
#include "google_apis/gaia/identity_provider.h"
#include "net/base/escape.h"
#include "net/base/load_flags.h"
#include "net/http/http_status_code.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_request_context_getter.h"

namespace autofill {
namespace payments {

namespace {

const char kUnmaskCardRequestPath[] =
    "payments/apis-secure/creditcardservice/getrealpan?s7e_suffix=chromewallet";
const char kUnmaskCardRequestFormat[] =
    "requestContentType=application/json; charset=utf-8&request=%s"
    "&s7e_13_cvc=%s";

const char kGetUploadDetailsRequestPath[] =
    "payments/apis/chromepaymentsservice/getdetailsforsavecard";

const char kUploadCardRequestPath[] =
    "payments/apis-secure/chromepaymentsservice/savecard"
    "?s7e_suffix=chromewallet";
const char kUploadCardRequestFormat[] =
    "requestContentType=application/json; charset=utf-8&request=%s"
    "&s7e_1_pan=%s&s7e_13_cvc=%s";

const char kTokenServiceConsumerId[] = "wallet_client";
const char kPaymentsOAuth2Scope[] =
    "https://www.googleapis.com/auth/wallet.chrome";

GURL GetRequestUrl(const std::string& path) {
  if (base::CommandLine::ForCurrentProcess()->HasSwitch("sync-url")) {
    if (IsPaymentsProductionEnabled()) {
      LOG(ERROR) << "You are using production Payments but you specified a "
                    "--sync-url. You likely want to disable the sync sandbox "
                    "or switch to sandbox Payments. Both are controlled in "
                    "about:flags.";
    }
  } else if (!IsPaymentsProductionEnabled()) {
    LOG(ERROR) << "You are using sandbox Payments but you didn't specify a "
                  "--sync-url. You likely want to enable the sync sandbox "
                  "or switch to production Payments. Both are controlled in "
                  "about:flags.";
  }

  return GetBaseSecureUrl().Resolve(path);
}

std::unique_ptr<base::DictionaryValue> BuildRiskDictionary(
    const std::string& encoded_risk_data) {
  std::unique_ptr<base::DictionaryValue> risk_data(new base::DictionaryValue());
#if defined(OS_IOS)
  // Browser fingerprinting is not available on iOS. Instead, we generate
  // RiskAdvisoryData.
  risk_data->SetString("message_type", "RISK_ADVISORY_DATA");
  risk_data->SetString("encoding_type", "BASE_64_URL");
#else
  risk_data->SetString("message_type", "BROWSER_NATIVE_FINGERPRINTING");
  risk_data->SetString("encoding_type", "BASE_64");
#endif

  risk_data->SetString("value", encoded_risk_data);

  return risk_data;
}

void SetStringIfNotEmpty(const AutofillDataModel& profile,
                         const ServerFieldType& type,
                         const std::string& app_locale,
                         const std::string& path,
                         base::DictionaryValue* dictionary) {
  const base::string16 value = profile.GetInfo(AutofillType(type), app_locale);
  if (!value.empty())
    dictionary->SetString(path, value);
}

void AppendStringIfNotEmpty(const AutofillProfile& profile,
                            const ServerFieldType& type,
                            const std::string& app_locale,
                            base::ListValue* list) {
  const base::string16 value = profile.GetInfo(type, app_locale);
  if (!value.empty())
    list->AppendString(value);
}

// Returns a dictionary with the structure expected by Payments RPCs, containing
// each of the fields in |profile|, formatted according to |app_locale|. If
// |include_non_location_data| is false, the name and phone number in |profile|
// are not included.
std::unique_ptr<base::DictionaryValue> BuildAddressDictionary(
    const AutofillProfile& profile,
    const std::string& app_locale,
    bool include_non_location_data) {
  std::unique_ptr<base::DictionaryValue> postal_address(
      new base::DictionaryValue());

  if (include_non_location_data) {
    SetStringIfNotEmpty(profile, NAME_FULL, app_locale,
                        PaymentsClient::kRecipientName, postal_address.get());
  }

  std::unique_ptr<base::ListValue> address_lines(new base::ListValue());
  AppendStringIfNotEmpty(profile, ADDRESS_HOME_LINE1, app_locale,
                         address_lines.get());
  AppendStringIfNotEmpty(profile, ADDRESS_HOME_LINE2, app_locale,
                         address_lines.get());
  AppendStringIfNotEmpty(profile, ADDRESS_HOME_LINE3, app_locale,
                         address_lines.get());
  if (!address_lines->empty())
    postal_address->Set("address_line", std::move(address_lines));

  SetStringIfNotEmpty(profile, ADDRESS_HOME_CITY, app_locale, "locality_name",
                      postal_address.get());
  SetStringIfNotEmpty(profile, ADDRESS_HOME_STATE, app_locale,
                      "administrative_area_name", postal_address.get());
  SetStringIfNotEmpty(profile, ADDRESS_HOME_ZIP, app_locale,
                      "postal_code_number", postal_address.get());

  // Use GetRawInfo to get a country code instead of the country name:
  const base::string16 country_code = profile.GetRawInfo(ADDRESS_HOME_COUNTRY);
  if (!country_code.empty())
    postal_address->SetString("country_name_code", country_code);

  std::unique_ptr<base::DictionaryValue> address(new base::DictionaryValue());
  address->Set("postal_address", std::move(postal_address));

  if (include_non_location_data) {
    SetStringIfNotEmpty(profile, PHONE_HOME_WHOLE_NUMBER, app_locale,
                        PaymentsClient::kPhoneNumber, address.get());
  }

  return address;
}

// Populates the list of active experiments that affect either the data sent in
// payments RPCs or whether the RPCs are sent or not.
void SetActiveExperiments(const std::vector<const char*>& active_experiments,
                          base::DictionaryValue* request_dict) {
  if (active_experiments.empty())
    return;

  std::unique_ptr<base::ListValue> active_chrome_experiments(
      base::MakeUnique<base::ListValue>());
  for (const char* it : active_experiments)
    active_chrome_experiments->AppendString(it);

  request_dict->Set("active_chrome_experiments",
                    std::move(active_chrome_experiments));
}

class UnmaskCardRequest : public PaymentsRequest {
 public:
  UnmaskCardRequest(const PaymentsClient::UnmaskRequestDetails& request_details)
      : request_details_(request_details) {
    DCHECK(
        CreditCard::MASKED_SERVER_CARD == request_details.card.record_type() ||
        CreditCard::FULL_SERVER_CARD == request_details.card.record_type());
  }
  ~UnmaskCardRequest() override {}

  std::string GetRequestUrlPath() override { return kUnmaskCardRequestPath; }

  std::string GetRequestContentType() override {
    return "application/x-www-form-urlencoded";
  }

  std::string GetRequestContent() override {
    base::DictionaryValue request_dict;
    request_dict.SetString("encrypted_cvc", "__param:s7e_13_cvc");
    request_dict.SetString("credit_card_id", request_details_.card.server_id());
    request_dict.Set("risk_data_encoded",
                     BuildRiskDictionary(request_details_.risk_data));
    request_dict.Set("context", base::MakeUnique<base::DictionaryValue>());

    int value = 0;
    if (base::StringToInt(request_details_.user_response.exp_month, &value))
      request_dict.SetInteger("expiration_month", value);
    if (base::StringToInt(request_details_.user_response.exp_year, &value))
      request_dict.SetInteger("expiration_year", value);

    std::string json_request;
    base::JSONWriter::Write(request_dict, &json_request);
    std::string request_content = base::StringPrintf(
        kUnmaskCardRequestFormat,
        net::EscapeUrlEncodedData(json_request, true).c_str(),
        net::EscapeUrlEncodedData(
            base::UTF16ToASCII(request_details_.user_response.cvc), true)
            .c_str());
    VLOG(3) << "getrealpan request body: " << request_content;
    return request_content;
  }

  void ParseResponse(std::unique_ptr<base::DictionaryValue> response) override {
    response->GetString("pan", &real_pan_);
  }

  bool IsResponseComplete() override { return !real_pan_.empty(); }

  void RespondToDelegate(PaymentsClientDelegate* delegate,
                         AutofillClient::PaymentsRpcResult result) override {
    delegate->OnDidGetRealPan(result, real_pan_);
  }

 private:
  PaymentsClient::UnmaskRequestDetails request_details_;
  std::string real_pan_;
};

class GetUploadDetailsRequest : public PaymentsRequest {
 public:
  GetUploadDetailsRequest(const std::vector<AutofillProfile>& addresses,
                          const std::vector<const char*>& active_experiments,
                          const std::string& app_locale)
      : addresses_(addresses),
        active_experiments_(active_experiments),
        app_locale_(app_locale) {}
  ~GetUploadDetailsRequest() override {}

  std::string GetRequestUrlPath() override {
    return kGetUploadDetailsRequestPath;
  }

  std::string GetRequestContentType() override { return "application/json"; }

  std::string GetRequestContent() override {
    base::DictionaryValue request_dict;
    std::unique_ptr<base::DictionaryValue> context(new base::DictionaryValue());
    context->SetString("language_code", app_locale_);
    request_dict.Set("context", std::move(context));

    std::unique_ptr<base::ListValue> addresses(new base::ListValue());
    for (const AutofillProfile& profile : addresses_) {
      // These addresses are used by Payments to (1) accurately determine the
      // user's country in order to show the correct legal documents and (2) to
      // verify that the addresses are valid for their purposes so that we don't
      // offer save in a case where it would definitely fail (e.g. P.O. boxes).
      // The final parameter directs BuildAddressDictionary to omit names and
      // phone numbers, which aren't useful for these purposes.
      addresses->Append(BuildAddressDictionary(profile, app_locale_, false));
    }
    request_dict.Set("address", std::move(addresses));

    SetActiveExperiments(active_experiments_, &request_dict);

    std::string request_content;
    base::JSONWriter::Write(request_dict, &request_content);
    VLOG(3) << "getdetailsforsavecard request body: " << request_content;
    return request_content;
  }

  void ParseResponse(std::unique_ptr<base::DictionaryValue> response) override {
    response->GetString("context_token", &context_token_);
    base::DictionaryValue* unowned_legal_message;
    if (response->GetDictionary("legal_message", &unowned_legal_message))
      legal_message_ = unowned_legal_message->CreateDeepCopy();
  }

  bool IsResponseComplete() override {
    return !context_token_.empty() && legal_message_;
  }

  void RespondToDelegate(PaymentsClientDelegate* delegate,
                         AutofillClient::PaymentsRpcResult result) override {
    delegate->OnDidGetUploadDetails(result, context_token_,
                                    std::move(legal_message_));
  }

 private:
  const std::vector<AutofillProfile> addresses_;
  const std::vector<const char*> active_experiments_;
  std::string app_locale_;
  base::string16 context_token_;
  std::unique_ptr<base::DictionaryValue> legal_message_;
};

class UploadCardRequest : public PaymentsRequest {
 public:
  UploadCardRequest(const PaymentsClient::UploadRequestDetails& request_details)
      : request_details_(request_details) {}
  ~UploadCardRequest() override {}

  std::string GetRequestUrlPath() override { return kUploadCardRequestPath; }

  std::string GetRequestContentType() override {
    return "application/x-www-form-urlencoded";
  }

  std::string GetRequestContent() override {
    base::DictionaryValue request_dict;
    request_dict.SetString("encrypted_pan", "__param:s7e_1_pan");
    request_dict.SetString("encrypted_cvc", "__param:s7e_13_cvc");
    request_dict.Set("risk_data_encoded",
                     BuildRiskDictionary(request_details_.risk_data));

    const std::string& app_locale = request_details_.app_locale;
    std::unique_ptr<base::DictionaryValue> context(new base::DictionaryValue());
    context->SetString("language_code", app_locale);
    request_dict.Set("context", std::move(context));

    SetStringIfNotEmpty(request_details_.card, CREDIT_CARD_NAME_FULL,
                        app_locale, "cardholder_name", &request_dict);

    std::unique_ptr<base::ListValue> addresses(new base::ListValue());
    for (const AutofillProfile& profile : request_details_.profiles) {
      addresses->Append(BuildAddressDictionary(profile, app_locale, true));
    }
    request_dict.Set("address", std::move(addresses));

    request_dict.SetString("context_token", request_details_.context_token);

    int value = 0;
    const base::string16 exp_month = request_details_.card.GetInfo(
        AutofillType(CREDIT_CARD_EXP_MONTH), app_locale);
    const base::string16 exp_year = request_details_.card.GetInfo(
        AutofillType(CREDIT_CARD_EXP_4_DIGIT_YEAR), app_locale);
    if (base::StringToInt(exp_month, &value))
      request_dict.SetInteger("expiration_month", value);
    if (base::StringToInt(exp_year, &value))
      request_dict.SetInteger("expiration_year", value);

    SetActiveExperiments(request_details_.active_experiments, &request_dict);

    const base::string16 pan = request_details_.card.GetInfo(
        AutofillType(CREDIT_CARD_NUMBER), app_locale);
    std::string json_request;
    base::JSONWriter::Write(request_dict, &json_request);
    std::string request_content = base::StringPrintf(
        kUploadCardRequestFormat,
        net::EscapeUrlEncodedData(json_request, true).c_str(),
        net::EscapeUrlEncodedData(base::UTF16ToASCII(pan), true).c_str(),
        net::EscapeUrlEncodedData(base::UTF16ToASCII(request_details_.cvc),
                                  true)
            .c_str());
    VLOG(3) << "savecard request body: " << request_content;
    return request_content;
  }

  void ParseResponse(std::unique_ptr<base::DictionaryValue> response) override {
    response->GetString("credit_card_id", &server_id_);
  }

  bool IsResponseComplete() override { return true; }

  void RespondToDelegate(PaymentsClientDelegate* delegate,
                         AutofillClient::PaymentsRpcResult result) override {
    delegate->OnDidUploadCard(result, server_id_);
  }

 private:
  const PaymentsClient::UploadRequestDetails request_details_;
  std::string server_id_;
};

}  // namespace

const char PaymentsClient::kRecipientName[] = "recipient_name";
const char PaymentsClient::kPhoneNumber[] = "phone_number";

PaymentsClient::UnmaskRequestDetails::UnmaskRequestDetails() {}
PaymentsClient::UnmaskRequestDetails::~UnmaskRequestDetails() {}

PaymentsClient::UploadRequestDetails::UploadRequestDetails() {}
PaymentsClient::UploadRequestDetails::UploadRequestDetails(
    const UploadRequestDetails& other) = default;
PaymentsClient::UploadRequestDetails::~UploadRequestDetails() {}

PaymentsClient::PaymentsClient(net::URLRequestContextGetter* context_getter,
                               PaymentsClientDelegate* delegate)
    : OAuth2TokenService::Consumer(kTokenServiceConsumerId),
      context_getter_(context_getter),
      delegate_(delegate),
      has_retried_authorization_(false),
      weak_ptr_factory_(this) {
  DCHECK(delegate);
}

PaymentsClient::~PaymentsClient() {}

void PaymentsClient::Prepare() {
  if (access_token_.empty())
    StartTokenFetch(false);
}

void PaymentsClient::UnmaskCard(
    const PaymentsClient::UnmaskRequestDetails& request_details) {
  IssueRequest(base::MakeUnique<UnmaskCardRequest>(request_details), true);
}

void PaymentsClient::GetUploadDetails(
    const std::vector<AutofillProfile>& addresses,
    const std::vector<const char*>& active_experiments,
    const std::string& app_locale) {
  IssueRequest(base::MakeUnique<GetUploadDetailsRequest>(
                   addresses, active_experiments, app_locale),
               false);
}

void PaymentsClient::UploadCard(
    const PaymentsClient::UploadRequestDetails& request_details) {
  IssueRequest(base::MakeUnique<UploadCardRequest>(request_details), true);
}

void PaymentsClient::IssueRequest(std::unique_ptr<PaymentsRequest> request,
                                  bool authenticate) {
  request_ = std::move(request);
  has_retried_authorization_ = false;
  InitializeUrlFetcher();

  if (!authenticate)
    url_fetcher_->Start();
  else if (access_token_.empty())
    StartTokenFetch(false);
  else
    SetOAuth2TokenAndStartRequest();
}

void PaymentsClient::InitializeUrlFetcher() {
  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("payments_sync_cards", R"(
        semantics {
          sender: "Payments"
          description:
            "This service communicates with Google Payments servers to upload "
            "(save) or receive the user's credit card info."
          trigger:
            "Requests are triggered by a user action, such as selecting a "
            "masked server card from Chromium's credit card autofill dropdown, "
            "submitting a form which has credit card information, or accepting "
            "the prompt to save a credit card to Payments servers."
          data:
            "In case of save, a protocol buffer containing relevant address "
            "and credit card information which should be saved in Google "
            "Payments servers, along with user credentials. In case of load, a "
            "protocol buffer containing the id of the credit card to unmask, "
            "an encrypted cvc value, an optional updated card expiration date, "
            "and user credentials."
          destination: GOOGLE_OWNED_SERVICE
        }
        policy {
          cookies_allowed: NO
          setting:
            "Users can enable or disable this feature in Chromium settings by "
            "toggling 'Credit cards and addresses using Google Payments', "
            "under 'Advanced sync settings...'. This feature is enabled by "
            "default."
          chrome_policy {
            AutoFillEnabled {
              policy_options {mode: MANDATORY}
              AutoFillEnabled: false
            }
          }
        })");
  url_fetcher_ =
      net::URLFetcher::Create(0, GetRequestUrl(request_->GetRequestUrlPath()),
                              net::URLFetcher::POST, this, traffic_annotation);

  data_use_measurement::DataUseUserData::AttachToFetcher(
      url_fetcher_.get(), data_use_measurement::DataUseUserData::AUTOFILL);
  url_fetcher_->SetRequestContext(context_getter_.get());
  url_fetcher_->SetLoadFlags(net::LOAD_DO_NOT_SAVE_COOKIES |
                             net::LOAD_DO_NOT_SEND_COOKIES |
                             net::LOAD_DISABLE_CACHE);

  url_fetcher_->SetUploadData(request_->GetRequestContentType(),
                              request_->GetRequestContent());
}

void PaymentsClient::CancelRequest() {
  request_.reset();
  url_fetcher_.reset();
  access_token_request_.reset();
  access_token_.clear();
  has_retried_authorization_ = false;
}

void PaymentsClient::OnURLFetchComplete(const net::URLFetcher* source) {
  DCHECK_EQ(source, url_fetcher_.get());

  // |url_fetcher_|, which is aliased to |source|, might continue to be used in
  // this method, but should be freed once control leaves the method.
  std::unique_ptr<net::URLFetcher> scoped_url_fetcher(std::move(url_fetcher_));
  std::unique_ptr<base::DictionaryValue> response_dict;
  int response_code = source->GetResponseCode();
  std::string data;
  source->GetResponseAsString(&data);
  VLOG(2) << "Got data: " << data;

  AutofillClient::PaymentsRpcResult result = AutofillClient::SUCCESS;

  switch (response_code) {
    // Valid response.
    case net::HTTP_OK: {
      std::string error_code;
      std::unique_ptr<base::Value> message_value = base::JSONReader::Read(data);
      if (message_value.get() &&
          message_value->IsType(base::Value::Type::DICTIONARY)) {
        response_dict.reset(
            static_cast<base::DictionaryValue*>(message_value.release()));
        response_dict->GetString("error.code", &error_code);
        request_->ParseResponse(std::move(response_dict));
      }

      if (base::LowerCaseEqualsASCII(error_code, "internal"))
        result = AutofillClient::TRY_AGAIN_FAILURE;
      else if (!error_code.empty() || !request_->IsResponseComplete())
        result = AutofillClient::PERMANENT_FAILURE;

      break;
    }

    case net::HTTP_UNAUTHORIZED: {
      if (has_retried_authorization_) {
        result = AutofillClient::PERMANENT_FAILURE;
        break;
      }
      has_retried_authorization_ = true;

      InitializeUrlFetcher();
      StartTokenFetch(true);
      return;
    }

    // TODO(estade): is this actually how network connectivity issues are
    // reported?
    case net::HTTP_REQUEST_TIMEOUT: {
      result = AutofillClient::NETWORK_ERROR;
      break;
    }

    // Handle anything else as a generic (permanent) failure.
    default: {
      result = AutofillClient::PERMANENT_FAILURE;
      break;
    }
  }

  if (result != AutofillClient::SUCCESS) {
    VLOG(1) << "Payments returned error: " << response_code
            << " with data: " << data;
  }

  request_->RespondToDelegate(delegate_, result);
}

void PaymentsClient::OnGetTokenSuccess(
    const OAuth2TokenService::Request* request,
    const std::string& access_token,
    const base::Time& expiration_time) {
  DCHECK_EQ(request, access_token_request_.get());
  access_token_ = access_token;
  if (url_fetcher_)
    SetOAuth2TokenAndStartRequest();

  access_token_request_.reset();
}

void PaymentsClient::OnGetTokenFailure(
    const OAuth2TokenService::Request* request,
    const GoogleServiceAuthError& error) {
  DCHECK_EQ(request, access_token_request_.get());
  VLOG(1) << "Unhandled OAuth2 error: " << error.ToString();
  if (url_fetcher_) {
    url_fetcher_.reset();
    request_->RespondToDelegate(delegate_, AutofillClient::PERMANENT_FAILURE);
  }
  access_token_request_.reset();
}

void PaymentsClient::StartTokenFetch(bool invalidate_old) {
  // We're still waiting for the last request to come back.
  if (!invalidate_old && access_token_request_)
    return;

  OAuth2TokenService::ScopeSet payments_scopes;
  payments_scopes.insert(kPaymentsOAuth2Scope);
  IdentityProvider* identity = delegate_->GetIdentityProvider();
  if (invalidate_old) {
    DCHECK(!access_token_.empty());
    identity->GetTokenService()->InvalidateAccessToken(
        identity->GetActiveAccountId(), payments_scopes, access_token_);
  }
  access_token_.clear();
  access_token_request_ = identity->GetTokenService()->StartRequest(
      identity->GetActiveAccountId(), payments_scopes, this);
}

void PaymentsClient::SetOAuth2TokenAndStartRequest() {
  url_fetcher_->AddExtraRequestHeader(net::HttpRequestHeaders::kAuthorization +
                                      std::string(": Bearer ") + access_token_);

  url_fetcher_->Start();
}

}  // namespace payments
}  // namespace autofill
