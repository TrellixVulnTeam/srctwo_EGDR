// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

var naclModule = null;

/**
 * A helper function to abbreviate getElementById.
 *
 * @param {string} elementId The id to get.
 * @return {Element}
 */
function $(elementId) {
  return document.getElementById(elementId);
}

/**
 * MIME type for PNaCl
 *
 * @return {string} MIME type
 */
function PNaClmimeType() {
  return 'application/x-pnacl';
}

/**
 * Check if the browser supports PNaCl.
 *
 * @return {bool}
 */
function browserSupportsPNaCl() {
  var mimetype = PNaClmimeType();
  return navigator.mimeTypes[mimetype] !== undefined;
}

/**
 * Get the URL for Google Cloud Storage.
 *
 * @param {string} name The relative path to the file.
 * @return {string}
 */
function getDataURL(name) {
  var revision = '236779';
  var baseUrl = '//storage.googleapis.com/gonacl/demos/publish/';
  return baseUrl + revision + '/cube/' + name;
}

/**
 * Create the Native Client <embed> element as a child of the DOM element
 * named "listener".
 *
 * @param {string} name The name of the example.
 * @param {number} width The width to create the plugin.
 * @param {number} height The height to create the plugin.
 * @param {Object} attrs Dictionary of attributes to set on the module.
 */
function createNaClModule(name, width, height, attrs) {
  var moduleEl = document.createElement('embed');
  moduleEl.setAttribute('name', 'nacl_module');
  moduleEl.setAttribute('id', 'nacl_module');
  moduleEl.setAttribute('width', width);
  moduleEl.setAttribute('height', height);
  moduleEl.setAttribute('path', '');
  moduleEl.setAttribute('src', getDataURL(name + '.nmf'));
  moduleEl.setAttribute('type', PNaClmimeType());

  // Add any optional arguments
  if (attrs) {
    for (var key in attrs) {
      moduleEl.setAttribute(key, attrs[key]);
    }
  }

  // The <EMBED> element is wrapped inside a <DIV>, which has both a 'load'
  // and a 'message' event listener attached.  This wrapping method is used
  // instead of attaching the event listeners directly to the <EMBED> element
  // to ensure that the listeners are active before the NaCl module 'load'
  // event fires.
  var listenerDiv = $('listener');
  listenerDiv.appendChild(moduleEl);
}

/**
 * Add the default event listeners to the element with id "listener".
 */
function attachDefaultListeners() {
  var listenerDiv = $('listener');
  listenerDiv.addEventListener('load', moduleDidLoad, true);
  listenerDiv.addEventListener('error', moduleLoadError, true);
  listenerDiv.addEventListener('progress', moduleLoadProgress, true);
  listenerDiv.addEventListener('message', handleMessage, true);
  listenerDiv.addEventListener('crash', handleCrash, true);
  attachListeners();
}

/**
 * Called when the Browser can not communicate with the Module
 *
 * This event listener is registered in attachDefaultListeners above.
 *
 * @param {Object} event
 */
function handleCrash(event) {
  if (naclModule.exitStatus == -1) {
    updateStatus('CRASHED');
  } else {
    updateStatus('EXITED [' + naclModule.exitStatus + ']');
  }
}

/**
 * Called when the NaCl module is loaded.
 *
 * This event listener is registered in attachDefaultListeners above.
 */
function moduleDidLoad() {
  var bar = $('progress-bar');
  bar.style.width = 100;
  naclModule = $('nacl_module');
  hideStatus();
}

/**
 * Hide the status field and progress bar.
 */
function hideStatus() {
  $('loading-cover').style.display = 'none';
}

/**
 * Called when the plugin fails to load.
 *
 * @param {Object} event
 */
function moduleLoadError(event) {
  updateStatus('Load failed.');
}

/**
 * Called when the plugin reports progress events.
 *
 * @param {Object} event
 */
function moduleLoadProgress(event) {
  $('progress').style.display = 'block';

  var loadPercent = 0.0;
  var bar = $('progress-bar');

  if (event.lengthComputable && event.total > 0) {
    loadPercent = event.loaded / event.total * 100.0;
  } else {
    // The total length is not yet known.
    loadPercent = 10;
  }
  bar.style.width = loadPercent + "%";
}

/**
 * If the element with id 'statusField' exists, then set its HTML to the status
 * message as well.
 *
 * @param {string} opt_message The message to set.
 */
function updateStatus(opt_message) {
  var statusField = $('statusField');
  if (statusField) {
    statusField.style.display = 'block';
    statusField.textContent = opt_message;
  }
}

/**
 * Add event listeners after the NaCl module has loaded.  These listeners will
 * forward messages to the NaCl module via postMessage()
 */
function attachListeners() {
  $('xAngle').addEventListener('change', postAngleMessage);
  $('yAngle').addEventListener('change', postAngleMessage);
  $('animateOff').addEventListener('click', function() {
    $('animateOn').checked = '';
    naclModule.postMessage(false);
  });
  $('animateOn').addEventListener('click', function() {
    $('animateOff').checked = '';
    naclModule.postMessage(true);
  });
}

function postAngleMessage() {
  var xAngle = parseFloat($('xAngle').value);
  var yAngle = parseFloat($('yAngle').value);
  naclModule.postMessage([xAngle, yAngle]);
}

/**
 * Load a texture and send pixel data down to NaCl module.
 * @param {string} name
 */
function loadTexture(name) {
  // Load image from jpg, decompress into canvas.
  var img = new Image();
  img.onload = function() {
    var graph = document.createElement('canvas');
    graph.width = img.width;
    graph.height = img.height;
    var context = graph.getContext('2d');
    context.drawImage(img, 0, 0);
    var imageData = context.getImageData(0, 0, img.width, img.height);
    // Send NaCl module the raw image data obtained from canvas.
    naclModule.postMessage({'message' : 'texture',
                            'name' : name,
                            'width' : img.width,
                            'height' : img.height,
                            'data' : imageData.data.buffer});
  };
  // A cross-origin request to an image is "tainted", and cannot be read into a
  // canvas without specifying this. See
  // https://developer.mozilla.org/en-US/docs/HTML/CORS_Enabled_Image
  img.crossOrigin = 'Anonymous';
  img.src = getDataURL(name);
}

/**
 * Handle a message coming from the NaCl module.
 * @param {Object} message_event
 */
function handleMessage(event) {
  if (event.data instanceof Array) {
    if (event.data.length != 2)
      return;
    var xAngle = event.data[0];
    var yAngle = event.data[1];
    $('xAngle').value = xAngle;
    $('yAngle').value = yAngle;
  } else if (typeof(event.data) === 'number') {
    $('fps').textContent = event.data.toFixed(1);
  }
}

/**
 * Listen for the DOM content to be loaded. This event is fired when parsing of
 * the page's document has finished.
 */
document.addEventListener('DOMContentLoaded', function() {
  updateStatus('Loading...');
  if (!browserSupportsPNaCl()) {
    updateStatus('Browser does not support PNaCl or PNaCl is disabled');
  } else if (naclModule == null) {
    createNaClModule('cube', '100%', '100%');
    attachDefaultListeners();
  } else {
    // It's possible that the Native Client module onload event fired
    // before the page's onload event.  In this case, the status message
    // will reflect 'SUCCESS', but won't be displayed.  This call will
    // display the current message.
    updateStatus('Waiting.');
  }
});
