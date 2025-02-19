'use strict';
promise_test(() => {
  let val = new Uint8Array([1]);
  return setBluetoothFakeAdapter('DisconnectingHealthThermometerAdapter')
    .then(() => requestDeviceWithKeyDown({
      filters: [{services: ['health_thermometer']}]}))
    .then(device => device.gatt.connect())
    .then(gattServer => gattServer.getPrimaryService('health_thermometer'))
    .then(service => service.getCharacteristic('measurement_interval'))
    .then(characteristic => characteristic.getDescriptor(user_description.name))
    .then(descriptor => {
      return setBluetoothFakeAdapter('MissingServiceHeartRateAdapter')
        .then(() => assert_promise_rejects_with_message(
          descriptor.CALLS([readValue()|writeValue(val)]),
          new DOMException(
            'GATT Service no longer exists.',
            'InvalidStateError'),
          'Service got removed.'));
    });
}, 'Service gets removed. Reject with InvalidStateError.');
