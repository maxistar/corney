use <body_thin.scad>
use <board_cover.scad>
use <reset_button_right.scad>
use <_buttons_low_profile.scad>
use <_auxiliary.scad>


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
      resetButtonSmall();
    }
  } else {
    translate([0, 0, 15.5]) {
      cover();
      resetButtonSmall();
    }
  }

  
}

module panelbuttonsmoved(fullheight = false) {
  translate([0, 0, -4]) {
    panelbuttons(fullheight=fullheight);
  }
}

//panel();
panelbuttonsmoved();

