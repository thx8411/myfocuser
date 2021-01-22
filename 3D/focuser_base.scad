arc_d=317;
arc_l=69;
arc_w=34;
arc_h=3.8;

base_l=85;
base_h=6;

main_screw_d=8;

screw_d=3.25;
screw_space_l=34;
screw_space_w=17;

screw_lips_d=5.5;
screw_lips_h=3;

$fn=180;

module retaining_screw_positiv() {
    cylinder(screw_lips_h+arc_h, d=screw_lips_d);
}


module retaining_screw_negativ() {
    translate([0,0,-screw_lips_h]) {
        cylinder(arc_h+base_h+screw_lips_h, d=screw_d);
    }
    translate([0,0,5]) {
        cylinder(screw_lips_h, d=screw_lips_d);
    }
}

module byj_holder() {
    translate([0,-45/2,0]) {
        difference() {
            cube([17,45,32]);
        
            // motor hole
            translate([0,45/2,15]) {
                rotate([0,90,0]) {
                    cylinder(20, d=28.5);
                }
            }
            
            // moto cables hole
            translate([0,45/2-20/2,20]) {
                cube([20,20,20]);
            }
        
            // screw holes
            translate([0,45/2-35/2,15]) {
                rotate([0,90,0]) {
                    cylinder(20, d=2.8);
                }
            }
            translate([0,45/2+35/2,15]) {
                rotate([0,90,0]) {
                    cylinder(20, d=2.8);
                }
            }
        }
    }
}

module unl2003_holder() {
    translate([0,-35/2,0]) {
        difference() {
            cube([4,35,32]);
            // screw holes
            translate([0,35/2-29.5/2,32/2+26.5/2]) {
                rotate([0,90,0]) {
                    cylinder(4, d=2.8);
                }
            }
            translate([0,35/2+29.5/2,32/2-26.5/2]) {
                rotate([0,90,0]) {
                    cylinder(4, d=2.8);
                }
            }
            translate([0,35/2+29.5/2,32/2+26.5/2]) {
                rotate([0,90,0]) {
                    cylinder(4, d=2.8);
                }
            }
            translate([0,35/2-29.5/2,32/2-26.5/2]) {
                rotate([0,90,0]) {
                    cylinder(4, d=2.8);
                }
            }
        }
    }
}

module nano_holder() {
    translate([0,-45/2,0]) {
        difference() {
            cube([7,45,32]);
            translate([0,1.5,32/2-17.5/2]) {
                cube([5,45-1.5,17.5]);
            }
            translate([1,1.5,32/2-18.5/2]) {
                cube([2,45-1.5,18.5]);
            }
            // temp sensor hole
            // TO FIX
            translate([7/2+2.5/2,45/2,28]) {
                rotate([0,90,0]) {
                    cylinder(2.5, d=5);
                    translate([0,0,-10+2.5]) { 
                        cylinder(10-2.5, d=4);
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////

// base
difference() {
    translate([0,-arc_w/2,-base_h]) {
        rotate([-90,0,0]) {
            union() {
                // arc
                translate([0,-arc_d/2+arc_h,0]) {
                    intersection() {
                        cylinder(arc_w, d=arc_d);
                        translate([-arc_l/2,arc_d/2-arc_h,0]) {
                            cube([arc_l,arc_h,arc_w]);
                        }
                    }
                }
                // base
                translate([-base_l/2,-base_h,0]) {
                    cube([base_l,base_h,arc_w]);
                }
            }
        }
        // retaining screws
        translate([-screw_space_l/2,-screw_space_w/2+arc_w/2,-(arc_h+base_h-screw_lips_h)+2]) {
            retaining_screw_positiv();
        }
        translate([screw_space_l/2,-screw_space_w/2+arc_w/2,-(arc_h+base_h-screw_lips_h)+2]) {
            retaining_screw_positiv();
        }
        translate([-screw_space_l/2,screw_space_w/2+arc_w/2,-(arc_h+base_h-screw_lips_h)+2]) {
            retaining_screw_positiv();
        }
        translate([screw_space_l/2,screw_space_w/2+arc_w/2,-(arc_h+base_h-screw_lips_h)+2]) {
            retaining_screw_positiv();
        }
    }
    // main screw
    translate([0,0,-(arc_h+base_h)]) {
        cylinder(arc_h+base_h, d=main_screw_d);
    }
    // retaining screws
    translate([-screw_space_l/2,-screw_space_w/2,-(arc_h+base_h)+2]) {
        retaining_screw_negativ();
    }
    translate([screw_space_l/2,-screw_space_w/2,-(arc_h+base_h)+2]) {
        retaining_screw_negativ();
    }
    translate([-screw_space_l/2,screw_space_w/2,-(arc_h+base_h)+2]) {
        retaining_screw_negativ();
    }
    translate([screw_space_l/2,screw_space_w/2,-(arc_h+base_h)+2]) {
        retaining_screw_negativ();
    }
    
    // hood screw holes
    translate([base_l/2-10,arc_w/2+1,-base_h/2]) {
        rotate([90,0,0]) {
            cylinder(arc_w+2, d=2.8);
        }
    }
    translate([-base_l/2+10,arc_w/2+1,-base_h/2]) {
        rotate([90,0,0]) {
            cylinder(arc_w+2, d=2.8);
        }
    }   
}

translate([-base_l/2,0,0]) {
    byj_holder();
}

translate([-base_l/2+28,0,0]) {
    unl2003_holder();
}

translate([+base_l/2-7,0,0]) {
    nano_holder();
}