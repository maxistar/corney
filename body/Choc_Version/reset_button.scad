use <body_thin.scad>
use <board_cover.scad>

module resetButton(buttonHeight = 12) {
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

module resetButtonSmall() {
  resetButton(8);
}

resetButton();

translate([10,0,0]) {
resetButtonSmall();
}