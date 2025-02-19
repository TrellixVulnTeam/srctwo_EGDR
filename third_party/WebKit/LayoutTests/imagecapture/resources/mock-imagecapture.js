"use strict";

class MockImageCapture {
  constructor() {
    this.interceptor_ = new MojoInterfaceInterceptor(
        media.mojom.ImageCapture.name);
    this.interceptor_.oninterfacerequest =
        e => this.bindingSet_.addBinding(this, e.handle);
    this.interceptor_.start();

    this.state_ = {
      state: {
        supportedWhiteBalanceModes: [
          media.mojom.MeteringMode.SINGLE_SHOT,
          media.mojom.MeteringMode.CONTINUOUS
        ],
        currentWhiteBalanceMode: media.mojom.MeteringMode.CONTINUOUS,
        supportedExposureModes: [
          media.mojom.MeteringMode.SINGLE_SHOT,
          media.mojom.MeteringMode.CONTINUOUS
        ],
        currentExposureMode: media.mojom.MeteringMode.SINGLE_SHOT,
        supportedFocusModes: [
          media.mojom.MeteringMode.MANUAL,
          media.mojom.MeteringMode.SINGLE_SHOT
        ],
        currentFocusMode: media.mojom.MeteringMode.MANUAL,
        pointsOfInterest: [{x: 0.4, y: 0.6}],

        exposureCompensation:
            {min: -200.0, max: 200.0, current: 33.0, step: 33.0},
        colorTemperature:
            {min: 2500.0, max: 6500.0, current: 6000.0, step: 1000.0},
        iso: {min: 100.0, max: 12000.0, current: 400.0, step: 1.0},

        brightness: {min: 1.0, max: 10.0, current: 5.0, step: 1.0},
        contrast: {min: 2.0, max: 9.0, current: 5.0, step: 1.0},
        saturation: {min: 3.0, max: 8.0, current: 6.0, step: 1.0},
        sharpness: {min: 4.0, max: 7.0, current: 7.0, step: 1.0},

        zoom: {min: 0.0, max: 10.0, current: 5.0, step: 5.0},

        supportsTorch: true,
        torch: false,

        redEyeReduction: media.mojom.RedEyeReduction.CONTROLLABLE,
        height: {min: 240.0, max: 2448.0, current: 240.0, step: 2.0},
        width: {min: 320.0, max: 3264.0, current: 320.0, step: 3.0},
        fillLightMode: [
          media.mojom.FillLightMode.AUTO, media.mojom.FillLightMode.FLASH
        ],
      }
    };
    this.settings_ = null;
    this.bindingSet_ = new mojo.BindingSet(media.mojom.ImageCapture);
  }

  getPhotoState(source_id) {
    return Promise.resolve(this.state_);
  }

  setOptions(source_id, settings) {
    this.settings_ = settings;
    if (settings.hasIso)
      this.state_.state.iso.current = settings.iso;
    if (settings.hasHeight)
      this.state_.state.height.current = settings.height;
    if (settings.hasWidth)
      this.state_.state.width.current = settings.width;
    if (settings.hasZoom)
      this.state_.state.zoom.current = settings.zoom;
    if (settings.hasFocusMode)
      this.state_.state.currentFocusMode = settings.focusMode;

    if (settings.pointsOfInterest.length > 0) {
      this.state_.state.pointsOfInterest =
          settings.pointsOfInterest;
    }

    if (settings.hasExposureMode)
      this.state_.state.currentExposureMode = settings.exposureMode;

    if (settings.hasExposureCompensation) {
      this.state_.state.exposureCompensation.current =
          settings.exposureCompensation;
    }
    if (settings.hasWhiteBalanceMode) {
      this.state_.state.currentWhiteBalanceMode =
          settings.whiteBalanceMode;
    }
    if (settings.hasFillLightMode)
      this.state_.state.fillLightMode = [settings.fillLightMode];
    if (settings.hasRedEyeReduction)
      this.state_.state.redEyeReduction = settings.redEyeReduction;
    if (settings.hasColorTemperature) {
      this.state_.state.colorTemperature.current =
          settings.colorTemperature;
    }
    if (settings.hasBrightness)
      this.state_.state.brightness.current = settings.brightness;
    if (settings.hasContrast)
      this.state_.state.contrast.current = settings.contrast;
    if (settings.hasSaturation)
      this.state_.state.saturation.current = settings.saturation;
    if (settings.hasSharpness)
      this.state_.state.sharpness.current = settings.sharpness;

    if (settings.hasTorch)
      this.state_.state.torch = settings.torch;

    return Promise.resolve({ success : true });
  }

  takePhoto(source_id) {
    return Promise.resolve({ blob : { mimeType : 'image/cat',
                                      data : new Array(2) } });
  }

  state() {
    return this.state_.state;
  }

  options() {
    return this.settings_;
  }
}

let mockImageCapture = new MockImageCapture();
