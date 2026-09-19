use <body_thin.scad>

module nice_nano_placeholder() {
  cube([20, 40, 6], center=true);
}

module flatSensor() {
        translate(getSensorPosition()) {
      circle(r=getSensorRadius());
    }
}