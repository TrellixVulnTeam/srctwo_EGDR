// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Javascript for bluetooth_internals.html, served from
 *     chrome://bluetooth-internals/.
 */

// Expose for testing.
/** @type {adapter_broker.AdapterBroker} */
var adapterBroker = null;
/** @type {device_collection.DeviceCollection} */
var devices = null;
/** @type {sidebar.Sidebar} */
var sidebarObj = null;

cr.define('bluetooth_internals', function() {
  /** @const */ var AdapterPage = adapter_page.AdapterPage;
  /** @const */ var DeviceDetailsPage = device_details_page.DeviceDetailsPage;
  /** @const */ var DevicesPage = devices_page.DevicesPage;
  /** @const */ var PageManager = cr.ui.pageManager.PageManager;
  /** @const */ var Snackbar = snackbar.Snackbar;
  /** @const */ var SnackbarType = snackbar.SnackbarType;

  devices = new device_collection.DeviceCollection([]);

  /** @type {adapter_page.AdapterPage} */
  var adapterPage = null;
  /** @type {devices_page.DevicesPage} */
  var devicesPage = null;

  /** @type {interfaces.BluetoothAdapter.DiscoverySession.ptrClass} */
  var discoverySession = null;

  /** @type {boolean} */
  var userRequestedScanStop = false;

  /**
   * Observer for page changes. Used to update page title header.
   * @extends {cr.ui.pageManager.PageManager.Observer}
   */
  var PageObserver = function() {};

  PageObserver.prototype = {
    __proto__: PageManager.Observer.prototype,

    updateHistory: function(path) {
      window.location.hash = '#' + path;
    },

    /**
     * Sets the page title. Called by PageManager.
     * @override
     * @param {string} title
     */
    updateTitle: function(title) {
      document.querySelector('.page-title').textContent = title;
    },
  };

  /**
   * Removes DeviceDetailsPage with matching device |address|. The associated
   * sidebar item is also removed.
   * @param {string} address
   */
  function removeDeviceDetailsPage(address) {
    var id = 'devices/' + address.toLowerCase();
    sidebarObj.removeItem(id);

    var deviceDetailsPage = PageManager.registeredPages[id];
    assert(deviceDetailsPage, 'Device Details page must exist');

    deviceDetailsPage.disconnect();
    deviceDetailsPage.pageDiv.parentNode.removeChild(deviceDetailsPage.pageDiv);

    // Inform the devices page that the user is inspecting this device.
    // This will update the links in the device table.
    devicesPage.setInspecting(
        deviceDetailsPage.deviceInfo, false /* isInspecting */);

    PageManager.unregister(deviceDetailsPage);
  }

  /**
   * Creates a DeviceDetailsPage with the given |deviceInfo|, appends it to
   * '#page-container', and adds a sidebar item to show the new page. If a
   * page exists that matches |deviceInfo.address|, nothing is created and the
   * existing page is returned.
   * @param {!interfaces.BluetoothDevice.Device} deviceInfo
   * @return {!device_details_page.DeviceDetailsPage}
   */
  function makeDeviceDetailsPage(deviceInfo) {
    var deviceDetailsPageId = 'devices/' + deviceInfo.address.toLowerCase();
    var deviceDetailsPage = PageManager.registeredPages[deviceDetailsPageId];
    if (deviceDetailsPage)
      return deviceDetailsPage;

    var pageSection = document.createElement('section');
    pageSection.hidden = true;
    pageSection.id = deviceDetailsPageId;
    $('page-container').appendChild(pageSection);

    deviceDetailsPage = new DeviceDetailsPage(deviceDetailsPageId, deviceInfo);
    deviceDetailsPage.pageDiv.addEventListener(
        'connectionchanged', function(event) {
          devices.updateConnectionStatus(
              event.detail.address, event.detail.status);
        });

    deviceDetailsPage.pageDiv.addEventListener('infochanged', function(event) {
      devices.addOrUpdate(event.detail.info);
    });

    deviceDetailsPage.pageDiv.addEventListener(
        'forgetpressed', function(event) {
          PageManager.showPageByName(devicesPage.name);
          removeDeviceDetailsPage(event.detail.address);
        });

    // Inform the devices page that the user is inspecting this device.
    // This will update the links in the device table.
    devicesPage.setInspecting(deviceInfo, true /* isInspecting */);
    PageManager.register(deviceDetailsPage);

    sidebarObj.addItem({
      pageName: deviceDetailsPageId,
      text: deviceInfo.name_for_display,
    });

    deviceDetailsPage.connect();
    return deviceDetailsPage;
  }

  /**
   * Updates the DeviceDetailsPage with the matching device |address| and
   * redraws it.
   * @param {string} address
   */
  function updateDeviceDetailsPage(address) {
    var detailPageId = 'devices/' + address.toLowerCase();
    var page = PageManager.registeredPages[detailPageId];
    if (page)
      page.redraw();
  }

  function updateStoppedDiscoverySession() {
    devicesPage.setScanStatus(devices_page.ScanStatus.OFF);
    discoverySession.ptr.reset();
    discoverySession = null;
  }

  function setupAdapterSystem(response) {
    adapterBroker.addEventListener('adapterchanged', function(event) {
      adapterPage.adapterFieldSet.value[event.detail.property] =
          event.detail.value;
      adapterPage.redraw();

      if (event.detail.property == adapter_broker.AdapterProperty.DISCOVERING &&
          !event.detail.value && !userRequestedScanStop && discoverySession) {
        updateStoppedDiscoverySession();
        Snackbar.show(
            'Discovery session ended unexpectedly', SnackbarType.WARNING);
      }
    });

    adapterPage.setAdapterInfo(response.info);

    adapterPage.pageDiv.addEventListener('refreshpressed', function() {
      adapterBroker.getInfo().then(function(response) {
        adapterPage.setAdapterInfo(response.info);
      });
    });
  }

  function setupDeviceSystem(response) {
    // Hook up device collection events.
    adapterBroker.addEventListener('deviceadded', function(event) {
      devices.addOrUpdate(event.detail.deviceInfo);
      updateDeviceDetailsPage(event.detail.deviceInfo.address);
    });
    adapterBroker.addEventListener('devicechanged', function(event) {
      devices.addOrUpdate(event.detail.deviceInfo);
      updateDeviceDetailsPage(event.detail.deviceInfo.address);
    });
    adapterBroker.addEventListener('deviceremoved', function(event) {
      devices.remove(event.detail.deviceInfo);
      updateDeviceDetailsPage(event.detail.deviceInfo.address);
    });

    response.devices.forEach(devices.addOrUpdate, devices /* this */);

    devicesPage.setDevices(devices);

    devicesPage.pageDiv.addEventListener('inspectpressed', function(event) {
      var detailsPage =
          makeDeviceDetailsPage(devices.getByAddress(event.detail.address));
      PageManager.showPageByName(detailsPage.name);
    });

    devicesPage.pageDiv.addEventListener('forgetpressed', function(event) {
      PageManager.showPageByName(devicesPage.name);
      removeDeviceDetailsPage(event.detail.address);
    });

    devicesPage.pageDiv.addEventListener('scanpressed', function(event) {
      if (discoverySession && discoverySession.ptr.isBound()) {
        userRequestedScanStop = true;
        devicesPage.setScanStatus(devices_page.ScanStatus.STOPPING);

        discoverySession.stop().then(function(response) {
          if (response.success) {
            updateStoppedDiscoverySession();
            userRequestedScanStop = false;
            return;
          }

          devicesPage.setScanStatus(devices_page.ScanStatus.ON);
          Snackbar.show('Failed to stop discovery session', SnackbarType.ERROR);
          userRequestedScanStop = false;
        });

        return;
      }

      devicesPage.setScanStatus(devices_page.ScanStatus.STARTING);
      adapterBroker.startDiscoverySession()
          .then(function(session) {
            discoverySession = session;

            discoverySession.ptr.setConnectionErrorHandler(function() {
              updateStoppedDiscoverySession();
              Snackbar.show('Discovery session ended', SnackbarType.WARNING);
            });

            devicesPage.setScanStatus(devices_page.ScanStatus.ON);
          })
          .catch(function(error) {
            devicesPage.setScanStatus(devices_page.ScanStatus.OFF);
            Snackbar.show(
                'Failed to start discovery session', SnackbarType.ERROR);
            console.error(error);
          });
    });
  }

  function setupPages() {
    sidebarObj = new window.sidebar.Sidebar($('sidebar'));
    $('menu-btn').addEventListener('click', function() {
      sidebarObj.open();
    });
    PageManager.addObserver(sidebarObj);
    PageManager.addObserver(new PageObserver());

    devicesPage = new DevicesPage();
    PageManager.register(devicesPage);
    adapterPage = new AdapterPage();
    PageManager.register(adapterPage);

    // Set up hash-based navigation.
    window.addEventListener('hashchange', function() {
      // If a user navigates and the page doesn't exist, do nothing.
      var pageName = window.location.hash.substr(1);
      if ($(pageName))
        PageManager.showPageByName(pageName);
    });

    if (!window.location.hash) {
      PageManager.showPageByName(adapterPage.name);
      return;
    }

    // Only the root pages are available on page load.
    PageManager.showPageByName(window.location.hash.split('/')[0].substr(1));
  }

  function initializeViews() {
    setupPages();

    adapter_broker.getAdapterBroker()
        .then(function(broker) {
          adapterBroker = broker;
        })
        .then(function() {
          return adapterBroker.getInfo();
        })
        .then(setupAdapterSystem)
        .then(function() {
          return adapterBroker.getDevices();
        })
        .then(setupDeviceSystem)
        .catch(function(error) {
          Snackbar.show(error.message, SnackbarType.ERROR);
          console.error(error);
        });
  }

  return {
    initializeViews: initializeViews,
  };
});

document.addEventListener(
    'DOMContentLoaded', bluetooth_internals.initializeViews);
