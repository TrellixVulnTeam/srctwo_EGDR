'use strict';
promise_test(() => {
  let expected = new DOMException('Origin is not allowed to access the ' +
                                  'service. Tip: Add the service UUID to ' +
                                  '\'optionalServices\' in requestDevice() ' +
                                  'options. https://goo.gl/HxfxSQ',
                                  'SecurityError');
  return getHealthThermometerDevice({
      filters: [{services: ['health_thermometer']}]})
    .then(({device}) => Promise.all([
      assert_promise_rejects_with_message(
        device.gatt.CALLS([
          getPrimaryService(generic_access.alias)|
          getPrimaryServices(generic_access.alias)[UUID]
        ]), expected),
      assert_promise_rejects_with_message(
        device.gatt.FUNCTION_NAME(generic_access.name), expected),
      assert_promise_rejects_with_message(
        device.gatt.FUNCTION_NAME(generic_access.uuid), expected)]));
}, 'Request for present service without permission. Reject with SecurityError.');
