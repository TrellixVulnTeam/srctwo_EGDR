'use strict';

importScripts('interfaces-idls.js');
importScripts('worker-testharness.js');
importScripts('/resources/WebIDLParser.js');
importScripts('/resources/idlharness.js');

var idlArray = new IdlArray();
idlArray.add_untested_idls(idls.untested);
idlArray.add_idls(idls.tested);
idlArray.add_objects({
    ServiceWorkerGlobalScope: ['self'],
    Clients: ['self.clients'],
    ServiceWorkerRegistration: ['self.registration'],
    CacheStorage: ['self.caches']
    // TODO: Test instances of Client and WindowClient, e.g.
    // Client: ['self.clientInstance'],
    // WindowClient: ['self.windowClientInstance']
  });

promise_test(function(t) {
    return create_temporary_cache(t)
      .then(function(cache) {
          self.cacheInstance = cache;

          idlArray.add_objects({ Cache: ['self.cacheInstance'] });
          idlArray.test();
        });
  }, 'test setup (cache creation)');

test(function() {
    var req = new Request('http://{{host}}/',
                          {method: 'POST',
                           headers: [['Content-Type', 'Text/Html']]});
    assert_equals(
      new ExtendableEvent('ExtendableEvent').type,
      'ExtendableEvent', 'Type of ExtendableEvent should be ExtendableEvent');
    assert_throws(new TypeError, function() {
        new FetchEvent('FetchEvent');
    }, 'FetchEvent constructor with one argument throws');
    assert_throws(new TypeError, function() {
        new FetchEvent('FetchEvent', {});
    }, 'FetchEvent constructor with empty init dict throws');
    assert_throws(new TypeError, function() {
        new FetchEvent('FetchEvent', {request: null});
    }, 'FetchEvent constructor with null request member throws');
    assert_equals(
      new FetchEvent('FetchEvent', {request: req}).type,
      'FetchEvent', 'Type of FetchEvent should be FetchEvent');
    assert_equals(
      new FetchEvent('FetchEvent', {request: req}).cancelable,
      false, 'Default FetchEvent.cancelable should be false');
    assert_equals(
      new FetchEvent('FetchEvent', {request: req}).bubbles,
      false, 'Default FetchEvent.bubbles should be false');
    assert_equals(
      new FetchEvent('FetchEvent', {request: req}).clientId,
      null, 'Default FetchEvent.clientId should be null');
    assert_equals(
      new FetchEvent('FetchEvent', {request: req}).isReload,
      false, 'Default FetchEvent.isReload should be false');
    assert_equals(
      new FetchEvent('FetchEvent', {request: req, cancelable: false}).cancelable,
      false, 'FetchEvent.cancelable should be false');
    assert_equals(
      new FetchEvent('FetchEvent', {request: req, clientId : 'test-client-id'}).clientId, 'test-client-id',
      'FetchEvent.clientId with option {clientId : "test-client-id"} should be "test-client-id"');
    assert_equals(
      new FetchEvent('FetchEvent', {request: req, isReload : true}).isReload, true,
      'FetchEvent.isReload with option {isReload : true} should be true');
    assert_equals(
      new FetchEvent('FetchEvent', {request : req, isReload : true}).request.url,
      'http://{{host}}/',
      'FetchEvent.request.url should return the value it was initialized to');
  }, 'Event constructors');
