// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Custom binding for the contextMenus API.

var binding = apiBridge || require('binding').Binding.create('contextMenus');
var contextMenusHandlers = require('contextMenusHandlers');

binding.registerCustomHook(function(bindingsAPI) {
  var apiFunctions = bindingsAPI.apiFunctions;

  var handlers = contextMenusHandlers.create(false /* isWebview */);

  apiFunctions.setHandleRequest('create', handlers.requestHandlers.create);

  apiFunctions.setHandleRequest('remove', handlers.requestHandlers.remove);

  apiFunctions.setHandleRequest('update', handlers.requestHandlers.update);

  apiFunctions.setHandleRequest('removeAll',
                                handlers.requestHandlers.removeAll);
});

if (!apiBridge)
  exports.$set('binding', binding.generate());
