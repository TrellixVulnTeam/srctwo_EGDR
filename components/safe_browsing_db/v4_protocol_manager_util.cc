// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/safe_browsing_db/v4_protocol_manager_util.h"

#include "base/base64.h"
#include "base/hash.h"
#include "base/metrics/histogram_macros.h"
#include "base/rand_util.h"
#include "base/sha1.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "crypto/sha2.h"
#include "net/base/escape.h"
#include "net/base/ip_address.h"
#include "net/http/http_request_headers.h"
#include "url/url_util.h"

using base::Time;
using base::TimeDelta;

namespace safe_browsing {
const base::FilePath::CharType kStoreSuffix[] = FILE_PATH_LITERAL(".store");

// The Safe Browsing V4 server URL prefix.
const char kSbV4UrlPrefix[] = "https://safebrowsing.googleapis.com/v4";

namespace {

std::string Unescape(const std::string& url) {
  std::string unescaped_str(url);
  const int kMaxLoopIterations = 1024;
  size_t old_size = 0;
  int loop_var = 0;
  do {
    old_size = unescaped_str.size();
    unescaped_str = net::UnescapeURLComponent(
        unescaped_str,
        net::UnescapeRule::SPOOFING_AND_CONTROL_CHARS |
            net::UnescapeRule::SPACES | net::UnescapeRule::PATH_SEPARATORS |
            net::UnescapeRule::URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS);
  } while (old_size != unescaped_str.size() &&
           ++loop_var <= kMaxLoopIterations);

  return unescaped_str;
}

std::string Escape(const std::string& url) {
  std::string escaped_str;
  // The escaped string is larger so allocate double the length to reduce the
  // chance of the string being grown.
  escaped_str.reserve(url.length() * 2);
  const char* kHexString = "0123456789ABCDEF";
  for (size_t i = 0; i < url.length(); i++) {
    unsigned char c = static_cast<unsigned char>(url[i]);
    if (c <= ' ' || c > '~' || c == '#' || c == '%') {
      escaped_str += '%';
      escaped_str += kHexString[c >> 4];
      escaped_str += kHexString[c & 0xf];
    } else {
      escaped_str += c;
    }
  }

  return escaped_str;
}

}  // namespace

std::ostream& operator<<(std::ostream& os, const ListIdentifier& id) {
  os << "{hash: " << id.hash() << "; platform_type: " << id.platform_type()
     << "; threat_entry_type: " << id.threat_entry_type()
     << "; threat_type: " << id.threat_type() << "}";
  return os;
}

PlatformType GetCurrentPlatformType() {
#if defined(OS_WIN)
  return WINDOWS_PLATFORM;
#elif defined(OS_LINUX)
  return LINUX_PLATFORM;
#elif defined(OS_MACOSX)
  return OSX_PLATFORM;
#else
  // This should ideally never compile but it is getting compiled on Android.
  // See: https://bugs.chromium.org/p/chromium/issues/detail?id=621647
  // TODO(vakh): Once that bug is fixed, this should be removed. If we leave
  // the platform_type empty, the server won't recognize the request and
  // return an error response which will pollute our UMA metrics.
  return LINUX_PLATFORM;
#endif
}

ListIdentifier GetCertCsdDownloadWhitelistId() {
  return ListIdentifier(GetCurrentPlatformType(), CERT, CSD_DOWNLOAD_WHITELIST);
}

ListIdentifier GetChromeExtMalwareId() {
  return ListIdentifier(CHROME_PLATFORM, CHROME_EXTENSION, MALWARE_THREAT);
}

ListIdentifier GetChromeUrlApiId() {
  return ListIdentifier(CHROME_PLATFORM, URL, API_ABUSE);
}

ListIdentifier GetChromeFilenameClientIncidentId() {
  return ListIdentifier(CHROME_PLATFORM, FILENAME, CLIENT_INCIDENT);
}

ListIdentifier GetChromeUrlClientIncidentId() {
  return ListIdentifier(CHROME_PLATFORM, URL, CLIENT_INCIDENT);
}

ListIdentifier GetIpMalwareId() {
  return ListIdentifier(GetCurrentPlatformType(), IP_RANGE, MALWARE_THREAT);
}

ListIdentifier GetUrlCsdDownloadWhitelistId() {
  return ListIdentifier(GetCurrentPlatformType(), URL, CSD_DOWNLOAD_WHITELIST);
}

ListIdentifier GetUrlCsdWhitelistId() {
  return ListIdentifier(GetCurrentPlatformType(), URL, CSD_WHITELIST);
}

ListIdentifier GetUrlMalwareId() {
  return ListIdentifier(GetCurrentPlatformType(), URL, MALWARE_THREAT);
}

ListIdentifier GetUrlMalBinId() {
  return ListIdentifier(GetCurrentPlatformType(), URL, MALICIOUS_BINARY);
}

ListIdentifier GetUrlSocEngId() {
  return ListIdentifier(GetCurrentPlatformType(), URL,
                        SOCIAL_ENGINEERING_PUBLIC);
}

ListIdentifier GetUrlSubresourceFilterId() {
  return ListIdentifier(GetCurrentPlatformType(), URL, SUBRESOURCE_FILTER);
}

ListIdentifier GetUrlUwsId() {
  return ListIdentifier(GetCurrentPlatformType(), URL, UNWANTED_SOFTWARE);
}

std::string GetUmaSuffixForStore(const base::FilePath& file_path) {
  DCHECK_EQ(kStoreSuffix, file_path.BaseName().Extension());
  return base::StringPrintf(
      ".%" PRIsFP, file_path.BaseName().RemoveExtension().value().c_str());
}

StoreAndHashPrefix::StoreAndHashPrefix(ListIdentifier list_id,
                                       const HashPrefix& hash_prefix)
    : list_id(list_id), hash_prefix(hash_prefix) {}

StoreAndHashPrefix::~StoreAndHashPrefix() {}

bool StoreAndHashPrefix::operator==(const StoreAndHashPrefix& other) const {
  return list_id == other.list_id && hash_prefix == other.hash_prefix;
}

bool StoreAndHashPrefix::operator!=(const StoreAndHashPrefix& other) const {
  return !operator==(other);
}

size_t StoreAndHashPrefix::hash() const {
  std::size_t first = list_id.hash();
  std::size_t second = std::hash<std::string>()(hash_prefix);

  return base::HashInts(first, second);
}

bool SBThreatTypeSetIsValidForCheckBrowseUrl(const SBThreatTypeSet& set) {
  for (SBThreatType type : set) {
    switch (type) {
      case SB_THREAT_TYPE_URL_PHISHING:
      case SB_THREAT_TYPE_URL_MALWARE:
      case SB_THREAT_TYPE_URL_UNWANTED:
        break;

      default:
        return false;
    }
  }
  return true;
}

bool ListIdentifier::operator==(const ListIdentifier& other) const {
  return platform_type_ == other.platform_type_ &&
         threat_entry_type_ == other.threat_entry_type_ &&
         threat_type_ == other.threat_type_;
}

bool ListIdentifier::operator!=(const ListIdentifier& other) const {
  return !operator==(other);
}

size_t ListIdentifier::hash() const {
  std::size_t first = std::hash<unsigned int>()(platform_type_);
  std::size_t second = std::hash<unsigned int>()(threat_entry_type_);
  std::size_t third = std::hash<unsigned int>()(threat_type_);

  std::size_t interim = base::HashInts(first, second);
  return base::HashInts(interim, third);
}

ListIdentifier::ListIdentifier(PlatformType platform_type,
                               ThreatEntryType threat_entry_type,
                               ThreatType threat_type)
    : platform_type_(platform_type),
      threat_entry_type_(threat_entry_type),
      threat_type_(threat_type) {
  DCHECK(PlatformType_IsValid(platform_type));
  DCHECK(ThreatEntryType_IsValid(threat_entry_type));
  DCHECK(ThreatType_IsValid(threat_type));
}

ListIdentifier::ListIdentifier(const ListUpdateResponse& response)
    : ListIdentifier(response.platform_type(),
                     response.threat_entry_type(),
                     response.threat_type()) {}

V4ProtocolConfig::V4ProtocolConfig(const std::string& client_name,
                                   bool disable_auto_update,
                                   const std::string& key_param,
                                   const std::string& version)
    : client_name(client_name),
      disable_auto_update(disable_auto_update),
      key_param(key_param),
      version(version) {}

V4ProtocolConfig::V4ProtocolConfig(const V4ProtocolConfig& other) = default;

V4ProtocolConfig::~V4ProtocolConfig() {}

// static
base::TimeDelta V4ProtocolManagerUtil::GetNextBackOffInterval(
    size_t* error_count,
    size_t* multiplier) {
  DCHECK(multiplier && error_count);
  (*error_count)++;
  if (*error_count > 1 && *error_count < 9) {
    // With error count 9 and above we will hit the 24 hour max interval.
    // Cap the multiplier here to prevent integer overflow errors.
    *multiplier *= 2;
  }
  base::TimeDelta next =
      base::TimeDelta::FromMinutes(*multiplier * (1 + base::RandDouble()) * 15);
  base::TimeDelta day = base::TimeDelta::FromHours(24);
  return next < day ? next : day;
}

// static
void V4ProtocolManagerUtil::RecordHttpResponseOrErrorCode(
    const char* metric_name,
    const net::URLRequestStatus& status,
    int response_code) {
  UMA_HISTOGRAM_SPARSE_SLOWLY(
      metric_name, status.is_success() ? response_code : status.error());
}

// static
void V4ProtocolManagerUtil::GetRequestUrlAndHeaders(
    const std::string& request_base64,
    const std::string& method_name,
    const V4ProtocolConfig& config,
    GURL* gurl,
    net::HttpRequestHeaders* headers) {
  *gurl = GURL(ComposeUrl(kSbV4UrlPrefix, method_name, request_base64,
                          config.key_param));
  UpdateHeaders(headers);
}

// static
std::string V4ProtocolManagerUtil::ComposeUrl(const std::string& prefix,
                                              const std::string& method,
                                              const std::string& request_base64,
                                              const std::string& key_param) {
  DCHECK(!prefix.empty() && !method.empty());
  std::string url = base::StringPrintf(
      "%s/%s?$req=%s&$ct=application/x-protobuf", prefix.c_str(),
      method.c_str(), request_base64.c_str());
  if (!key_param.empty()) {
    base::StringAppendF(&url, "&key=%s",
                        net::EscapeQueryParamValue(key_param, true).c_str());
  }
  return url;
}

// static
void V4ProtocolManagerUtil::UpdateHeaders(net::HttpRequestHeaders* headers) {
  // NOTE(vakh): The following header informs the envelope server (which sits in
  // front of Google's stubby server) that the received GET request should be
  // interpreted as a POST.
  headers->SetHeaderIfMissing("X-HTTP-Method-Override", "POST");
}

// static
void V4ProtocolManagerUtil::UrlToFullHashes(
    const GURL& url,
    std::vector<FullHash>* full_hashes) {
  std::string canon_host, canon_path, canon_query;
  CanonicalizeUrl(url, &canon_host, &canon_path, &canon_query);

  std::vector<std::string> hosts;
  if (url.HostIsIPAddress()) {
    hosts.push_back(url.host());
  } else {
    GenerateHostVariantsToCheck(canon_host, &hosts);
  }

  std::vector<std::string> paths;
  GeneratePathVariantsToCheck(canon_path, canon_query, &paths);
  for (const std::string& host : hosts) {
    for (const std::string& path : paths) {
      full_hashes->push_back(crypto::SHA256HashString(host + path));
    }
  }
}

// static
bool V4ProtocolManagerUtil::FullHashToHashPrefix(const FullHash& full_hash,
                                                 PrefixSize prefix_size,
                                                 HashPrefix* hash_prefix) {
  if (full_hash.size() < prefix_size) {
    return false;
  }
  *hash_prefix = full_hash.substr(0, prefix_size);
  return true;
}

// static
bool V4ProtocolManagerUtil::FullHashToSmallestHashPrefix(
    const FullHash& full_hash,
    HashPrefix* hash_prefix) {
  return FullHashToHashPrefix(full_hash, kMinHashPrefixLength, hash_prefix);
}

// static
bool V4ProtocolManagerUtil::FullHashMatchesHashPrefix(
    const FullHash& full_hash,
    const HashPrefix& hash_prefix) {
  return full_hash.compare(0, hash_prefix.length(), hash_prefix) == 0;
}

// static
void V4ProtocolManagerUtil::GenerateHostsToCheck(
    const GURL& url,
    std::vector<std::string>* hosts) {
  std::string canon_host;
  CanonicalizeUrl(url, &canon_host, NULL, NULL);
  GenerateHostVariantsToCheck(canon_host, hosts);
}

// static
void V4ProtocolManagerUtil::GeneratePathsToCheck(
    const GURL& url,
    std::vector<std::string>* paths) {
  std::string canon_path;
  std::string canon_query;
  CanonicalizeUrl(url, NULL, &canon_path, &canon_query);
  GeneratePathVariantsToCheck(canon_path, canon_query, paths);
}

// static
void V4ProtocolManagerUtil::GeneratePatternsToCheck(
    const GURL& url,
    std::vector<std::string>* urls) {
  std::string canon_host;
  std::string canon_path;
  std::string canon_query;
  CanonicalizeUrl(url, &canon_host, &canon_path, &canon_query);

  std::vector<std::string> hosts, paths;
  GenerateHostVariantsToCheck(canon_host, &hosts);
  GeneratePathVariantsToCheck(canon_path, canon_query, &paths);
  for (size_t h = 0; h < hosts.size(); ++h) {
    for (size_t p = 0; p < paths.size(); ++p) {
      urls->push_back(hosts[h] + paths[p]);
    }
  }
}

// static
void V4ProtocolManagerUtil::CanonicalizeUrl(const GURL& url,
                                            std::string* canonicalized_hostname,
                                            std::string* canonicalized_path,
                                            std::string* canonicalized_query) {
  DCHECK(url.is_valid());

  // We only canonicalize "normal" URLs.
  if (!url.IsStandard())
    return;

  // Following canonicalization steps are excluded since url parsing takes care
  // of those :-
  // 1. Remove any tab (0x09), CR (0x0d), and LF (0x0a) chars from url.
  //    (Exclude escaped version of these chars).
  // 2. Normalize hostname to 4 dot-seperated decimal values.
  // 3. Lowercase hostname.
  // 4. Resolve path sequences "/../" and "/./".

  // That leaves us with the following :-
  // 1. Remove fragment in URL.
  GURL url_without_fragment;
  GURL::Replacements f_replacements;
  f_replacements.ClearRef();
  f_replacements.ClearUsername();
  f_replacements.ClearPassword();
  url_without_fragment = url.ReplaceComponents(f_replacements);

  // 2. Do URL unescaping until no more hex encoded characters exist.
  std::string url_unescaped_str(Unescape(url_without_fragment.spec()));
  url::Parsed parsed;
  url::ParseStandardURL(url_unescaped_str.data(), url_unescaped_str.length(),
                        &parsed);

  // 3. In hostname, remove all leading and trailing dots.
  base::StringPiece host;
  if (parsed.host.len > 0)
    host.set(url_unescaped_str.data() + parsed.host.begin, parsed.host.len);

  base::StringPiece host_without_end_dots =
      base::TrimString(host, ".", base::TrimPositions::TRIM_ALL);

  // 4. In hostname, replace consecutive dots with a single dot.
  std::string host_without_consecutive_dots(
      RemoveConsecutiveChars(host_without_end_dots, '.'));

  // 5. In path, replace runs of consecutive slashes with a single slash.
  base::StringPiece path;
  if (parsed.path.len > 0)
    path.set(url_unescaped_str.data() + parsed.path.begin, parsed.path.len);
  std::string path_without_consecutive_slash(RemoveConsecutiveChars(path, '/'));

  url::Replacements<char> hp_replacements;
  hp_replacements.SetHost(
      host_without_consecutive_dots.data(),
      url::Component(0, host_without_consecutive_dots.length()));
  hp_replacements.SetPath(
      path_without_consecutive_slash.data(),
      url::Component(0, path_without_consecutive_slash.length()));

  std::string url_unescaped_with_can_hostpath;
  url::StdStringCanonOutput output(&url_unescaped_with_can_hostpath);
  url::Parsed temp_parsed;
  url::ReplaceComponents(url_unescaped_str.data(), url_unescaped_str.length(),
                         parsed, hp_replacements, NULL, &output, &temp_parsed);
  output.Complete();

  // 6. Step needed to revert escaping done in url::ReplaceComponents.
  url_unescaped_with_can_hostpath = Unescape(url_unescaped_with_can_hostpath);

  // 7. After performing all above steps, percent-escape all chars in url which
  // are <= ASCII 32, >= 127, #, %. Escapes must be uppercase hex characters.
  std::string escaped_canon_url_str(Escape(url_unescaped_with_can_hostpath));
  url::Parsed final_parsed;
  url::ParseStandardURL(escaped_canon_url_str.data(),
                        escaped_canon_url_str.length(), &final_parsed);

  if (canonicalized_hostname && final_parsed.host.len > 0) {
    *canonicalized_hostname = escaped_canon_url_str.substr(
        final_parsed.host.begin, final_parsed.host.len);
  }
  if (canonicalized_path && final_parsed.path.len > 0) {
    *canonicalized_path = escaped_canon_url_str.substr(final_parsed.path.begin,
                                                       final_parsed.path.len);
  }
  if (canonicalized_query && final_parsed.query.len > 0) {
    *canonicalized_query = escaped_canon_url_str.substr(
        final_parsed.query.begin, final_parsed.query.len);
  }
}

// static
std::string V4ProtocolManagerUtil::RemoveConsecutiveChars(base::StringPiece str,
                                                          const char c) {
  std::string output;
  // Output is at most the length of the original string.
  output.reserve(str.size());

  size_t i = 0;
  while (i < str.size()) {
    output.append(1, str[i++]);
    if (str[i - 1] == c) {
      while (i < str.size() && str[i] == c) {
        i++;
      }
    }
  }

  return output;
}

// static
void V4ProtocolManagerUtil::GenerateHostVariantsToCheck(
    const std::string& host,
    std::vector<std::string>* hosts) {
  hosts->clear();

  if (host.empty())
    return;

  // Per the Safe Browsing Protocol v2 spec, we try the host, and also up to 4
  // hostnames formed by starting with the last 5 components and successively
  // removing the leading component.  The last component isn't examined alone,
  // since it's the TLD or a subcomponent thereof.
  //
  // Note that we don't need to be clever about stopping at the "real" eTLD --
  // the data on the server side has been filtered to ensure it will not
  // blacklist a whole TLD, and it's not significantly slower on our side to
  // just check too much.
  //
  // Also note that because we have a simple blacklist, not some sort of complex
  // whitelist-in-blacklist or vice versa, it doesn't matter what order we check
  // these in.
  const size_t kMaxHostsToCheck = 4;
  bool skipped_last_component = false;
  for (std::string::const_reverse_iterator i(host.rbegin());
       i != host.rend() && hosts->size() < kMaxHostsToCheck; ++i) {
    if (*i == '.') {
      if (skipped_last_component)
        hosts->push_back(std::string(i.base(), host.end()));
      else
        skipped_last_component = true;
    }
  }
  hosts->push_back(host);
}

// static
void V4ProtocolManagerUtil::GeneratePathVariantsToCheck(
    const std::string& path,
    const std::string& query,
    std::vector<std::string>* paths) {
  paths->clear();

  if (path.empty())
    return;

  // Per the Safe Browsing Protocol v2 spec, we try the exact path with/without
  // the query parameters, and also up to 4 paths formed by starting at the root
  // and adding more path components.
  //
  // As with the hosts above, it doesn't matter what order we check these in.
  const size_t kMaxPathsToCheck = 4;
  for (std::string::const_iterator i(path.begin());
       i != path.end() && paths->size() < kMaxPathsToCheck; ++i) {
    if (*i == '/')
      paths->push_back(std::string(path.begin(), i + 1));
  }

  if (!paths->empty() && paths->back() != path)
    paths->push_back(path);

  if (!query.empty())
    paths->push_back(path + "?" + query);
}

// static
void V4ProtocolManagerUtil::SetClientInfoFromConfig(
    ClientInfo* client_info,
    const V4ProtocolConfig& config) {
  DCHECK(client_info);
  client_info->set_client_id(config.client_name);
  client_info->set_client_version(config.version);
}

// static
bool V4ProtocolManagerUtil::GetIPV6AddressFromString(
    const std::string& ip_address,
    net::IPAddress* address) {
  DCHECK(address);
  if (!address->AssignFromIPLiteral(ip_address))
    return false;
  if (address->IsIPv4())
    *address = net::ConvertIPv4ToIPv4MappedIPv6(*address);
  return address->IsIPv6();
}

// static
bool V4ProtocolManagerUtil::IPAddressToEncodedIPV6Hash(
    const std::string& ip_address,
    FullHash* hashed_encoded_ip) {
  net::IPAddress address;
  if (!GetIPV6AddressFromString(ip_address, &address)) {
    return false;
  }
  std::string packed_ip = net::IPAddressToPackedString(address);
  if (packed_ip.empty()) {
    return false;
  }

  const std::string hash = base::SHA1HashString(packed_ip);
  DCHECK_EQ(20u, hash.size());
  hashed_encoded_ip->resize(hash.size() + 1);
  hashed_encoded_ip->replace(0, hash.size(), hash);
  (*hashed_encoded_ip)[hash.size()] = static_cast<unsigned char>(128);
  return true;
}

}  // namespace safe_browsing
