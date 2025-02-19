// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function getStyleURL() {
  // This is not a style sheet, but it must still be classified as "stylesheet"
  // because it is loaded as a style sheet.
  return getServerURL('empty.html?as-style');
}

function getScriptURL() {
  // The file is empty, so JS errors will not be generated upon execution.
  //
  // We load from '127.0.0.1', as that is a whitelistable source of script
  // from outside the extension's package.
  return getServerURL('empty.html?as-script', '127.0.0.1');
}

function getFontURL() {
  // Not a font, but will be loaded as a font.
  return getServerURL('empty.html?as-font');
}

function getWorkerURL() {
  // This file is empty, so it does not generate JavaScript errors when loaded
  // as a worker script.
  return getServerURL('empty.html?as-worker', '127.0.0.1');
}

function getPingURL() {
  return getServerURL('empty.html?as-ping');
}

function getBeaconURL() {
  return getServerURL('empty.html?as-beacon');
}

function getCSPReportURL() {
  // dont-ignore-me is included so that framework.js does not filter out the
  // request of type "other".
  return getServerURL('csp-violation-dont-ignore-me');
}

// A slow URL used for the beacon test, to make sure that the test fails
// deterministically if there is a bug that causes the frameId/tabId to not be
// set if the frame is removed during the request.
function getSlowURL() {
  return getServerURL('slow?2.718');
}

function getScriptFilter() {
  // Scripts and worker scripts are internally represented by a different
  // ResourceType, but they still map to the same public "script" type.
  // We have plenty of tests that confirm that requests are visible when no
  // filter is applied. We also need to check whether the requests are still
  // visible even after applying a "script" filter.
  // This is part of the regression test for crbug.com/591988.
  return {urls: ['<all_urls>'], types: ['script']};
}

runTests([
  function typeStylesheet() {
    expect([
      { label: 'onBeforeRequest',
        event: 'onBeforeRequest',
        details: {
          type: 'stylesheet',
          url: getStyleURL(),
          frameUrl: 'unknown frame URL',
          // tabId 0 = tab opened by test runner;
          // tabId 1 = this tab.
          tabId: 1,
        }
      },
      { label: 'onBeforeSendHeaders',
        event: 'onBeforeSendHeaders',
        details: {
          type: 'stylesheet',
          url: getStyleURL(),
          tabId: 1,
        },
      },
      { label: 'onSendHeaders',
        event: 'onSendHeaders',
        details: {
          type: 'stylesheet',
          url: getStyleURL(),
          tabId: 1,
        },
      },
      { label: 'onHeadersReceived',
        event: 'onHeadersReceived',
        details: {
          type: 'stylesheet',
          url: getStyleURL(),
          tabId: 1,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onResponseStarted',
        event: 'onResponseStarted',
        details: {
          type: 'stylesheet',
          url: getStyleURL(),
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onCompleted',
        event: 'onCompleted',
        details: {
          type: 'stylesheet',
          url: getStyleURL(),
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      }],
      [['onBeforeRequest', 'onBeforeSendHeaders', 'onSendHeaders',
        'onHeadersReceived', 'onResponseStarted', 'onCompleted']]);
    var style = document.createElement('link');
    style.rel = 'stylesheet';
    style.type = 'text/css';
    style.href = getStyleURL();
    document.body.appendChild(style);
  },

  function typeScript() {
    expect([
      { label: 'onBeforeRequest',
        event: 'onBeforeRequest',
        details: {
          type: 'script',
          url: getScriptURL(),
          // Unknown because data:-URLs are invisible to the webRequest API,
          // and the script is loaded via a data:-URL document in a frame.
          frameUrl: 'unknown frame URL',
          frameId: 1,
          parentFrameId: 0,
          // tabId 0 = tab opened by test runner;
          // tabId 1 = this tab.
          tabId: 1,
        }
      },
      { label: 'onBeforeSendHeaders',
        event: 'onBeforeSendHeaders',
        details: {
          type: 'script',
          url: getScriptURL(),
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
        },
      },
      { label: 'onSendHeaders',
        event: 'onSendHeaders',
        details: {
          type: 'script',
          url: getScriptURL(),
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
        },
      },
      { label: 'onHeadersReceived',
        event: 'onHeadersReceived',
        details: {
          type: 'script',
          url: getScriptURL(),
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onResponseStarted',
        event: 'onResponseStarted',
        details: {
          type: 'script',
          url: getScriptURL(),
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onCompleted',
        event: 'onCompleted',
        details: {
          type: 'script',
          url: getScriptURL(),
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      }],
      [['onBeforeRequest', 'onBeforeSendHeaders', 'onSendHeaders',
        'onHeadersReceived', 'onResponseStarted', 'onCompleted']],
      getScriptFilter());

    var frame = document.createElement('iframe');
    frame.src = 'data:text/html,<script src="' + getScriptURL() + '"></script>';
    document.body.appendChild(frame);
  },

  function typeFont() {
    expect([
      { label: 'onBeforeRequest',
        event: 'onBeforeRequest',
        details: {
          type: 'font',
          url: getFontURL(),
          frameUrl: 'unknown frame URL',
          // tabId 0 = tab opened by test runner;
          // tabId 1 = this tab.
          tabId: 1,
        }
      },
      { label: 'onBeforeSendHeaders',
        event: 'onBeforeSendHeaders',
        details: {
          type: 'font',
          url: getFontURL(),
          tabId: 1,
        },
      },
      { label: 'onSendHeaders',
        event: 'onSendHeaders',
        details: {
          type: 'font',
          url: getFontURL(),
          tabId: 1,
        },
      },
      { label: 'onHeadersReceived',
        event: 'onHeadersReceived',
        details: {
          type: 'font',
          url: getFontURL(),
          tabId: 1,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onResponseStarted',
        event: 'onResponseStarted',
        details: {
          type: 'font',
          url: getFontURL(),
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onCompleted',
        event: 'onCompleted',
        details: {
          type: 'font',
          url: getFontURL(),
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      }],
      [['onBeforeRequest', 'onBeforeSendHeaders', 'onSendHeaders',
        'onHeadersReceived', 'onResponseStarted', 'onCompleted']]);

    new FontFace('allegedly-a-font-family', 'url(' + getFontURL() + ')').load();
  },

  function typeWorker() {
    expect([
      { label: 'onBeforeRequest',
        event: 'onBeforeRequest',
        details: {
          type: 'script',
          url: getWorkerURL(),
          frameUrl: 'unknown frame URL',
          // tabId 0 = tab opened by test runner;
          // tabId 1 = this tab.
          tabId: 1,
        }
      },
      { label: 'onBeforeSendHeaders',
        event: 'onBeforeSendHeaders',
        details: {
          type: 'script',
          url: getWorkerURL(),
          tabId: 1,
        },
      },
      { label: 'onSendHeaders',
        event: 'onSendHeaders',
        details: {
          type: 'script',
          url: getWorkerURL(),
          tabId: 1,
        },
      },
      { label: 'onHeadersReceived',
        event: 'onHeadersReceived',
        details: {
          type: 'script',
          url: getWorkerURL(),
          tabId: 1,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onResponseStarted',
        event: 'onResponseStarted',
        details: {
          type: 'script',
          url: getWorkerURL(),
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onCompleted',
        event: 'onCompleted',
        details: {
          type: 'script',
          url: getWorkerURL(),
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      }],
      [['onBeforeRequest', 'onBeforeSendHeaders', 'onSendHeaders',
        'onHeadersReceived', 'onResponseStarted', 'onCompleted']],
      getScriptFilter());

    new Worker(getWorkerURL());

    // TODO(robwu): add tests for SharedWorker and ServiceWorker.
    // (probably same as above, but using -1 because they are not specific to
    //  any tab. In general we really need a lot more test coverage for the
    //  interaction between Service workers and extensions.)
  },

  function typePing() {
    expect([
      { label: 'onBeforeRequest',
        event: 'onBeforeRequest',
        details: {
          type: 'ping',
          method: 'POST',
          url: getPingURL(),
          frameUrl: 'unknown frame URL',
          frameId: 0,
          tabId: 1,
        }
      },
      { label: 'onBeforeSendHeaders',
        event: 'onBeforeSendHeaders',
        details: {
          type: 'ping',
          method: 'POST',
          url: getPingURL(),
          frameId: 0,
          tabId: 1,
        },
      },
      { label: 'onSendHeaders',
        event: 'onSendHeaders',
        details: {
          type: 'ping',
          method: 'POST',
          url: getPingURL(),
          frameId: 0,
          tabId: 1,
        },
      },
      { label: 'onHeadersReceived',
        event: 'onHeadersReceived',
        details: {
          type: 'ping',
          method: 'POST',
          url: getPingURL(),
          frameId: 0,
          tabId: 1,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onResponseStarted',
        event: 'onResponseStarted',
        details: {
          type: 'ping',
          method: 'POST',
          url: getPingURL(),
          frameId: 0,
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onCompleted',
        event: 'onCompleted',
        details: {
          type: 'ping',
          method: 'POST',
          url: getPingURL(),
          frameId: 0,
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      }],
      [['onBeforeRequest', 'onBeforeSendHeaders', 'onSendHeaders',
        'onHeadersReceived', 'onResponseStarted', 'onCompleted']]);

    var a = document.createElement('a');
    a.ping = getPingURL();
    a.href = 'javascript:';
    a.click();
  },

  function typeBeacon() {
    expect([
      { label: 'onBeforeRequest',
        event: 'onBeforeRequest',
        details: {
          type: 'ping',
          method: 'POST',
          url: getBeaconURL(),
          frameUrl: 'unknown frame URL',
          frameId: 0,
          tabId: 1,
        }
      },
      { label: 'onBeforeSendHeaders',
        event: 'onBeforeSendHeaders',
        details: {
          type: 'ping',
          method: 'POST',
          url: getBeaconURL(),
          frameId: 0,
          tabId: 1,
        },
      },
      { label: 'onSendHeaders',
        event: 'onSendHeaders',
        details: {
          type: 'ping',
          method: 'POST',
          url: getBeaconURL(),
          frameId: 0,
          tabId: 1,
        },
      },
      { label: 'onHeadersReceived',
        event: 'onHeadersReceived',
        details: {
          type: 'ping',
          method: 'POST',
          url: getBeaconURL(),
          frameId: 0,
          tabId: 1,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onResponseStarted',
        event: 'onResponseStarted',
        details: {
          type: 'ping',
          method: 'POST',
          url: getBeaconURL(),
          frameId: 0,
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onCompleted',
        event: 'onCompleted',
        details: {
          type: 'ping',
          method: 'POST',
          url: getBeaconURL(),
          frameId: 0,
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      }],
      [['onBeforeRequest', 'onBeforeSendHeaders', 'onSendHeaders',
        'onHeadersReceived', 'onResponseStarted', 'onCompleted']]);

    navigator.sendBeacon(getBeaconURL(), 'beacon data');
  },

  function sendBeaconInFrameOnUnload() {
    expect([
      { label: 'onBeforeRequest',
        event: 'onBeforeRequest',
        details: {
          type: 'ping',
          method: 'POST',
          url: getSlowURL(),
          frameUrl: 'unknown frame URL',
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
        }
      },
      { label: 'onBeforeSendHeaders',
        event: 'onBeforeSendHeaders',
        details: {
          type: 'ping',
          method: 'POST',
          url: getSlowURL(),
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
        },
      },
      { label: 'onSendHeaders',
        event: 'onSendHeaders',
        details: {
          type: 'ping',
          method: 'POST',
          url: getSlowURL(),
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
        },
      },
      { label: 'onHeadersReceived',
        event: 'onHeadersReceived',
        details: {
          type: 'ping',
          method: 'POST',
          url: getSlowURL(),
          // TODO(robwu): These IDs should be identical to the previous IDs, but
          // unfortunately the context is lost when the frames are destroyed.
          // This should be fixed - https://crbug.com/522129
          frameId: -1,
          parentFrameId: -1,
          tabId: -1,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onResponseStarted',
        event: 'onResponseStarted',
        details: {
          type: 'ping',
          method: 'POST',
          url: getSlowURL(),
          frameId: -1,
          parentFrameId: -1,
          tabId: -1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      },
      { label: 'onCompleted',
        event: 'onCompleted',
        details: {
          type: 'ping',
          method: 'POST',
          url: getSlowURL(),
          frameId: -1,
          parentFrameId: -1,
          tabId: -1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 200 OK',
          statusCode: 200,
        },
      }],
      [['onBeforeRequest', 'onBeforeSendHeaders', 'onSendHeaders',
        'onHeadersReceived', 'onResponseStarted', 'onCompleted']]);

    var frame = document.createElement('iframe');
    document.body.appendChild(frame);
    frame.contentWindow.onunload = function() {
      console.log('Going to send beacon...');
      var sentBeacon = frame.contentWindow.navigator.sendBeacon(getSlowURL());
      chrome.test.assertTrue(sentBeacon);
    };
    frame.remove();
  },

  function typeOther_cspreport() {
    expect([
      { label: 'onBeforeRequest',
        event: 'onBeforeRequest',
        details: {
          type: 'csp_report',
          method: 'POST',
          url: getCSPReportURL(),
          frameUrl: 'unknown frame URL',
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
        }
      },
      { label: 'onBeforeSendHeaders',
        event: 'onBeforeSendHeaders',
        details: {
          type: 'csp_report',
          method: 'POST',
          url: getCSPReportURL(),
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
        },
      },
      { label: 'onSendHeaders',
        event: 'onSendHeaders',
        details: {
          type: 'csp_report',
          method: 'POST',
          url: getCSPReportURL(),
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
        },
      },
      { label: 'onHeadersReceived',
        event: 'onHeadersReceived',
        details: {
          type: 'csp_report',
          method: 'POST',
          url: getCSPReportURL(),
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
          statusLine: 'HTTP/1.1 404 Not Found',
          statusCode: 404,
        },
      },
      { label: 'onResponseStarted',
        event: 'onResponseStarted',
        details: {
          type: 'csp_report',
          method: 'POST',
          url: getCSPReportURL(),
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 404 Not Found',
          statusCode: 404,
        },
      },
      { label: 'onCompleted',
        event: 'onCompleted',
        details: {
          type: 'csp_report',
          method: 'POST',
          url: getCSPReportURL(),
          frameId: 1,
          parentFrameId: 0,
          tabId: 1,
          ip: '127.0.0.1',
          fromCache: false,
          statusLine: 'HTTP/1.1 404 Not Found',
          statusCode: 404,
        },
      }],
      [['onBeforeRequest', 'onBeforeSendHeaders', 'onSendHeaders',
        'onHeadersReceived', 'onResponseStarted', 'onCompleted']], {
        urls: ['<all_urls>'], types: ['csp_report']
      });

    var frame = document.createElement('iframe');
    frame.src =
      getServerURL('extensions/api_test/webrequest/csp/violation.html');
    document.body.appendChild(frame);
  },

  // Note: The 'websocket' type is tested separately in 'test_websocket.js' and
  // 'test_websocket_auth.js'.
]);
