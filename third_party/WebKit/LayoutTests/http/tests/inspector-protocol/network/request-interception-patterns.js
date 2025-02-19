(async function(testRunner) {
  var {page, session, dp} = await testRunner.startBlank(
      `Tests interception pattern only matches requests based on pattern.`);

  var dataForNames = {
    'small-test-1.txt': 'I was intercepted.',
    'small-test-2.txt': 'I was also intercepted.',
    'test-page.html': 'I own test page!',
  };

  /** @type {!Map<number, string>} */
  var inflightRequests = new Map();
  /** @type {!Map<string, !Promise>} */
  var requestInterceptionWaitingMap = new Map();
  session.protocol.Network.onRequestWillBeSent(event => {
    var requestId = event.params.requestId;
    var fileName = nameForUrl(event.params.request.url);
    testRunner.log('Request Will Be Sent: ' + fileName);
    inflightRequests.set(requestId, fileName);
    var interceptionWaitingResolver = requestInterceptionWaitingMap.get(fileName);
    if (interceptionWaitingResolver)
      interceptionWaitingResolver();
  });

  session.protocol.Network.onRequestIntercepted(async event => {
    var fileName = nameForUrl(event.params.request.url);
    // Because requestWillBeSent and interception requests come from different processes, they may come in random order,
    // This will syncronize them to ensure requestWillBeSent is processed first and will stall here till it does.
    if (!inflightRequests.get(fileName)) {
      await new Promise(resolve => requestInterceptionWaitingMap.set(fileName, resolve));
      requestInterceptionWaitingMap.delete(fileName);
    }
    testRunner.log('Request Intercepted: ' + fileName);

    var rawContent = dataForNames[fileName];
    var headers = [
      'HTTP/1.1 200 OK',
      'Date: ' + (new Date()).toUTCString(),
      'Server: Test Interception',
      'Connection: closed',
      'Content-Length: ' + rawContent.length,
      'Content-Type: text/html',
    ];
    var encodedResponse = btoa(headers.join('\r\n') + '\r\n\r\n' + rawContent);
    session.protocol.Network.continueInterceptedRequest({interceptionId: event.params.interceptionId, rawResponse: encodedResponse});
  });

  session.protocol.Network.onLoadingFailed(event => { throw 'This test should never fail to load a resource.' });

  var responseWasReceivedCallback = () => { throw 'Must be overriden first.' };
  session.protocol.Network.onResponseReceived(async event => {
    var url = inflightRequests.get(event.params.requestId);
    testRunner.log('Response Received for: ' + url);

    var message = await session.protocol.Network.getResponseBody({requestId: event.params.requestId});
    var body = message.result.base64Encoded ? atob(message.result.body) : message.result.body;
    testRunner.log('Response Content: ' + body.replace(/[\r\n]+/g, '\\n'));
    testRunner.log('');
    responseWasReceivedCallback();
  });

  /**
   * @param {string} url
   * @return {string}
   */
  function nameForUrl(url) {
    return url.substr(url.lastIndexOf('/') + 1);
  }

  /**
   * @return {!Promise}
   */
  async function testUrls() {
    session.evaluate(`fetch('../network/resources/small-test-1.txt')`);
    await new Promise(resolve => responseWasReceivedCallback = resolve);
    session.evaluate(`fetch('../network/resources/small-test-2.txt')`);
    await new Promise(resolve => responseWasReceivedCallback = resolve);
    session.evaluate(`fetch('../resources/test-page.html')`);
    await new Promise(resolve => responseWasReceivedCallback = resolve);
    testRunner.log('');
  }

  /**
   * @param {boolean} enabled
   * @param {!Array<string>} patterns
   * @return {!Promise}
   */
  function setPatterns(enabled, patterns) {
    var params = {enabled};
    if (patterns)
      params.patterns = patterns;
    testRunner.log('setRequestInterceptionEnabled(' + enabled + ', ' + JSON.stringify(patterns) + ')');
    return session.protocol.Network.setRequestInterceptionEnabled(params);
  }

  testRunner.log('');
  await session.protocol.Network.enable();
  await session.protocol.Network.setCacheDisabled({cacheDisabled: true});

  await setPatterns(true);
  await testUrls();
  await setPatterns(true, []);
  await testUrls();
  await setPatterns(false, []);
  await testUrls();
  await setPatterns(false, ['*']);
  await testUrls();
  await setPatterns(false);
  await testUrls();
  await setPatterns(true, ['*']);
  await testUrls();
  await setPatterns(true, ['*small-test-1.txt']);
  await testUrls();
  await setPatterns(true, ['*small-test-?.txt']);
  await testUrls();
  await setPatterns(true, ['*small-test-*.txt']);
  await testUrls();
  await setPatterns(true, ['*small-test-\\*.txt']);
  await testUrls();
  await setPatterns(true, ['*small-test*']);
  await testUrls();
  await setPatterns(true, ['*small-test-1.txt', '*small-test-2.txt']);
  await testUrls();
  await setPatterns(true, ['*small-test-1.txt', '*small-*']);
  await testUrls();
  await setPatterns(true, ['*small-test-1.txt', '*-*']);
  await testUrls();
  await setPatterns(true, ['*-*', '*small-test-1.txt']);
  await testUrls();

  testRunner.completeTest();
})
