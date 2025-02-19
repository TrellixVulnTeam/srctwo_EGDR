// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The PlayerUtils provides utility functions to binding common media events
// to specific player functions. It also provides functions to load media source
// base on test configurations.
var PlayerUtils = new function() {
}

// Prepares a video element for playback by setting default event handlers
// and source attribute.
PlayerUtils.registerDefaultEventListeners = function(player) {
  Utils.timeLog('Registering video event handlers.');
  // Map from event name to event listener function name.  It is common for
  // event listeners to be named onEventName.
  var eventListenerMap = {
    'encrypted': 'onEncrypted',
  };
  for (eventName in eventListenerMap) {
    var eventListenerFunction = player[eventListenerMap[eventName]];
    if (eventListenerFunction) {
      player.video.addEventListener(eventName, function(e) {
        player[eventListenerMap[e.type]](e);
      });
    }
  }

  player.video.addEventListener('error', function(error) {
    // This most likely happens on pipeline failures (e.g. when the CDM
    // crashes). Don't report a failure if the test is checking that sessions
    // are closed on a crash.
    Utils.timeLog('onHTMLElementError', error.target.error.message);
    if (player.testConfig.keySystem == CRASH_TEST_KEYSYSTEM) {
      // On failure the session should have been closed, so verify.
      player.session.closed.then(
          function(result) {
            Utils.setResultInTitle(EME_SESSION_CLOSED_AND_ERROR);
          },
          function(error) { Utils.failTest(error); });
    } else {
      Utils.failTest(error);
    }
  });
};

// Register the necessary event handlers needed when playing encrypted content.
// Returns a promise that resolves to the player.
PlayerUtils.registerEMEEventListeners = function(player) {
  player.video.addEventListener('encrypted', function(message) {

    function addMediaKeySessionListeners(mediaKeySession) {
      mediaKeySession.addEventListener('message', function(message) {
        player.video.receivedKeyMessage = true;
        if (Utils.isRenewalMessage(message)) {
          Utils.timeLog('MediaKeySession onMessage - renewal', message);
          player.video.receivedRenewalMessage = true;
        } else {
          if (message.messageType != 'license-request') {
            Utils.failTest('Unexpected message type "' + message.messageType +
                               '" received.',
                           EME_MESSAGE_UNEXPECTED_TYPE);
          }
        }
        player.onMessage(message);
      });
    }

    function getStatusForHdcpPolicy(mediaKeys, hdcpVersion, expectedResult) {
      var policy = new MediaKeysPolicy({minHdcpVersion: hdcpVersion});

      return mediaKeys.getStatusForPolicy(policy).then(function(keyStatus) {
        if (keyStatus == expectedResult) {
          return Promise.resolve();
        } else {
          return Promise.reject(
              "keyStatus " + keyStatus + " does not match " + expectedResult);
        }
      }, function(error) {
        if (expectedResult == "rejected") {
          return Promise.resolve();
        } else {
          return Promise.reject("Promise rejected unexpectedly.");
        }
      });
    }

    function testGetStatusForHdcpPolicy(mediaKeys) {
      const keySystem = this.testConfig.keySystem;
      Utils.timeLog('Key system: ' + keySystem);
      if (keySystem == EXTERNAL_CLEARKEY) {
        return Promise.resolve().then(function() {
          return getStatusForHdcpPolicy(mediaKeys, "", "usable");
        }).then(function() {
          return getStatusForHdcpPolicy(mediaKeys, "hdcp-1.0", "usable");
        }).then(function() {
          return getStatusForHdcpPolicy(
              mediaKeys, "hdcp-2.2", "output-restricted");
        });
      } else {
        return Promise.resolve().then(function() {
          return getStatusForHdcpPolicy(mediaKeys, "", "rejected");
        }).then(function() {
          return getStatusForHdcpPolicy(mediaKeys, "hdcp-1.0", "rejected");
        });
      }
    }

    try {
      if (player.testConfig.sessionToLoad) {
        // Create a session to load using a new MediaKeys.
        // TODO(jrummell): Add a test that covers remove().
        player.access.createMediaKeys()
            .then(function(mediaKeys) {
              // As the tests run with a different origin every time, there is
              // no way currently to create a session in one test and then load
              // it in a subsequent test (https://crbug.com/715349).
              // So if |sessionToLoad| is 'LoadableSession', create a session
              // that can be loaded and use that session to load. Otherwise
              // use the name provided (which allows for testing load() on a
              // session which doesn't exist).
              if (player.testConfig.sessionToLoad == 'LoadableSession') {
                return Utils.createSessionToLoad(
                    mediaKeys, player.testConfig.sessionToLoad);
              } else {
                return player.testConfig.sessionToLoad;
              }
            })
            .then(function(sessionId) {
              Utils.timeLog('Loading session: ' + sessionId);
              player.session =
                  message.target.mediaKeys.createSession('persistent-license');
              addMediaKeySessionListeners(player.session);
              return player.session.load(sessionId);
            })
            .then(
                function(result) {
                  if (!result)
                    Utils.failTest('Session not found.', EME_SESSION_NOT_FOUND);
                },
                function(error) {
                  Utils.failTest(error, EME_LOAD_FAILED);
                });
      } else if (player.testConfig.policyCheck) {
        // TODO(xhwang): We should be able to move all policy check code to a
        // new test js file once we figure out an easy way to separate the
        // requrestMediaKeySystemAccess() logic from the rest of this file.
        Utils.timeLog('Policy check test.');
        player.access.createMediaKeys().then(function(mediaKeys) {
          // Call getStatusForPolicy() before creating any MediaKeySessions.
          return testGetStatusForHdcpPolicy(mediaKeys);
        }).then(function(result) {
          Utils.timeLog('Policy check test passed.');
          Utils.setResultInTitle(UNIT_TEST_SUCCESS);
        }).catch(function(error) {
          Utils.timeLog('Policy check test failed.');
          Utils.failTest(error, UNIT_TEST_FAILURE);
        });
      } else {
        Utils.timeLog('Creating new media key session for initDataType: ' +
                      message.initDataType + ', initData: ' +
                      Utils.getHexString(new Uint8Array(message.initData)));
        player.session = message.target.mediaKeys.createSession();
        addMediaKeySessionListeners(player.session);
        player.session.generateRequest(message.initDataType, message.initData)
            .catch(function(error) {
              // Ignore the error if a crash is expected. This ensures that
              // the decoder actually detects and reports the error.
              if (this.testConfig.keySystem != CRASH_TEST_KEYSYSTEM) {
                Utils.failTest(error, EME_GENERATEREQUEST_FAILED);
              }
            });
      }
    } catch (e) {
      Utils.failTest(e);
    }
  });

  this.registerDefaultEventListeners(player);
  player.video.receivedKeyMessage = false;
  player.video.receivedRenewalMessage = false;
  Utils.timeLog('Setting video media keys: ' + player.testConfig.keySystem);

  var config = {
    audioCapabilities: [],
    videoCapabilities: [],
    persistentState: 'optional',
    sessionTypes: ['temporary'],
  };

  // requestMediaKeySystemAccess() requires at least one of 'audioCapabilities'
  // or 'videoCapabilities' to be specified. It also requires only codecs
  // specific to the capability, so unlike MSE cannot have both audio and
  // video codecs in the contentType.
  if (player.testConfig.mediaType) {
    if (player.testConfig.mediaType.substring(0, 5) == 'video') {
      config.videoCapabilities = [{contentType: player.testConfig.mediaType}];
    } else if (player.testConfig.mediaType.substring(0, 5) == 'audio') {
      config.audioCapabilities = [{contentType: player.testConfig.mediaType}];
    }
    // Handle special cases where both audio and video are needed.
    if (player.testConfig.mediaType == 'video/webm; codecs="vorbis, vp8"') {
      config.audioCapabilities = [{contentType: 'audio/webm; codecs="vorbis"'}];
      config.videoCapabilities = [{contentType: 'video/webm; codecs="vp8"'}];
    } else if (
        player.testConfig.mediaType == 'video/webm; codecs="opus, vp9"') {
      config.audioCapabilities = [{contentType: 'audio/webm; codecs="opus"'}];
      config.videoCapabilities = [{contentType: 'video/webm; codecs="vp9"'}];
    }
  } else {
    // Some tests (e.g. mse_different_containers.html) specify audio and
    // video codecs seperately.
    if (player.testConfig.videoFormat == 'ENCRYPTED_MP4' ||
        player.testConfig.videoFormat == 'CLEAR_MP4') {
      config.videoCapabilities =
          [{contentType: 'video/mp4; codecs="avc1.4D000C"'}];
    } else if (
        player.testConfig.videoFormat == 'ENCRYPTED_WEBM' ||
        player.testConfig.videoFormat == 'CLEAR_WEBM') {
      config.videoCapabilities = [{contentType: 'video/webm; codecs="vp8"'}];
    }
    if (player.testConfig.audioFormat == 'ENCRYPTED_MP4' ||
        player.testConfig.audioFormat == 'CLEAR_MP4') {
      config.audioCapabilities =
          [{contentType: 'audio/mp4; codecs="mp4a.40.2"'}];
    } else if (
        player.testConfig.audioFormat == 'ENCRYPTED_WEBM' ||
        player.testConfig.audioFormat == 'CLEAR_WEBM') {
      config.audioCapabilities = [{contentType: 'audio/webm; codecs="vorbis"'}];
    }
  }

  // The File IO test requires persistent state support.
  if (player.testConfig.keySystem == FILE_IO_TEST_KEYSYSTEM) {
    config.persistentState = 'required';
  } else if (
      player.testConfig.sessionToLoad ||
      player.testConfig.keySystem == STORAGE_ID_TEST_KEYSYSTEM) {
    config.persistentState = 'required';
    config.sessionTypes = ['temporary', 'persistent-license'];
  }

  return navigator
      .requestMediaKeySystemAccess(player.testConfig.keySystem, [config])
      .then(function(access) {
        player.access = access;
        return access.createMediaKeys();
      })
      .then(function(mediaKeys) {
        return player.video.setMediaKeys(mediaKeys);
      })
      .then(function(result) {
        return player;
      })
      .catch(function(error) {
        Utils.failTest(error, NOTSUPPORTEDERROR);
      });
};

PlayerUtils.setVideoSource = function(player) {
  if (player.testConfig.useMSE) {
    Utils.timeLog('Loading media using MSE.');
    var mediaSource =
        MediaSourceUtils.loadMediaSourceFromTestConfig(player.testConfig);
    player.video.src = window.URL.createObjectURL(mediaSource);
  } else {
    Utils.timeLog('Loading media using src.');
    player.video.src = player.testConfig.mediaFile;
  }
  Utils.timeLog('video.src has been set to ' + player.video.src);
};

// Initialize the player to play encrypted content. Returns a promise that
// resolves to the player.
PlayerUtils.initEMEPlayer = function(player) {
  return player.registerEventListeners().then(function(result) {
    PlayerUtils.setVideoSource(player);
    Utils.timeLog('initEMEPlayer() done');
    return player;
  });
};

// Return the appropriate player based on test configuration.
PlayerUtils.createPlayer = function(video, testConfig) {
  function getPlayerType(keySystem) {
    switch (keySystem) {
      case WIDEVINE_KEYSYSTEM:
        return WidevinePlayer;
      case CLEARKEY:
      case EXTERNAL_CLEARKEY:
      case EXTERNAL_CLEARKEY_RENEWAL:
      case CRASH_TEST_KEYSYSTEM:
        return ClearKeyPlayer;
      case FILE_IO_TEST_KEYSYSTEM:
      case OUTPUT_PROTECTION_TEST_KEYSYSTEM:
      case PLATFORM_VERIFICATION_TEST_KEYSYSTEM:
      case VERIFY_HOST_FILES_TEST_KEYSYSTEM:
      case STORAGE_ID_TEST_KEYSYSTEM:
        return UnitTestPlayer;
      default:
        Utils.timeLog(keySystem + ' is not a known key system');
        return ClearKeyPlayer;
    }
  }
  var Player = getPlayerType(testConfig.keySystem);
  return new Player(video, testConfig);
};
