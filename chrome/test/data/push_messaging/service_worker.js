// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Use an absolute path since this could be loaded from a different scope,
// which would affect the scope of the importScripts call here.
self.importScripts('/push_messaging/push_constants.js');

// Don't wait for clients of old SW to close before activating.
self.addEventListener('install', () => skipWaiting());

// The "onpush" event currently understands two values as message payload
// data coming from the test. Any other input is passed through to the
// document unchanged.
//
// "shownotification" - Display a Web Notification with event.waitUntil().
// "shownotification-without-waituntil"
//     - Display a Web Notification without using event.waitUntil().
this.onpush = function(event) {
  if (event.data === null) {
    sendMessageToClients('push', '[NULL]');
    return;
  }

  var data = event.data.text();
  if (!data.startsWith('shownotification')) {
    sendMessageToClients('push', data);
    return;
  }

  var result = registration.showNotification('Push test title', {
    body: 'Push test body',
    tag: 'push_test_tag'
  });

  if (data == 'shownotification-without-waituntil') {
    sendMessageToClients('push', 'immediate:' + data);
    return;
  }

  event.waitUntil(result.then(function() {
    sendMessageToClients('push', data);
  }, function(ex) {
    sendMessageToClients('push', String(ex));
  }));
};

self.addEventListener('message', function handler (event) {
  let pushSubscriptionOptions = {
      userVisibleOnly: true
  };
  if (event.data.command == 'workerSubscribe') {
    pushSubscriptionOptions.applicationServerKey = kApplicationServerKey.buffer;
  } else if (event.data.command == 'workerSubscribeWithNumericKey') {
    pushSubscriptionOptions.applicationServerKey =
        new TextEncoder().encode(event.data.key);
  } else if (event.data.command == 'workerSubscribeNoKey') {
    // Nothing to set up
  } else {
    sendMessageToClients('message', 'error - unknown message request');
    return;
  }

  self.registration.pushManager.subscribe(pushSubscriptionOptions)
      .then(function(subscription) {
        sendMessageToClients('message', subscription.endpoint);
      }, function(error) {
        sendErrorToClients(error);
      });
});

function sendErrorToClients(error) {
  sendMessageToClients('error', error.name + ' - ' + error.message);
}

function sendMessageToClients(type, data) {
  var message = JSON.stringify({
    'type': type,
    'data': data
  });
  clients.matchAll().then(function(clients) {
    clients.forEach(function(client) {
      client.postMessage(message);
    });
  }, function(error) {
    console.log(error);
  });
}
