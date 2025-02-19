/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ResourceLoaderOptions_h
#define ResourceLoaderOptions_h

#include "platform/CrossThreadCopier.h"
#include "platform/loader/fetch/FetchInitiatorInfo.h"
#include "platform/loader/fetch/IntegrityMetadata.h"
#include "platform/weborigin/SecurityOrigin.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/RefPtr.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

enum DataBufferingPolicy : uint8_t { kBufferData, kDoNotBufferData };

enum ContentSecurityPolicyDisposition : uint8_t {
  kCheckContentSecurityPolicy,
  kDoNotCheckContentSecurityPolicy
};

enum RequestInitiatorContext : uint8_t {
  kDocumentContext,
  kWorkerContext,
};

enum SynchronousPolicy : uint8_t {
  kRequestSynchronously,
  kRequestAsynchronously
};

// Used by the DocumentThreadableLoader to turn off part of the CORS handling
// logic in the ResourceFetcher to use its own CORS handling logic.
enum CORSHandlingByResourceFetcher {
  kDisableCORSHandlingByResourceFetcher,
  kEnableCORSHandlingByResourceFetcher,
};

// Was the request generated from a "parser-inserted" element?
// https://html.spec.whatwg.org/multipage/scripting.html#parser-inserted
enum ParserDisposition : uint8_t { kParserInserted, kNotParserInserted };

enum CacheAwareLoadingEnabled : uint8_t {
  kNotCacheAwareLoadingEnabled,
  kIsCacheAwareLoadingEnabled
};

struct ResourceLoaderOptions {
  USING_FAST_MALLOC(ResourceLoaderOptions);

 public:
  ResourceLoaderOptions()
      : data_buffering_policy(kBufferData),
        content_security_policy_option(kCheckContentSecurityPolicy),
        request_initiator_context(kDocumentContext),
        synchronous_policy(kRequestAsynchronously),
        cors_handling_by_resource_fetcher(kEnableCORSHandlingByResourceFetcher),
        cors_flag(false),
        parser_disposition(kParserInserted),
        cache_aware_loading_enabled(kNotCacheAwareLoadingEnabled) {}

  FetchInitiatorInfo initiator_info;

  // ATTENTION: When adding members, update
  // CrossThreadResourceLoaderOptionsData, too.

  DataBufferingPolicy data_buffering_policy;

  ContentSecurityPolicyDisposition content_security_policy_option;
  RequestInitiatorContext request_initiator_context;
  SynchronousPolicy synchronous_policy;

  // When set to true, the ResourceFetcher suppresses part of its CORS handling
  // logic. Used by DocumentThreadableLoader which does CORS handling by
  // itself.
  CORSHandlingByResourceFetcher cors_handling_by_resource_fetcher;

  // Corresponds to the CORS flag in the Fetch spec.
  bool cors_flag;

  RefPtr<SecurityOrigin> security_origin;
  String content_security_policy_nonce;
  IntegrityMetadataSet integrity_metadata;
  ParserDisposition parser_disposition;
  CacheAwareLoadingEnabled cache_aware_loading_enabled;
};

// Encode AtomicString (in FetchInitiatorInfo) as String to cross threads.
struct CrossThreadResourceLoaderOptionsData {
  DISALLOW_NEW();
  explicit CrossThreadResourceLoaderOptionsData(
      const ResourceLoaderOptions& options)
      : data_buffering_policy(options.data_buffering_policy),
        content_security_policy_option(options.content_security_policy_option),
        initiator_info(options.initiator_info),
        request_initiator_context(options.request_initiator_context),
        synchronous_policy(options.synchronous_policy),
        cors_handling_by_resource_fetcher(
            options.cors_handling_by_resource_fetcher),
        cors_flag(options.cors_flag),
        security_origin(options.security_origin
                            ? options.security_origin->IsolatedCopy()
                            : nullptr),
        content_security_policy_nonce(
            options.content_security_policy_nonce.IsolatedCopy()),
        integrity_metadata(options.integrity_metadata),
        parser_disposition(options.parser_disposition),
        cache_aware_loading_enabled(options.cache_aware_loading_enabled) {}

  operator ResourceLoaderOptions() const {
    ResourceLoaderOptions options;
    options.data_buffering_policy = data_buffering_policy;
    options.content_security_policy_option = content_security_policy_option;
    options.initiator_info = initiator_info;
    options.request_initiator_context = request_initiator_context;
    options.synchronous_policy = synchronous_policy;
    options.cors_handling_by_resource_fetcher =
        cors_handling_by_resource_fetcher;
    options.cors_flag = cors_flag;
    options.security_origin = security_origin;
    options.content_security_policy_nonce = content_security_policy_nonce;
    options.integrity_metadata = integrity_metadata;
    options.parser_disposition = parser_disposition;
    options.cache_aware_loading_enabled = cache_aware_loading_enabled;
    return options;
  }

  DataBufferingPolicy data_buffering_policy;
  ContentSecurityPolicyDisposition content_security_policy_option;
  CrossThreadFetchInitiatorInfoData initiator_info;
  RequestInitiatorContext request_initiator_context;
  SynchronousPolicy synchronous_policy;

  CORSHandlingByResourceFetcher cors_handling_by_resource_fetcher;
  bool cors_flag;
  RefPtr<SecurityOrigin> security_origin;

  String content_security_policy_nonce;
  IntegrityMetadataSet integrity_metadata;
  ParserDisposition parser_disposition;
  CacheAwareLoadingEnabled cache_aware_loading_enabled;
};

template <>
struct CrossThreadCopier<ResourceLoaderOptions> {
  using Type = CrossThreadResourceLoaderOptionsData;
  static Type Copy(const ResourceLoaderOptions& options) {
    return CrossThreadResourceLoaderOptionsData(options);
  }
};

}  // namespace blink

#endif  // ResourceLoaderOptions_h
