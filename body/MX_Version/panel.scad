difference() {

  linear_extrude(1.5) {
    //minkowski() {
    import("plate_outline.svg");
    //circle(0.1);
    //}
  }

  translate([0, 0, -5])
    linear_extrude(10) {
      import("buttons.svg");
    }

  translate([0, 0, -5])
    linear_extrude(10) {
      import("holes.svg");
    }
}
