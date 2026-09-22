use <body_thin.scad>
use <board_cover.scad>
use <reset_button_right.scad>
use <_buttons_low_profile.scad>
use <_auxiliary.scad>


module panelbuttons(fullheight = false, simplified = false) {
  if (simplified) {
    body_simplified();
  } else {
    body();
    panel();
  }


  buttons();

  /*
  translate([58, 13, 20]) {
    nice_nano_placeholder();
  }
  */

  if (simplifiled) {
    translate([0, 0, 15.5]) {
      cover_simplified();
      resetButtonSmall();
    }
  } else {
    translate([0, 0, 15.5]) {
      cover();
      resetButtonSmall();
    }
  }

  
}

module panelbuttonsmoved(fullheight = false, simplified=false) {
  translate([0, 0, -4]) {
    panelbuttons(fullheight=fullheight, simplified = simplified);
  }
}

//panel();
panelbuttonsmoved(simplifiled = true);

