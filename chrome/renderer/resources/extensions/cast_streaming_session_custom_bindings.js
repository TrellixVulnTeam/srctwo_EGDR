// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Custom binding for the Cast Streaming Session API.

var binding = apiBridge ||
              require('binding').Binding.create('cast.streaming.session');
var natives = requireNative('cast_streaming_natives');

binding.registerCustomHook(function(bindingsAPI, extensionId) {
  var apiFunctions = bindingsAPI.apiFunctions;
  apiFunctions.setHandleRequest('create',
      function(audioTrack, videoTrack, callback) {
        natives.CreateSession(audioTrack, videoTrack, callback);
  });
});

if (!apiBridge)
  exports.$set('binding', binding.generate());
