// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/web_view/internal/translate/web_view_translate_ranker_factory.h"

#include <utility>

#include "base/memory/ptr_util.h"
#include "base/memory/singleton.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/keyed_service/ios/browser_state_dependency_manager.h"
#include "components/translate/core/browser/translate_ranker_impl.h"
#include "ios/web_view/internal/pref_names.h"
#include "ios/web_view/internal/web_view_browser_state.h"

namespace ios_web_view {

// static
WebViewTranslateRankerFactory* WebViewTranslateRankerFactory::GetInstance() {
  return base::Singleton<WebViewTranslateRankerFactory>::get();
}

// static
translate::TranslateRanker* WebViewTranslateRankerFactory::GetForBrowserState(
    WebViewBrowserState* state) {
  return static_cast<translate::TranslateRanker*>(
      GetInstance()->GetServiceForBrowserState(state, true));
}

WebViewTranslateRankerFactory::WebViewTranslateRankerFactory()
    : BrowserStateKeyedServiceFactory(
          "TranslateRankerService",
          BrowserStateDependencyManager::GetInstance()) {}

WebViewTranslateRankerFactory::~WebViewTranslateRankerFactory() {}

std::unique_ptr<KeyedService>
WebViewTranslateRankerFactory::BuildServiceInstanceFor(
    web::BrowserState* context) const {
  WebViewBrowserState* web_view_browser_state =
      WebViewBrowserState::FromBrowserState(context);
  std::unique_ptr<translate::TranslateRankerImpl> ranker =
      base::MakeUnique<translate::TranslateRankerImpl>(
          translate::TranslateRankerImpl::GetModelPath(
              web_view_browser_state->GetStatePath()),
          translate::TranslateRankerImpl::GetModelURL(),
          nullptr /* ukm::UkmRecorder */);
  // WebView has no consumer of translate ranker events, so don't generate them.
  ranker->EnableLogging(false);

  // TODO(crbug.com/703565): remove std::move() once Xcode 9.0+ is required.
  return std::move(ranker);
}

}  // namespace ios_web_view
