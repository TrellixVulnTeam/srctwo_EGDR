// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/reading_list/reading_list_download_service_factory.h"

#include "base/files/file_path.h"
#include "base/memory/singleton.h"
#include "components/dom_distiller/core/distiller.h"
#include "components/dom_distiller/core/distiller_url_fetcher.h"
#include "components/keyed_service/ios/browser_state_dependency_manager.h"
#include "ios/chrome/browser/bookmarks/bookmark_model_factory.h"
#include "ios/chrome/browser/browser_state/browser_state_otr_helper.h"
#include "ios/chrome/browser/browser_state/chrome_browser_state.h"
#include "ios/chrome/browser/favicon/favicon_service_factory.h"
#include "ios/chrome/browser/history/history_service_factory.h"
#include "ios/chrome/browser/reading_list/reading_list_distiller_page_factory.h"
#include "ios/chrome/browser/reading_list/reading_list_download_service.h"
#include "ios/chrome/browser/reading_list/reading_list_model_factory.h"

// static
ReadingListDownloadService*
ReadingListDownloadServiceFactory::GetForBrowserState(
    ios::ChromeBrowserState* browser_state) {
  return static_cast<ReadingListDownloadService*>(
      GetInstance()->GetServiceForBrowserState(browser_state, true));
}

// static
ReadingListDownloadServiceFactory*
ReadingListDownloadServiceFactory::GetInstance() {
  return base::Singleton<ReadingListDownloadServiceFactory>::get();
}

ReadingListDownloadServiceFactory::ReadingListDownloadServiceFactory()
    : BrowserStateKeyedServiceFactory(
          "ReadingListDownloadService",
          BrowserStateDependencyManager::GetInstance()) {
  DependsOn(ReadingListModelFactory::GetInstance());
  DependsOn(ios::FaviconServiceFactory::GetInstance());
  DependsOn(ios::HistoryServiceFactory::GetInstance());
  DependsOn(ios::BookmarkModelFactory::GetInstance());
}

ReadingListDownloadServiceFactory::~ReadingListDownloadServiceFactory() {}

std::unique_ptr<KeyedService>
ReadingListDownloadServiceFactory::BuildServiceInstanceFor(
    web::BrowserState* context) const {
  ios::ChromeBrowserState* chrome_browser_state =
      ios::ChromeBrowserState::FromBrowserState(context);

  std::unique_ptr<reading_list::ReadingListDistillerPageFactory>
      distiller_page_factory =
          base::MakeUnique<reading_list::ReadingListDistillerPageFactory>(
              context);

  auto distiller_url_fetcher_factory =
      base::MakeUnique<dom_distiller::DistillerURLFetcherFactory>(
          context->GetRequestContext());

  dom_distiller::proto::DomDistillerOptions options;
  auto distiller_factory =
      base::MakeUnique<dom_distiller::DistillerFactoryImpl>(
          std::move(distiller_url_fetcher_factory), options);

  auto reading_list_download_service =
      base::MakeUnique<ReadingListDownloadService>(
          ReadingListModelFactory::GetForBrowserState(chrome_browser_state),
          chrome_browser_state->GetPrefs(),
          chrome_browser_state->GetStatePath(),
          chrome_browser_state->GetRequestContext(),
          std::move(distiller_factory), std::move(distiller_page_factory));
  // TODO(crbug.com/703565): remove std::move() once Xcode 9.0+ is required.
  return std::move(reading_list_download_service);
}

web::BrowserState* ReadingListDownloadServiceFactory::GetBrowserStateToUse(
    web::BrowserState* context) const {
  return GetBrowserStateRedirectedInIncognito(context);
}
