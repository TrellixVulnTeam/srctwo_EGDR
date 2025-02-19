'use strict';
promise_test(() => {
  let val = new Uint8Array([1]);
  return setBluetoothFakeAdapter('DisconnectingHealthThermometerAdapter')
      .then(
          () => requestDeviceWithKeyDown(
              {filters: [{services: ['health_thermometer']}]}))
      .then(device => device.gatt.connect())
      .then(gattServer => {
        return gattServer.getPrimaryService('health_thermometer')
            .then(service => service.getCharacteristic('measurement_interval'))
            .then(measurement_interval => {
              gattServer.disconnect();
              return assert_promise_rejects_with_message(
                  measurement_interval.CALLS(
                      [getDescriptor(user_description.name) |
                       getDescriptors(user_description.name)[UUID] |
                       getDescriptors()]),
                  new DOMException(
                      'GATT Server is disconnected. Cannot retrieve descriptors. ' +
                          '(Re)connect first with `device.gatt.connect`.',
                      'NetworkError'));
            });
      });
}, 'disconnect() called before FUNCTION_NAME. Reject with NetworkError.');
