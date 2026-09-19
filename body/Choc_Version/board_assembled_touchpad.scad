use <body_thin.scad>
use <body_thin_touchpad.scad>
use <board_cover_touchpad.scad>
use <reset_button.scad>
use <_buttons_low_profile.scad>
use <_auxiliary.scad>



module panelbuttons(fullheight = false) {
  bodyTouchpad();

  panel();

  buttons();

  translate([58, 13, 20]) {
    nice_nano_placeholder();
  }

  if (fullheight) {
    translate([0, 0, 23]) {
      cover_touchpad();
      resetButton();
    }
  } else {
    translate([0, 0, 16]) {
      cover_touchpad();
      resetButton();
    }
  }

  
}

module panelbuttonsmoved_touchpad(fullheight = false) {
  translate([0, 0, -4]) {
    panelbuttons(fullheight=fullheight);
  }
}

//panel();
panelbuttonsmoved_touchpad();

// sensor
showSensor = true;
if (showSensor) {
  translate([0, 0, 27]) {
    flatSensor();
  }
}
