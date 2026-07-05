use <body_thin.scad>
use <board_cover.scad>


module button() {
  rotate([0, 0, 45]) {
    cylinder(h=10, r1=12, r2=10, center=true, $fn=4);
  }
}

module button3() {
  translate([0, 18 * 2, 0]) {
    button();
  }

  translate([0, 18, 0]) {
    button();
  }
  button();
}

module buttons() {

  translate([0, 0, 30]) {

    translate([-19 * 2, -11, 0]) {
      button3();
    }

    translate([-19, -9, 0]) {
      button3();
    }

    translate([0, -7, 0]) {
      button3();
    }

    translate([18.5, -9, 0]) {
      button3();
    }

    translate([18.5 * 2, -13, 0]) {
      button3();
    }

    translate([18.5 * 3, -13, 0]) {
      button3();
    }

    translate([-52, -35, 0]) {
      rotate([0, 0, 23]) {
        scale([1, 1.5, 1]) {
          button();
        }
      }
    }

    translate([-32, -32, 0]) {
      rotate([0, 0, 13]) {
        button();
      }
    }

    translate([-10, -30, 0]) {
      rotate([0, 0, 1]) {
        button();
      }
    }
  }
}


module panelbuttons(fullheight=false) {
  body();

  scale([-1, 1, 1]) {
    panel();
  }

  buttons();

  translate([-58, 13, 20]) {
    cube([20, 40, 10], center=true);
  }

  if (fullheight) {
  translate([0, 0, 23]) {
    cover();
  }
  } else {
  translate([0, 0, 15]) {
    cover();
  }
  }
}

module panelbuttonsmoved(fullheight=false) {
  translate([0, 0, -3]) {
    panelbuttons(fullheight=fullheight);
  }
}



module twokeyboards(fullheight=false) {
  panelbuttonsmoved(fullheight=fullheight);

  scale([1, 1, -1]) {
    rotate([0, 0, 180]) {
      panelbuttonsmoved(fullheight=fullheight);
      //bottom_panel();
    }
  }
}

panelbuttonsmoved();