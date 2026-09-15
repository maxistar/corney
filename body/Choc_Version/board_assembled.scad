use <body_thin.scad>
use <board_cover.scad>
use <_buttons_low_profile.scad>

module nice_nano_placeholder() {
  cube([20, 40, 6], center=true);
}

module panelbuttons(fullheight = false) {
  body();

  panel();

  buttons();

  translate([58, 13, 20]) {
    nice_nano_placeholder();
  }

  if (fullheight) {
    translate([0, 0, 23]) {
      cover();
      resetButton();
    }
  } else {
    translate([0, 0, 16]) {
      cover();
      resetButton();
    }
  }

  
}

module panelbuttonsmoved(fullheight = false) {
  translate([0, 0, -3]) {
    panelbuttons(fullheight=fullheight);
  }
}

module twokeyboards(fullheight = false) {
  panelbuttonsmoved(fullheight=fullheight);

  scale([1, 1, -1]) {
    rotate([0, 0, 180]) {
      panelbuttonsmoved(fullheight=fullheight);
      //bottom_panel();
    }
  }
}

//panel();
panelbuttonsmoved();

// sensor
showSensor = true;
if (showSensor) {
  translate([0, 0, 27]) {
    translate(getSensorPosition()) {
      circle(r=getSensorRadius());
    }
  }
}
