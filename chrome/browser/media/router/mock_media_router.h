// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_MEDIA_ROUTER_MOCK_MEDIA_ROUTER_H_
#define CHROME_BROWSER_MEDIA_ROUTER_MOCK_MEDIA_ROUTER_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "build/build_config.h"
#include "chrome/browser/media/router/media_router_base.h"
#include "chrome/common/media_router/issue.h"
#include "chrome/common/media_router/media_route.h"
#include "chrome/common/media_router/media_sink.h"
#include "chrome/common/media_router/media_source.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "url/origin.h"
#if !defined(OS_ANDROID)
#include "chrome/browser/media/router/mojo/media_route_controller.h"
#endif  // !defined(OS_ANDROID)

namespace content {
class BrowserContext;
}

namespace media_router {

// Media Router mock class. Used for testing purposes.
class MockMediaRouter : public MediaRouterBase {
 public:
  // This method can be passed into MediaRouterFactory::SetTestingFactory() to
  // make the factory return a MockMediaRouter.
  static std::unique_ptr<KeyedService> Create(content::BrowserContext* context);

  MockMediaRouter();
  ~MockMediaRouter() override;

  // TODO(crbug.com/729950): Use MOCK_METHOD directly once GMock gets the
  // move-only type support.
  void CreateRoute(const MediaSource::Id& source,
                   const MediaSink::Id& sink_id,
                   const url::Origin& origin,
                   content::WebContents* web_contents,
                   std::vector<MediaRouteResponseCallback> callbacks,
                   base::TimeDelta timeout,
                   bool incognito) {
    CreateRouteInternal(source, sink_id, origin, web_contents, callbacks,
                        timeout, incognito);
  }
  MOCK_METHOD7(CreateRouteInternal,
               void(const MediaSource::Id& source,
                    const MediaSink::Id& sink_id,
                    const url::Origin& origin,
                    content::WebContents* web_contents,
                    std::vector<MediaRouteResponseCallback>& callbacks,
                    base::TimeDelta timeout,
                    bool incognito));

  void JoinRoute(const MediaSource::Id& source,
                 const std::string& presentation_id,
                 const url::Origin& origin,
                 content::WebContents* web_contents,
                 std::vector<MediaRouteResponseCallback> callbacks,
                 base::TimeDelta timeout,
                 bool incognito) {
    JoinRouteInternal(source, presentation_id, origin, web_contents, callbacks,
                      timeout, incognito);
  }
  MOCK_METHOD7(JoinRouteInternal,
               void(const MediaSource::Id& source,
                    const std::string& presentation_id,
                    const url::Origin& origin,
                    content::WebContents* web_contents,
                    std::vector<MediaRouteResponseCallback>& callbacks,
                    base::TimeDelta timeout,
                    bool incognito));

  void ConnectRouteByRouteId(const MediaSource::Id& source,
                             const MediaRoute::Id& route_id,
                             const url::Origin& origin,
                             content::WebContents* web_contents,
                             std::vector<MediaRouteResponseCallback> callbacks,
                             base::TimeDelta timeout,
                             bool incognito) {
    ConnectRouteByRouteIdInternal(source, route_id, origin, web_contents,
                                  callbacks, timeout, incognito);
  }
  MOCK_METHOD7(ConnectRouteByRouteIdInternal,
               void(const MediaSource::Id& source,
                    const MediaRoute::Id& route_id,
                    const url::Origin& origin,
                    content::WebContents* web_contents,
                    std::vector<MediaRouteResponseCallback>& callbacks,
                    base::TimeDelta timeout,
                    bool incognito));

  MOCK_METHOD1(DetachRoute, void(const MediaRoute::Id& route_id));
  MOCK_METHOD1(TerminateRoute, void(const MediaRoute::Id& route_id));

  void SendRouteMessage(const MediaRoute::Id& route_id,
                        const std::string& message,
                        SendRouteMessageCallback callback) {
    SendRouteMessageInternal(route_id, message, callback);
  }
  MOCK_METHOD3(SendRouteMessageInternal,
               void(const MediaRoute::Id& route_id,
                    const std::string& message,
                    SendRouteMessageCallback& callback));

  void SendRouteBinaryMessage(const MediaRoute::Id& route_id,
                              std::unique_ptr<std::vector<uint8_t>> data,
                              SendRouteMessageCallback callback) override {
    SendRouteBinaryMessageInternal(route_id, data.get(), callback);
  }
  MOCK_METHOD3(SendRouteBinaryMessageInternal,
               void(const MediaRoute::Id& route_id,
                    std::vector<uint8_t>* data,
                    SendRouteMessageCallback& callback));

  MOCK_METHOD1(AddIssue, void(const IssueInfo& issue));
  MOCK_METHOD1(ClearIssue, void(const Issue::Id& issue_id));
  MOCK_METHOD0(OnUserGesture, void());

  void SearchSinks(const MediaSink::Id& sink_id,
                   const MediaSource::Id& source_id,
                   const std::string& search_input,
                   const std::string& domain,
                   MediaSinkSearchResponseCallback sink_callback) {
    SearchSinksInternal(sink_id, source_id, search_input, domain,
                        sink_callback);
  }
  MOCK_METHOD5(SearchSinksInternal,
               void(const MediaSink::Id& sink_id,
                    const MediaSource::Id& source_id,
                    const std::string& search_input,
                    const std::string& domain,
                    MediaSinkSearchResponseCallback& sink_callback));

  MOCK_METHOD2(ProvideSinks,
               void(const std::string&, std::vector<MediaSinkInternal>));
  MOCK_METHOD1(OnPresentationSessionDetached,
               void(const MediaRoute::Id& route_id));
  std::unique_ptr<PresentationConnectionStateSubscription>
  AddPresentationConnectionStateChangedCallback(
      const MediaRoute::Id& route_id,
      const content::PresentationConnectionStateChangedCallback& callback)
      override {
    OnAddPresentationConnectionStateChangedCallbackInvoked(callback);
    return connection_state_callbacks_.Add(callback);
  }
  MOCK_CONST_METHOD0(GetCurrentRoutes, std::vector<MediaRoute>());

  MOCK_METHOD0(OnIncognitoProfileShutdown, void());
#if !defined(OS_ANDROID)
  MOCK_METHOD1(
      GetRouteController,
      scoped_refptr<MediaRouteController>(const MediaRoute::Id& route_id));
#endif  // !defined(OS_ANDROID)
  MOCK_METHOD1(OnAddPresentationConnectionStateChangedCallbackInvoked,
               void(const content::PresentationConnectionStateChangedCallback&
                        callback));
  MOCK_METHOD1(RegisterIssuesObserver, void(IssuesObserver* observer));
  MOCK_METHOD1(UnregisterIssuesObserver, void(IssuesObserver* observer));
  MOCK_METHOD1(RegisterMediaSinksObserver, bool(MediaSinksObserver* observer));
  MOCK_METHOD1(UnregisterMediaSinksObserver,
               void(MediaSinksObserver* observer));
  MOCK_METHOD1(RegisterMediaRoutesObserver,
               void(MediaRoutesObserver* observer));
  MOCK_METHOD1(UnregisterMediaRoutesObserver,
               void(MediaRoutesObserver* observer));
  MOCK_METHOD1(RegisterRouteMessageObserver,
               void(RouteMessageObserver* observer));
  MOCK_METHOD1(UnregisterRouteMessageObserver,
               void(RouteMessageObserver* observer));
#if !defined(OS_ANDROID)
  MOCK_METHOD2(DetachRouteController,
               void(const MediaRoute::Id& route_id,
                    MediaRouteController* controller));
#endif  // !defined(OS_ANDROID)

 private:
  base::CallbackList<void(
      const content::PresentationConnectionStateChangeInfo&)>
      connection_state_callbacks_;
};

}  // namespace media_router

#endif  // CHROME_BROWSER_MEDIA_ROUTER_MOCK_MEDIA_ROUTER_H_
