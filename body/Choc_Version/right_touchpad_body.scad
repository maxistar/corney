use <body_thin_touchpad.scad>

// The touchpad enclosure geometry is the canonical right-hand Corney half.
// Keep this explicit wrapper as the printable entry point so releases do not
// require users to infer the intended side from a generic filename.
module rightTouchpadBody() {
  bodyTouchpad();
}

rightTouchpadBody();
