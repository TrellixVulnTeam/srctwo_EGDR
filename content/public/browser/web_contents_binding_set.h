// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_WEB_CONTENTS_BINDING_SET_H_
#define CONTENT_PUBLIC_BROWSER_WEB_CONTENTS_BINDING_SET_H_

#include <memory>
#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "content/common/content_export.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "mojo/public/cpp/bindings/associated_binding_set.h"
#include "mojo/public/cpp/bindings/associated_interface_request.h"
#include "mojo/public/cpp/bindings/scoped_interface_endpoint_handle.h"

namespace content {

class RenderFrameHost;
class WebContentsImpl;

// Base class for something which owns a mojo::AssociatedBindingSet on behalf
// of a WebContents. See WebContentsFrameBindingSet<T> below.
class CONTENT_EXPORT WebContentsBindingSet {
 public:
  class CONTENT_EXPORT Binder {
   public:
    virtual ~Binder() {}

    virtual void OnRequestForFrame(
        RenderFrameHost* render_frame_host,
        mojo::ScopedInterfaceEndpointHandle handle);
  };

  void SetBinderForTesting(std::unique_ptr<Binder> binder) {
    binder_for_testing_ = std::move(binder);
  }

  template <typename Interface>
  static WebContentsBindingSet* GetForWebContents(WebContents* web_contents) {
    return GetForWebContents(web_contents, Interface::Name_);
  }

 protected:
  WebContentsBindingSet(WebContents* web_contents,
                        const std::string& interface_name,
                        std::unique_ptr<Binder> binder);
  ~WebContentsBindingSet();

 private:
  friend class WebContentsImpl;

  static WebContentsBindingSet* GetForWebContents(WebContents* web_contents,
                                                  const char* interface_name);

  void CloseAllBindings();
  void OnRequestForFrame(RenderFrameHost* render_frame_host,
                         mojo::ScopedInterfaceEndpointHandle handle);

  const base::Closure remove_callback_;
  std::unique_ptr<Binder> binder_;
  std::unique_ptr<Binder> binder_for_testing_;

  DISALLOW_COPY_AND_ASSIGN(WebContentsBindingSet);
};

// Owns a set of Channel-associated interface bindings with frame context on
// message dispatch.
//
// To use this, a |mojom::Foo| implementation need only own an instance of
// WebContentsFrameBindingSet<mojom::Foo>. This allows remote RenderFrames to
// acquire handles to the |mojom::Foo| interface via
// RenderFrame::GetRemoteAssociatedInterfaces() and send messages here. When
// messages are dispatched to the implementation, the implementation can call
// GetCurrentTargetFrame() on this object (see below) to determine which
// frame sent the message.
//
// For example:
//
//   class FooImpl : public mojom::Foo {
//    public:
//     explicit FooImpl(WebContents* web_contents)
//         : web_contents_(web_contents), bindings_(web_contents, this) {}
//
//     // mojom::Foo:
//     void DoFoo() override {
//       if (bindings_.GetCurrentTargetFrame() == web_contents_->GetMainFrame())
//           ; // Do something interesting
//     }
//
//    private:
//     WebContents* web_contents_;
//     WebContentsFrameBindingSet<mojom::Foo> bindings_;
//   };
//
// When an instance of FooImpl is constructed over a WebContents, the mojom::Foo
// interface will be exposed to all remote RenderFrame objects. If the
// WebContents is destroyed at any point, the bindings will automatically reset
// and will cease to dispatch further incoming messages.
//
// If FooImpl is destroyed first, the bindings are automatically removed and
// future incoming interface requests for mojom::Foo will be rejected.
//
// Because this object uses Channel-associated interface bindings, all messages
// sent via these interfaces are ordered with respect to legacy Chrome IPC
// messages on the relevant IPC::Channel (i.e. the Channel between the browser
// and whatever render process hosts the sending frame.)
template <typename Interface>
class WebContentsFrameBindingSet : public WebContentsBindingSet {
 public:
  WebContentsFrameBindingSet(WebContents* web_contents, Interface* impl)
      : WebContentsBindingSet(
            web_contents,
            Interface::Name_,
            base::MakeUnique<FrameInterfaceBinder>(this, web_contents, impl)) {}
  ~WebContentsFrameBindingSet() {}

  // Returns the RenderFrameHost currently targeted by a message dispatch to
  // this interface. Must only be called during the extent of a message dispatch
  // for this interface.
  RenderFrameHost* GetCurrentTargetFrame() {
    DCHECK(current_target_frame_);
    return current_target_frame_;
  }

  void SetCurrentTargetFrameForTesting(RenderFrameHost* render_frame_host) {
    current_target_frame_ = render_frame_host;
  }

 private:
  class FrameInterfaceBinder : public Binder, public WebContentsObserver {
   public:
    FrameInterfaceBinder(WebContentsFrameBindingSet* binding_set,
                         WebContents* web_contents,
                         Interface* impl)
        : WebContentsObserver(web_contents), impl_(impl) {
      bindings_.set_pre_dispatch_handler(
          base::Bind(&WebContentsFrameBindingSet::WillDispatchForContext,
                     base::Unretained(binding_set)));
    }

    ~FrameInterfaceBinder() override {}

    // Binder:
    void OnRequestForFrame(
        RenderFrameHost* render_frame_host,
        mojo::ScopedInterfaceEndpointHandle handle) override {
      auto id = bindings_.AddBinding(
          impl_, mojo::AssociatedInterfaceRequest<Interface>(std::move(handle)),
          render_frame_host);
      frame_to_bindings_map_[render_frame_host].push_back(id);
    }

    // WebContentsObserver:
    void RenderFrameDeleted(RenderFrameHost* render_frame_host) override {
      auto it = frame_to_bindings_map_.find(render_frame_host);
      if (it == frame_to_bindings_map_.end())
        return;
      for (auto id : it->second)
        bindings_.RemoveBinding(id);
      frame_to_bindings_map_.erase(it);
    }

    Interface* const impl_;
    mojo::AssociatedBindingSet<Interface, RenderFrameHost*> bindings_;
    std::map<RenderFrameHost*, std::vector<mojo::BindingId>>
        frame_to_bindings_map_;

    DISALLOW_COPY_AND_ASSIGN(FrameInterfaceBinder);
  };

  void WillDispatchForContext(RenderFrameHost* const& frame_host) {
    current_target_frame_ = frame_host;
  }

  RenderFrameHost* current_target_frame_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(WebContentsFrameBindingSet);
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_WEB_CONTENTS_BINDING_SET_H_
