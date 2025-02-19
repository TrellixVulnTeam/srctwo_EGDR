/*
 * Mock implementation of mojo PresentationService.
 */

"use strict";

class MockPresentationConnection {
};

class PresentationServiceMock {
  constructor() {
    this.pendingResponse_ = null;
    this.bindingSet_ = new mojo.BindingSet(blink.mojom.PresentationService);
    this.controllerConnectionPtr_ = null;
    this.receiverConnectionRequest_ = null;

    this.interceptor_ = new MojoInterfaceInterceptor(
        blink.mojom.PresentationService.name);
    this.interceptor_.oninterfacerequest =
        e => this.bindingSet_.addBinding(this, e.handle);
    this.interceptor_.start();

    this.onSetClient = null;
  }

  reset() {
    this.bindingSet_.closeAllBindings();
    this.interceptor_.stop();
  }

  setClient(client) {
    this.client_ = client;

    if (this.onSetClient)
      this.onSetClient();
  }

  async startPresentation(urls) {
    return {
        presentationInfo: { url: urls[0], id: 'fakePresentationId' },
        error: null,
    };
  }

  async reconnectPresentation(urls) {
    return {
        presentationInfo: { url: urls[0], id: 'fakePresentationId' },
        error: null,
    };
  }

  terminate(presentationUrl, presentationId) {
    this.client_.onConnectionStateChanged(
        { url: presentationUrl, id: presentationId },
        blink.mojom.PresentationConnectionState.TERMINATED);
  }

  setPresentationConnection(
      presentation_info, controllerConnectionPtr,
      receiverConnectionRequest) {
    this.controllerConnectionPtr_ = controllerConnectionPtr;
    this.receiverConnectionRequest_ = receiverConnectionRequest;
    this.client_.onConnectionStateChanged(
        presentation_info,
        blink.mojom.PresentationConnectionState.CONNECTED);
  }

  onReceiverConnectionAvailable(
      strUrl, id, opt_controllerConnectionPtr, opt_receiverConnectionRequest) {
    const mojoUrl = new url.mojom.Url();
    mojoUrl.url = strUrl;
    var controllerConnectionPtr = opt_controllerConnectionPtr;
    if (!controllerConnectionPtr) {
      controllerConnectionPtr = new blink.mojom.PresentationConnectionPtr();
      const connectionBinding = new mojo.Binding(
          blink.mojom.PresentationConnection,
          new MockPresentationConnection(),
          mojo.makeRequest(controllerConnectionPtr));
    }

    var receiverConnectionRequest = opt_receiverConnectionRequest;
    if (!receiverConnectionRequest) {
      receiverConnectionRequest = mojo.makeRequest(
          new blink.mojom.PresentationConnectionPtr());
    }

    this.client_.onReceiverConnectionAvailable(
        { url: mojoUrl, id: id },
        controllerConnectionPtr, receiverConnectionRequest);
  }

  getControllerConnectionPtr() {
    return this.controllerConnectionPtr_;
  }

  getReceiverConnectionRequest() {
    return this.receiverConnectionRequest_;
  }
}

function waitForClick(callback, button) {
  button.addEventListener('click', callback, { once: true });

  if (!('eventSender' in window))
    return;

  const boundingRect = button.getBoundingClientRect();
  const x = boundingRect.left + boundingRect.width / 2;
  const y = boundingRect.top + boundingRect.height / 2;

  eventSender.mouseMoveTo(x, y);
  eventSender.mouseDown();
  eventSender.mouseUp();
}

let presentationServiceMock = new PresentationServiceMock();
