// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('__crWeb.mojoApi');

// Note: The API should stay in sync with the IDL definitions in
// third_party/WebKit/Source/core/mojo

var Mojo = Mojo || {};

/**
 * MojoResult {number}: Result codes for Mojo operations.
 */
Mojo.RESULT_OK = 0;
Mojo.RESULT_CANCELLED = 1;
Mojo.RESULT_UNKNOWN = 2;
Mojo.RESULT_INVALID_ARGUMENT = 3;
Mojo.RESULT_DEADLINE_EXCEEDED = 4;
Mojo.RESULT_NOT_FOUND = 5;
Mojo.RESULT_ALREADY_EXISTS = 6;
Mojo.RESULT_PERMISSION_DENIED = 7;
Mojo.RESULT_RESOURCE_EXHAUSTED = 8;
Mojo.RESULT_FAILED_PRECONDITION = 9;
Mojo.RESULT_ABORTED = 10;
Mojo.RESULT_OUT_OF_RANGE = 11;
Mojo.RESULT_UNIMPLEMENTED = 12;
Mojo.RESULT_INTERNAL = 13;
Mojo.RESULT_UNAVAILABLE = 14;
Mojo.RESULT_DATA_LOSS = 15;
Mojo.RESULT_BUSY = 16;
Mojo.RESULT_SHOULD_WAIT = 17;

/**
 * Creates a message pipe.
 *
 * @return {result: !MojoResult, handle0: !MojoHandle=, handle1: !MojoHandle=}
 *     Result code and (on success) the two message pipe handles.
 */
Mojo.createMessagePipe = function() {
  var result = Mojo.internal.sendMessage({
    name: "Mojo.createMessagePipe",
    args: {}
  });
  if (result.result == Mojo.RESULT_OK) {
    result.handle0 = new MojoHandle(result.handle0);
    result.handle1 = new MojoHandle(result.handle1);
  }
  return result;
};

/**
 * Binds to the specified Mojo interface.
 * @param {string} interfaceName The interface name to connect.
 * @param {!MojoHandle} requestHandle The interface request handle.
 */
Mojo.bindInterface = function(interfaceName, requestHandle) {
  Mojo.internal.sendMessage({
    name: "Mojo.bindInterface",
    args: {
      interfaceName: interfaceName,
      requestHandle: requestHandle.takeNativeHandle_()
    }
  });
};

/**
 * @constructor
 * @param {?number=} nativeHandle An opaque number representing the underlying
 *     Mojo system resource.
 */
var MojoHandle = function(nativeHandle) {
  if (nativeHandle === undefined)
    nativeHandle = null;

  /**
   * @type {number|null}
   */
  this.nativeHandle_ = nativeHandle;
};

/**
 * Takes the native handle value. This is not part of the public API.
 * @return {?number}
 */
MojoHandle.prototype.takeNativeHandle_ = function() {
  var nativeHandle = this.nativeHandle_;
  this.nativeHandle_ = null;
  return nativeHandle;
};

/**
 * Closes the handle.
 */
MojoHandle.prototype.close = function() {
  if (this.nativeHandle_ === null)
    return;

  var nativeHandle = this.nativeHandle_;
  this.nativeHandle_ = null;
  Mojo.internal.sendMessage({
    name: "MojoHandle.close",
    args: {
      handle: nativeHandle
    }
  });
};

/**
 * Begins watching the handle for |signals| to be satisfied or unsatisfiable.
 *
 * @param {readable: boolean=, writable: boolean=, peerClosed: boolean=} signals
 *     The signals to watch.
 * @param {!function(!MojoResult)} calback Called with a result any time
 *     the watched signals become satisfied or unsatisfiable.
 *
 * @return {!MojoWatcher} A MojoWatcher instance that could be used to cancel
 *     the watch.
 */
MojoHandle.prototype.watch = function(signals, callback) {
  var HANDLE_SIGNAL_NONE = 0;
  var HANDLE_SIGNAL_READABLE = 1;
  var HANDLE_SIGNAL_WRITABLE = 2;
  var HANDLE_SIGNAL_PEER_CLOSED = 4;

  var signalsValue = HANDLE_SIGNAL_NONE;
  if (signals.readable) {
    signalsValue |= HANDLE_SIGNAL_READABLE;
  }
  if (signals.writable) {
    signalsValue |= HANDLE_SIGNAL_WRITABLE;
  }
  if (signalsValue.peerClosed) {
    signalsValue |= HANDLE_SIGNAL_PEER_CLOSED;
  }

  var watchId = Mojo.internal.sendMessage({
    name: "MojoHandle.watch",
    args: {
      handle: this.nativeHandle_,
      signals: signalsValue,
      callbackId: Mojo.internal.watchCallbacksHolder.getNextCallbackId()
    }
  });
  Mojo.internal.watchCallbacksHolder.addWatchCallback(watchId, callback);

  return new MojoWatcher(watchId);
};

/**
 * Writes a message to the message pipe.
 *
 * @param {!ArrayBufferView} buffer The message data. May be empty.
 * @param {!Array<!MojoHandle>} handles Any handles to attach. Handles are
 *     transferred and will no longer be valid. May be empty.
 * @return {!MojoResult} Result code.
 */
MojoHandle.prototype.writeMessage = function(buffer, handles) {
  var nativeHandles = handles.map(function(handle) {
    return handle.takeNativeHandle_();
  });
  return Mojo.internal.sendMessage({
    name: "MojoHandle.writeMessage",
    args: {
      handle: this.nativeHandle_,
      buffer: buffer,
      handles: nativeHandles,
    }
  });
};

/**
 * Reads a message from the message pipe.
 *
 * @return {result: !MojoResult,
 *          buffer: !ArrayBufferView=,
 *          handles: !Array<!MojoHandle>=}
 *     Result code and (on success) the data and handles recevied.
 */
MojoHandle.prototype.readMessage = function() {
  var result = Mojo.internal.sendMessage({
    name: "MojoHandle.readMessage",
    args: {
      handle: this.nativeHandle_
    }
  });

  if (result.result == Mojo.RESULT_OK) {
    result.buffer = new Uint8Array(result.buffer).buffer;
    result.handles = result.handles.map(function(handle) {
      return new MojoHandle(handle);
    });
  }
  return result;
};

/**
 * MojoWatcher identifies a watch on a MojoHandle and can be used to cancel the
 * watch.
 *
 * @constructor
 * @param {number} An opaque id representing the watch.
 */
var MojoWatcher = function(watchId) {
  this.watchId_ = watchId;
};

/*
 * Cancels a handle watch.
 */
MojoWatcher.prototype.cancel = function() {
  Mojo.internal.sendMessage({
    name: "MojoWatcher.cancel",
    args: {
      watchId: this.watchId_
    }
  });
  Mojo.internal.watchCallbacksHolder.removeWatchCallback(watchId);
};

// -----------------------------------------------------------------------------
// Mojo API implementation details. It is not part of the public API.

Mojo.internal = Mojo.internal || {};

/**
 * Synchronously sends a message to Mojo backend.
 * @param {!Object} message The message to send.
 * @return {Object=} Response from Mojo backend.
 */
Mojo.internal.sendMessage = function(message) {
  var response = window.prompt(__gCrWeb.common.JSONStringify(message));
  return response ? JSON.parse(response) : undefined;
};

/**
 * Holds callbacks for all currently active watches.
 */
Mojo.internal.watchCallbacksHolder = (function() {
  /**
   * Next callback id to be used for watch.
   * @type{number}
   */
  var nextCallbackId = 0;

  /**
   * Map where keys are callbacks ids and values are callbacks.
   * @type {!Map<number, !function(!MojoResult)>}
   */
  var callbacks = new Map();

  /**
   * Map where keys are watch ids and values are callback ids.
   * @type {!Map<number, number>}
   */
  var callbackIds = new Map();

  /**
   * Calls watch callback.
   *
   * @param {number} callbackId Callback id previously returned from
         {@code getNextCallbackId}.
   * @param {!MojoResult} mojoResult The result code to call the callback with.
   */
  var callCallback = function(callbackId, mojoResult) {
    var callback = callbacks.get(callbackId);

    // Signalling the watch is asynchronous operation and this function may be
    // called for already removed watch.
    if (callback)
      callback(mojoResult);
  };

  /**
   * Returns next callback id to be used for watch (idempotent).
   *
   * @return {number} callback id.
   */
  var getNextCallbackId = function() {
    return nextCallbackId;
  };

  /**
   * Adds callback which must be executed when the watch fires.
   *
   * @param {number} watchId The value returned from "MojoHandle.watch" Mojo
   *     backend.
   * @param {!function(!MojoResult)} callback The callback which should be
   *     executed when the watch fires.
   */
  var addWatchCallback = function(watchId, callback) {
    callbackIds.set(watchId, nextCallbackId);
    callbacks.set(nextCallbackId, callback);
    ++nextCallbackId;
  };

  /**
   * Removes callback which should no longer be executed.
   *
   * @param {!number} watchId The id to remove callback for.
   */
  var removeWatchCallback = function (watchId) {
    callbacks.delete(callbackIds.get(watchId));
    callbackIds.delete(watchId);
  };

  return {
    callCallback: callCallback,
    getNextCallbackId: getNextCallbackId,
    addWatchCallback: addWatchCallback,
    removeWatchCallback: removeWatchCallback
  };
})();

