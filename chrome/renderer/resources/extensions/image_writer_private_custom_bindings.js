// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Custom binding for the image writer private API.

var binding =
    apiBridge || require('binding').Binding.create('imageWriterPrivate');

binding.registerCustomHook(function(bindingsAPI) {
  var apiFunctions = bindingsAPI.apiFunctions;

  apiFunctions.setUpdateArgumentsPostValidate(
      'writeFromFile', function(device, fileEntry, options, callback) {
    var fileSystemName = fileEntry.filesystem.name;
    var relativePath = $String.slice(fileEntry.fullPath, 1);
    return [device, fileSystemName, relativePath, callback];
  });
});

if (!apiBridge)
  exports.$set('binding', binding.generate());
