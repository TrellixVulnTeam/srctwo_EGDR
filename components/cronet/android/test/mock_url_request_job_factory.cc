// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/memory/ptr_util.h"
#include "components/cronet/android/test/cronet_test_util.h"
#include "jni/MockUrlRequestJobFactory_jni.h"
#include "net/test/url_request/ssl_certificate_error_job.h"
#include "net/test/url_request/url_request_failed_job.h"
#include "net/test/url_request/url_request_hanging_read_job.h"
#include "net/test/url_request/url_request_mock_data_job.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_filter.h"
#include "net/url_request/url_request_intercepting_job_factory.h"
#include "url/gurl.h"

using base::android::JavaParamRef;
using base::android::ScopedJavaLocalRef;

namespace cronet {

// Intercept URLRequestJob creation using URLRequestFilter from
// libcronet_tests.so
class UrlInterceptorJobFactoryHandle {
 public:
  // |jcontext_adapter| points to a URLRequestContextAdapater.
  UrlInterceptorJobFactoryHandle(jlong jcontext_adapter)
      : jcontext_adapter_(jcontext_adapter) {
    TestUtil::RunAfterContextInit(
        jcontext_adapter,
        base::Bind(&UrlInterceptorJobFactoryHandle::InitOnNetworkThread,
                   base::Unretained(this)));
  }
  // Should only be called on network thread; other threads should use
  // ShutDown().
  ~UrlInterceptorJobFactoryHandle() {
    DCHECK(
        TestUtil::GetTaskRunner(jcontext_adapter_)->BelongsToCurrentThread());
    TestUtil::GetURLRequestContext(jcontext_adapter_)
        ->set_job_factory(old_job_factory_);
  }

  void ShutDown() {
    TestUtil::RunAfterContextInit(
        jcontext_adapter_,
        base::Bind(&UrlInterceptorJobFactoryHandle::ShutdownOnNetworkThread,
                   base::Unretained(this)));
  }

 private:
  void InitOnNetworkThread() {
    net::URLRequestContext* request_context =
        TestUtil::GetURLRequestContext(jcontext_adapter_);
    old_job_factory_ = request_context->job_factory();
    new_job_factory_.reset(new net::URLRequestInterceptingJobFactory(
        const_cast<net::URLRequestJobFactory*>(old_job_factory_),
        net::URLRequestFilter::GetInstance()));
    request_context->set_job_factory(new_job_factory_.get());
  }

  void ShutdownOnNetworkThread() { delete this; }

  // The URLRequestContextAdapater this object intercepts from.
  const jlong jcontext_adapter_;
  // URLRequestJobFactory previously used in URLRequestContext.
  const net::URLRequestJobFactory* old_job_factory_;
  // URLRequestJobFactory inserted during tests to intercept URLRequests with
  // libcronet's URLRequestFilter.
  std::unique_ptr<net::URLRequestInterceptingJobFactory> new_job_factory_;
};

// URL interceptors are registered with the URLRequestFilter in
// libcronet_tests.so.  However tests are run on libcronet.so.  Use the
// URLRequestFilter in libcronet_tests.so with the URLRequestContext in
// libcronet.so by installing a URLRequestInterceptingJobFactory
// that calls into libcronet_tests.so's URLRequestFilter.
jlong AddUrlInterceptors(JNIEnv* env,
                         const JavaParamRef<jclass>& jcaller,
                         jlong jcontext_adapter) {
  net::URLRequestMockDataJob::AddUrlHandler();
  net::URLRequestFailedJob::AddUrlHandler();
  net::URLRequestHangingReadJob::AddUrlHandler();
  net::SSLCertificateErrorJob::AddUrlHandler();
  return reinterpret_cast<jlong>(
      new UrlInterceptorJobFactoryHandle(jcontext_adapter));
}

// Put back the old URLRequestJobFactory into the URLRequestContext.
void RemoveUrlInterceptorJobFactory(JNIEnv* env,
                                    const JavaParamRef<jclass>& jcaller,
                                    jlong jinterceptor_handle) {
  reinterpret_cast<UrlInterceptorJobFactoryHandle*>(jinterceptor_handle)
      ->ShutDown();
}

ScopedJavaLocalRef<jstring> GetMockUrlWithFailure(
    JNIEnv* jenv,
    const JavaParamRef<jclass>& jcaller,
    jint jphase,
    jint jnet_error) {
  GURL url(net::URLRequestFailedJob::GetMockHttpUrlWithFailurePhase(
      static_cast<net::URLRequestFailedJob::FailurePhase>(jphase),
      static_cast<int>(jnet_error)));
  return base::android::ConvertUTF8ToJavaString(jenv, url.spec());
}

ScopedJavaLocalRef<jstring> GetMockUrlForData(
    JNIEnv* jenv,
    const JavaParamRef<jclass>& jcaller,
    const JavaParamRef<jstring>& jdata,
    jint jdata_repeat_count) {
  std::string data(base::android::ConvertJavaStringToUTF8(jenv, jdata));
  GURL url(net::URLRequestMockDataJob::GetMockHttpUrl(data,
                                                      jdata_repeat_count));
  return base::android::ConvertUTF8ToJavaString(jenv, url.spec());
}

ScopedJavaLocalRef<jstring> GetMockUrlForSSLCertificateError(
    JNIEnv* jenv,
    const JavaParamRef<jclass>& jcaller) {
  GURL url(net::SSLCertificateErrorJob::GetMockUrl());
  return base::android::ConvertUTF8ToJavaString(jenv, url.spec());
}

ScopedJavaLocalRef<jstring> GetMockUrlForClientCertificateRequest(
    JNIEnv* jenv,
    const JavaParamRef<jclass>& jcaller) {
  GURL url(net::URLRequestMockDataJob::GetMockUrlForClientCertificateRequest());
  return base::android::ConvertUTF8ToJavaString(jenv, url.spec());
}

ScopedJavaLocalRef<jstring> GetMockUrlForHangingRead(
    JNIEnv* jenv,
    const JavaParamRef<jclass>& jcaller) {
  GURL url(net::URLRequestHangingReadJob::GetMockHttpUrl());
  return base::android::ConvertUTF8ToJavaString(jenv, url.spec());
}

}  // namespace cronet
