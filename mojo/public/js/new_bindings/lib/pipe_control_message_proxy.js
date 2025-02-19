// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  var internal = mojo.internal;

  function constructRunOrClosePipeMessage(runOrClosePipeInput) {
    var runOrClosePipeMessageParams = new
        mojo.pipeControl2.RunOrClosePipeMessageParams();
    runOrClosePipeMessageParams.input = runOrClosePipeInput;

    var messageName = mojo.pipeControl2.kRunOrClosePipeMessageId;
    var payloadSize =
        mojo.pipeControl2.RunOrClosePipeMessageParams.encodedSize;

    var builder = new internal.MessageV0Builder(messageName, payloadSize);
    builder.encodeStruct(mojo.pipeControl2.RunOrClosePipeMessageParams,
                         runOrClosePipeMessageParams);
    var message = builder.finish();
    message.setInterfaceId(internal.kInvalidInterfaceId);
    return message;
  }

  function PipeControlMessageProxy(receiver) {
    this.receiver_ = receiver;
  }

  PipeControlMessageProxy.prototype.notifyPeerEndpointClosed = function(
      interfaceId, reason) {
    var message = this.constructPeerEndpointClosedMessage(interfaceId, reason);
    this.receiver_.accept(message);
  };

  PipeControlMessageProxy.prototype.constructPeerEndpointClosedMessage =
      function(interfaceId, reason) {
    var event = new mojo.pipeControl2.PeerAssociatedEndpointClosedEvent();
    event.id = interfaceId;
    if (reason) {
      event.disconnectReason = new mojo.pipeControl2.DisconnectReason({
          customReason: reason.customReason,
          description: reason.description});
    }
    var runOrClosePipeInput = new mojo.pipeControl2.RunOrClosePipeInput();
    runOrClosePipeInput.peerAssociatedEndpointClosedEvent = event;
    return constructRunOrClosePipeMessage(runOrClosePipeInput);
  };

  internal.PipeControlMessageProxy = PipeControlMessageProxy;
})();
