// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/download/resource_downloader.h"

#include "content/browser/download/download_utils.h"
#include "content/browser/url_loader_factory_getter.h"
#include "content/common/throttling_url_loader.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"

namespace content {

// This class is only used for providing the WebContents to DownloadItemImpl.
class RequestHandle : public DownloadRequestHandleInterface {
 public:
  RequestHandle(int render_process_id, int render_frame_id)
      : render_process_id_(render_process_id),
        render_frame_id_(render_frame_id) {}
  RequestHandle(RequestHandle&& other)
      : render_process_id_(other.render_process_id_),
        render_frame_id_(other.render_frame_id_) {}

  // DownloadRequestHandleInterface
  WebContents* GetWebContents() const override {
    RenderFrameHost* host =
        RenderFrameHost::FromID(render_process_id_, render_frame_id_);
    if (host)
      return WebContents::FromRenderFrameHost(host);
    return nullptr;
  }
  DownloadManager* GetDownloadManager() const override { return nullptr; }
  void PauseRequest() const override {}
  void ResumeRequest() const override {}
  void CancelRequest(bool user_cancel) const override {}

 private:
  int render_process_id_;
  int render_frame_id_;

  DISALLOW_COPY_AND_ASSIGN(RequestHandle);
};

// static
std::unique_ptr<ResourceDownloader> ResourceDownloader::BeginDownload(
    base::WeakPtr<UrlDownloadHandler::Delegate> delegate,
    std::unique_ptr<DownloadUrlParameters> download_url_parameters,
    std::unique_ptr<ResourceRequest> request,
    scoped_refptr<URLLoaderFactoryGetter> url_loader_factory_getter,
    uint32_t download_id,
    bool is_parallel_request) {
  auto downloader = base::MakeUnique<ResourceDownloader>(
      delegate, std::move(download_url_parameters), url_loader_factory_getter,
      download_id, is_parallel_request);
  downloader->Start(std::move(request));

  return downloader;
}

ResourceDownloader::ResourceDownloader(
    base::WeakPtr<UrlDownloadHandler::Delegate> delegate,
    std::unique_ptr<DownloadUrlParameters> download_url_parameters,
    scoped_refptr<URLLoaderFactoryGetter> url_loader_factory_getter,
    uint32_t download_id,
    bool is_parallel_request)
    : delegate_(delegate),
      download_url_parameters_(std::move(download_url_parameters)),
      url_loader_factory_getter_(url_loader_factory_getter),
      response_handler_(download_url_parameters_.get(),
                        this,
                        is_parallel_request),
      download_id_(download_id),
      weak_ptr_factory_(this) {}

ResourceDownloader::~ResourceDownloader() = default;

void ResourceDownloader::Start(std::unique_ptr<ResourceRequest> request) {
  mojom::URLLoaderFactoryPtr* factory =
      download_url_parameters_->url().SchemeIs(url::kBlobScheme)
          ? url_loader_factory_getter_->GetBlobFactory()
          : url_loader_factory_getter_->GetNetworkFactory();

  url_loader_ = ThrottlingURLLoader::CreateLoaderAndStart(
      factory->get(), std::vector<std::unique_ptr<URLLoaderThrottle>>(),
      0,  // routing_id
      0,  // request_id
      mojom::kURLLoadOptionSendSSLInfo | mojom::kURLLoadOptionSniffMimeType,
      *(request.get()), &response_handler_,
      download_url_parameters_->GetNetworkTrafficAnnotation());
}

void ResourceDownloader::OnResponseStarted(
    std::unique_ptr<DownloadCreateInfo> download_create_info,
    mojom::DownloadStreamHandlePtr stream_handle) {
  download_create_info->download_id = download_id_;
  if (download_url_parameters_->render_process_host_id() >= 0) {
    download_create_info->request_handle.reset(new RequestHandle(
        download_url_parameters_->render_process_host_id(),
        download_url_parameters_->render_frame_host_routing_id()));
  }
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::BindOnce(&UrlDownloadHandler::Delegate::OnUrlDownloadStarted,
                     delegate_, std::move(download_create_info),
                     base::MakeUnique<DownloadManager::InputStream>(
                         std::move(stream_handle)),
                     download_url_parameters_->callback()));
}

}  // namespace content
