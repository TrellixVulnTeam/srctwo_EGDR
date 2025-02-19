// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chrome.tabs.getCurrent(function(tab) {
  runTestsForTab([
    // onAuthRequired is not a blocking function.
    function webSocketAuthRequiredNonBlocking() {
      var url = getWSTestURL(testWebSocketPort);
      expect(
        [  //events
          { label: 'onBeforeRequest',
            event: 'onBeforeRequest',
            details: {
              url: url,
              type: 'websocket',
              frameUrl: 'unknown frame URL',
            },
          },
          { label: 'onBeforeSendHeaders',
            event: 'onBeforeSendHeaders',
            details: {
              url: url,
              type: 'websocket',
            },
          },
          { label: 'onSendHeaders',
            event: 'onSendHeaders',
            details: {
              url: url,
              type: 'websocket',
            },
          },
          { label: 'onHeadersReceived',
            event: 'onHeadersReceived',
            details: {
              url: url,
              type: 'websocket',
              statusCode: 401,
              statusLine: 'HTTP/1.0 401 Unauthorized',
              responseHeadersExist: true,
            }
          },
          { label: 'onAuthRequired',
            event: 'onAuthRequired',
            details: {
              url: url,
              type: 'websocket',
              isProxy: false,
              scheme: 'basic',
              realm: 'Pywebsocket',
              challenger: {host: 'localhost', port: testWebSocketPort},
              statusCode: 401,
              statusLine: 'HTTP/1.0 401 Unauthorized',
              responseHeadersExist: true,
            }
          },
          { label: 'onResponseStarted',
            event: 'onResponseStarted',
            details: {
              url: url,
              type: 'websocket',
              ip: '127.0.0.1',
              fromCache: false,
              statusCode: 401,
              statusLine: 'HTTP/1.0 401 Unauthorized',
              responseHeadersExist: true,
            }
          },
          { label: 'onErrorOccurred',
            event: 'onErrorOccurred',
            details: {
              url: url,
              type: 'websocket',
              fromCache: false,
              error: 'net::ERR_ABORTED',
            }
          },
        ],
        [  // event order
          ['onBeforeRequest', 'onBeforeSendHeaders', 'onSendHeaders',
          'onHeadersReceived', 'onAuthRequired', 'onResponseStarted',
          'onErrorOccurred']
        ],
        {urls: ['<all_urls>']},  // filter
        ['responseHeaders']  // extraInfoSpec
      );
      testWebSocketConnection(url, false /* expectedToConnect*/);
    },

    // onAuthRequired is a blocking function but takes no action.
    function webSocketAuthRequiredSyncNoAction() {
      var url = getWSTestURL(testWebSocketPort);
      expect(
        [  // events
          { label: 'onBeforeRequest',
            event: 'onBeforeRequest',
            details: {
              url: url,
              type: 'websocket',
              frameUrl: 'unknown frame URL',
            }
          },
          { label: 'onBeforeSendHeaders',
            event: 'onBeforeSendHeaders',
            details: {
              url: url,
              type: 'websocket',
            },
          },
          { label: 'onSendHeaders',
            event: 'onSendHeaders',
            details: {
              url: url,
              type: 'websocket',
            }
          },
          { label: 'onHeadersReceived',
            event: 'onHeadersReceived',
            details: {
              url: url,
              type: 'websocket',
              statusCode: 401,
              statusLine: 'HTTP/1.0 401 Unauthorized',
            }
          },
          { label: 'onAuthRequired',
            event: 'onAuthRequired',
            details: {
              url: url,
              type: 'websocket',
              isProxy: false,
              scheme: 'basic',
              realm: 'Pywebsocket',
              challenger: {host: 'localhost', port: testWebSocketPort},
              statusCode: 401,
              statusLine: 'HTTP/1.0 401 Unauthorized',
            }
          },
          { label: 'onResponseStarted',
            event: 'onResponseStarted',
            details: {
              url: url,
              type: 'websocket',
              fromCache: false,
              ip: '127.0.0.1',
              statusCode: 401,
              statusLine: 'HTTP/1.0 401 Unauthorized',
            }
          },
          { label: 'onErrorOccurred',
            event: 'onErrorOccurred',
            details: {
              url: url,
              type: 'websocket',
              fromCache: false,
              error: 'net::ERR_ABORTED',
            }
          },
        ],
        [  // event order
          ['onBeforeRequest', 'onBeforeSendHeaders', 'onSendHeaders',
           'onHeadersReceived', 'onAuthRequired', 'onResponseStarted',
           'onErrorOccurred']
        ],
        {urls: ['<all_urls>']}, ['blocking']);
      testWebSocketConnection(url, false /* expectedToConnect*/);
    },

    // onAuthRequired is a blocking function that cancels the auth attempt.
    function webSocketAuthRequiredSyncCancelAuth() {
      var url = getWSTestURL(testWebSocketPort);
      expect(
        [  // events
          { label: 'onBeforeRequest',
            event: 'onBeforeRequest',
            details: {
              url: url,
              type: 'websocket',
              frameUrl: 'unknown frame URL',
            }
          },
          { label: 'onBeforeSendHeaders',
            event: 'onBeforeSendHeaders',
            details: {
              url: url,
              type: 'websocket',
            },
          },
          { label: 'onSendHeaders',
            event: 'onSendHeaders',
            details: {
              url: url,
              type: 'websocket',
            }
          },
          { label: 'onHeadersReceived',
            event: 'onHeadersReceived',
            details: {
              url: url,
              type: 'websocket',
              statusCode: 401,
              statusLine: 'HTTP/1.0 401 Unauthorized',
            }
          },
          { label: 'onAuthRequired',
            event: 'onAuthRequired',
            details: {
              url: url,
              type: 'websocket',
              isProxy: false,
              scheme: 'basic',
              realm: 'Pywebsocket',
              challenger: {host: 'localhost', port: testWebSocketPort},
              statusCode: 401,
              statusLine: 'HTTP/1.0 401 Unauthorized',
            },
            retval: {cancel: true}
          },
          { label: 'onResponseStarted',
            event: 'onResponseStarted',
            details: {
              url: url,
              type: 'websocket',
              fromCache: false,
              ip: '127.0.0.1',
              statusCode: 401,
              statusLine: 'HTTP/1.0 401 Unauthorized',
            }
          },
          { label: 'onErrorOccurred',
            event: 'onErrorOccurred',
            details: {
              url: url,
              type: 'websocket',
              fromCache: false,
              error: 'net::ERR_ABORTED',
            }
          },
        ],
        [  // event order
          ['onBeforeRequest', 'onBeforeSendHeaders', 'onSendHeaders',
           'onHeadersReceived', 'onAuthRequired', 'onResponseStarted',
           'onErrorOccurred']
        ],
        {urls: ['<all_urls>']}, ['blocking']);
      testWebSocketConnection(url, false /* expectedToConnect*/);
    },

    // onAuthRequired is a blocking function setting authentication credentials.
    function webSocketAuthRequiredSyncSetAuth() {
      var url = getWSTestURL(testWebSocketPort);
      expect(
        [  // events
          { label: 'onBeforeRequest',
            event: 'onBeforeRequest',
            details: {
              url: url,
              type: 'websocket',
              frameUrl: 'unknown frame URL',
            }
          },
          { label: 'onBeforeSendHeaders',
            event: 'onBeforeSendHeaders',
            details: {
              url: url,
              type: 'websocket',
            },
          },
          { label: 'onSendHeaders',
            event: 'onSendHeaders',
            details: {
              url: url,
              type: 'websocket',
            }
          },
          { label: 'onHeadersReceived',
            event: 'onHeadersReceived',
            details: {
              url: url,
              type: 'websocket',
              statusCode: 401,
              statusLine: 'HTTP/1.0 401 Unauthorized',
            }
          },
          { label: 'onAuthRequired',
            event: 'onAuthRequired',
            details: {
              url: url,
              type: 'websocket',
              isProxy: false,
              scheme: 'basic',
              realm: 'Pywebsocket',
              challenger: {host: 'localhost', port: testWebSocketPort},
              statusCode: 401,
              statusLine: 'HTTP/1.0 401 Unauthorized',
            },
            // Note: The test WebSocket server accepts only these credentials.
            retval: {authCredentials: {username: 'test', password: 'test'}}
          },
          { label: 'onResponseStarted',
            event: 'onResponseStarted',
            details: {
              url: url,
              type: 'websocket',
              ip: '127.0.0.1',
              fromCache: false,
              statusCode: 101,
              statusLine: 'HTTP/1.1 101 Switching Protocols',
            },
          },
          { label: 'onCompleted',
            event: 'onCompleted',
            details: {
              url: url,
              type: 'websocket',
              fromCache: false,
              statusCode: 101,
              statusLine: 'HTTP/1.1 101 Switching Protocols',
            }
          },
        ],
        [  // event order
          ['onBeforeRequest', 'onBeforeSendHeaders', 'onSendHeaders',
           'onHeadersReceived', 'onAuthRequired', 'onResponseStarted',
           'onCompleted']
        ],
        {urls: ['<all_urls>']}, ['blocking']);
      testWebSocketConnection(url, true /* expectedToConnect*/);
    },
  ], tab);
});
