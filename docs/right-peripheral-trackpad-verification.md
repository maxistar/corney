# Right-peripheral Cirque verification

> Historical acceptance record: its right-peripheral topology and physical results remain valid,
> but references to Studio behavior on the central are superseded by the Studio-free release.

This record covers the `move-corney-trackpad-to-right-peripheral` change on the pinned ZMK
`v0.3.0` commit `edf5c0814fd3ea202e43aad2d68fd32e882a518c`. The left nice!nano remains the
split central and host endpoint; the right nice!nano owns the I2C sensor and forwards its events
through input-split.

## Automated evidence

- [x] The enhanced left-central image builds with a `zmk,input-split` proxy and
  `zmk,input-listener`, without the Cirque device, I2C, or polling driver.
- [x] The existing `corney-right` image builds with I2C, the polling driver, Cirque at `0x2a`, and
  an input-split endpoint referencing the sensor.
- [x] Native integration coverage observes relative X/Y, wheel, primary-button press, a synthetic
  release at disconnect, and a subsequent event after reconnect with synchronization flags intact.
- [x] The build matrix still declares exactly one `corney-right` artifact.
- [x] Every target in `build.yaml` has been rebuilt and checked by
  `tests/verify_cirque_build.py`.
- [x] Host, native integration, BLE contract, and metadata suites all pass together.

The six `nice_nano_v2` artifacts were rebuilt from the pinned checkout. Generated configuration
inspection found the proxy-only topology in all four left-central variants, the physical sensor
endpoint only in `corney-right`, and neither component in `settings-reset`.

## Sensor-equipped right half

Record the firmware revision, printed-part revision, battery state, and host endpoints used.

The first prototype verified split pointing with the sensor in its original orientation and
`invert-x`. The final installation rotates the sensor 180 degrees so its flex exits into free space;
the matching candidate firmware therefore uses `invert-y` and no `invert-x`.

Candidate for the rotated installation: `CorneyMX-right-rotated-180.uf2`, SHA-256
`f7b7a74931fbe5ddcad91d0888dd13a3f5954c8e8d1eb1066bdf61b1280a6185`.

- [x] Finger right/left and up/down move the cursor in the same physical direction with the sensor
  rotated 180 degrees, `invert-y` enabled, and `invert-x` disabled.
- [ ] Relative wheel packets reach both USB and BLE HID endpoints. Wheel input was not observed with
  the alternate sensor used for acceptance; this may be a sensor limitation and is deferred until a
  replacement sensor is available.
- [ ] Primary tap presses and releases exactly once without a stuck button.
- [ ] Fast pointer movement remains responsive while ordinary keys and combos are typed on both
  halves.
- [x] Disconnecting and reconnecting the right half restores pointing operation.
- [ ] Disconnecting the right half during an active button state releases the host button.
- [ ] After suspend, touch alone does not claim to wake the right half; a right-side key wake
  reinitializes the sensor and pointing resumes after reconnect.
- [ ] Right-side battery reporting remains plausible. Record an active-use observation for the
  retained 8 ms polling interval.

## Sensorless right half

Flash the exact same `corney-right` image used above on a half without the Cirque installed.

- [x] Sensor initialization emits at most its bounded startup diagnostic and does not start a
  recurring polling-error loop.
- [x] Right-side keys, split connection and reconnect, suspend/resume, and battery reporting remain
  usable.

## Mechanical acceptance

- [ ] `right_touchpad_body.stl` and `right_touchpad_cover.stl` print and mate with the ordinary left
  half in the intended keyboard orientation.
- [ ] The Cirque aperture, controller USB port, reset control, power control, I2C routing, fasteners,
  and internal clearances are accessible and unobstructed.

## Keyboard regressions

- [ ] Ordinary keys, layers, ordinary and overlapping combos, and host switching retain their
  accepted behavior.
- [ ] Battery Service, Keyboard Helper subscriptions and layer telemetry, legacy layer control, and
  Studio connectivity retain their accepted behavior.

## Archive disposition

The change was accepted for archival with explicit hardware-test exceptions. Wheel behavior on the
alternate installed sensor remains unresolved until a replacement sensor is available. Local-key
wake/resume, active-button disconnect release, simultaneous fast pointing and typing, final
enclosure clearances, and the full regression checklist remain unverified. These unchecked items
must not be represented as completed release acceptance; follow-up work may resume them when the
new enclosure and sensor are available.
