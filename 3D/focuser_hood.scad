
include <roundedcube.scad>;

base_l=85;
base_h=6;
wall=2;
arc_w=34;

$fn=180;

difference() {
    // core
    roundedcube([base_l, 46+2*wall, base_h+32.75+wall], radius=1.5);
    
    // inside
    translate([0,wall,base_h]) {
        cube([base_l, 46, 32.75]);
    }
    translate([0,(46-(arc_w+1))/2+wall,0]) {
        cube([base_l, arc_w+1, base_h]);
    }
    
    // screw holes
    // hood screw holes
    translate([base_l-10,0,base_h/2]) {
        rotate([-90,0,0]) {
            cylinder(46+2*wall, d=3.2);
            cylinder(3, d=5.5);
            translate([0,0,46+2*wall-3]) {
                #cylinder(3, d=5.5);
            }
        }
    }
    translate([10,0,base_h/2]) {
        rotate([-90,0,0]) {
            cylinder(46+2*wall, d=3.2);
            cylinder(3, d=5.5);
            translate([0,0,46+2*wall-3]) {
                #cylinder(3, d=5.5);
            }
        }
    }
    
    // usb hole
    translate([0,0,32.75/2+base_h-8.5/2]) {
        cube([10.5,wall,8.5]);
    }
    
    // cables hole
    translate([base_l-20,45/2+wall-7/2,base_h+32.75]) {
        cube([20,7,wall-1]);
    }
    
    // sensor hole
    // TODO
}