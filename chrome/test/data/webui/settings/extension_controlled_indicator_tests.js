// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

suite('extension controlled indicator', function() {
  /** @type {TestExtensionControlBrowserProxy} */
  var browserProxy;

  /** @type {ExtensionControlledIndicatorElement} */
  var indicator;

  setup(function() {
    PolymerTest.clearBody();
    browserProxy = new TestExtensionControlBrowserProxy();
    settings.ExtensionControlBrowserProxyImpl.instance_ = browserProxy;
    indicator = document.createElement('extension-controlled-indicator');
    indicator.extensionId = 'peiafolljookckjknpgofpbjobgbmpge';
    indicator.extensionCanBeDisabled = true;
    indicator.extensionName = 'The Bestest Name Ever';
    document.body.appendChild(indicator);
    Polymer.dom.flush();
  });

  test('disable button tracks extensionCanBeDisabled', function() {
    assertTrue(indicator.extensionCanBeDisabled);
    assertTrue(!!indicator.$$('paper-button'));

    indicator.extensionCanBeDisabled = false;
    Polymer.dom.flush();
    assertFalse(!!indicator.$$('paper-button'));
  });

  test('label text and href', function() {
    var imgSrc = indicator.$$('img').src;
    assertTrue(imgSrc.includes(indicator.extensionId));

    var label = indicator.$$('span');
    assertTrue(!!label);
    var labelLink = label.querySelector('a');
    assertTrue(!!labelLink);
    assertEquals(labelLink.textContent, indicator.extensionName);

    assertEquals('chrome://extensions', new URL(labelLink.href).origin);
    assertTrue(labelLink.href.includes(indicator.extensionId));

    indicator.extensionId = 'dpjamkmjmigaoobjbekmfgabipmfilij';
    indicator.extensionName = "A Slightly Less Good Name (Can't Beat That ^)";
    Polymer.dom.flush();

    imgSrc = indicator.$$('img').src;
    assertTrue(imgSrc.includes(indicator.extensionId));

    label = indicator.$$('span');
    assertTrue(!!label);
    labelLink = label.querySelector('a');
    assertTrue(!!labelLink);
    assertEquals(labelLink.textContent, indicator.extensionName);
  });

  test('tapping disable button invokes browser proxy', function() {
    var disableButton = indicator.$$('paper-button');
    assertTrue(!!disableButton);
    MockInteractions.tap(disableButton);
    return browserProxy.whenCalled('disableExtension').then(
        function (extensionId) {
          assertEquals(extensionId, indicator.extensionId);
        });
  });
});
