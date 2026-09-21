use <body_thin.scad>
use <board_cover.scad>

$fn = 150;

module resetButton(buttonHeight = 11) {
  // reset button
  translate([0, 0, 1]) {
    translate(getResetButtonPosition()) {
      linear_extrude(buttonHeight) {
        circle(r=getResetButtonRadius() - 0.1);
      }
    }
  }

  translate([0, 0, -1]) {
    translate(getResetButtonPosition()) {
      linear_extrude(2) {
        circle(r=getResetButtonRadius() + 0.5);
      }
    }
  }
}


resetButton();

