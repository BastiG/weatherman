$fn = 20;

$pcb_w = 66.5;
$pcb_l = 101.5;
$pcb_h = 1.9;

$pcb_z = 4;
$pcb_top = $pcb_z + $pcb_h;

$wall = 2;
$dist = 0.5;

$hole_d = 3;
$stand_d = $hole_d + 2 * $wall;
$screw_d = 6;
$screw_l = 14;

$bottom_h = 8 + $dist + $pcb_h;
$top_h = 21 - $bottom_h;

$hole = [
    [4, 3.95],
    [$pcb_w-3.8, 3.95],
    [$pcb_w-3.6, $pcb_l-5.1],
    [3.8, $pcb_l-5.5]
];

/*translate([0, 0, $pcb_z])
    color("green", 1.0)
        #cube([$pcb_w, $pcb_l, $pcb_h]);*/

module bottom() {
    // Walls and floor
    translate([-$wall - $dist, -$wall, -$wall/2]) {
        cube([2 * $dist + 2 * $wall + $pcb_w, $dist + 2 * $wall + $pcb_l, $wall*2/3]);
        cube([2 * $dist + 2 * $wall + $pcb_w, $wall, $wall/2 + $bottom_h]);
        cube([$wall, $dist + 2 * $wall + $pcb_l, $wall/2 + $bottom_h]);
        cube([2 * $dist + 2 * $wall + $pcb_w, 0.5 * $wall, $wall/2 + $bottom_h + 2]);
        cube([0.5 * $wall, $dist + 2 * $wall + $pcb_l, $wall/2 + $bottom_h + 2]);
    }
    translate([$pcb_w + $dist + $wall, $pcb_l + $dist + $wall, -$wall/2]) {
        rotate([0,0,180]) {
            cube([2 * $dist + 2 * $wall + $pcb_w, $wall, $wall/2 + $bottom_h]);
            cube([$wall, $dist + 2 * $wall + $pcb_l, $wall/2 + $bottom_h]);
            cube([2 * $dist + 2 * $wall + $pcb_w, 0.5 * $wall, $wall/2 + $bottom_h + 2]);
            cube([0.5 * $wall, $dist + 2 * $wall + $pcb_l, $wall/2 + $bottom_h + 2]);
        }
    }

    // Covers for mounting holes
    translate([$pcb_w/2, $pcb_l/4, 0]) {
        difference() {
            union() {
                translate([0,-$hole_d,0])
                    cylinder(d=$screw_d+2*$wall, h=1.5*$wall, $fn=$fn);
                translate([0,$hole_d,0])
                    cylinder(d=$screw_d+2*$wall, h=1.5*$wall, $fn=$fn);
                translate([-($screw_d+2*$wall)/2,-$hole_d,0])
                    cube([$screw_d+2*$wall, 2*$hole_d, 1.5*$wall]);
            }
            translate([0,-$hole_d,0])
                cylinder(d=$screw_d, h=$wall, $fn=$fn);
            translate([0,$hole_d,0])
                cylinder(d=$screw_d, h=$wall, $fn=$fn);
            translate([-$screw_d/2,-$hole_d,0])
                cube([$screw_d, 2*$hole_d, $wall]);
        }
    }
    translate([$pcb_w/2, $pcb_l*3/4, 0]) {
        difference() {
            union() {
                translate([0,-$hole_d,0])
                    cylinder(d=$screw_d+2*$wall, h=1.5*$wall, $fn=$fn);
                translate([0,$hole_d,0])
                    cylinder(d=$screw_d+2*$wall, h=1.5*$wall, $fn=$fn);
                translate([-($screw_d+2*$wall)/2,-$hole_d,0])
                    cube([$screw_d+2*$wall, 2*$hole_d, 1.5*$wall]);
            }
            translate([0,-$hole_d,0])
                cylinder(d=$screw_d, h=$wall, $fn=$fn);
            translate([0,$hole_d,0])
                cylinder(d=$screw_d, h=$wall, $fn=$fn);
            translate([-$screw_d/2,-$hole_d,0])
                cube([$screw_d, 2*$hole_d, $wall]);
        }
    }

    // Standoffs for PCB + case screws
    translate([-0.5*$wall-$dist, -0.5*$wall, -0.5]) {
        cube([$hole[0][0]+0.5*$wall+$dist, $hole[0][1]+0.5*$wall+0.5*$stand_d, $pcb_z-1]);
        cube([$hole[0][0]+0.5*$wall+$dist+0.5*$stand_d, $hole[0][1]+0.5*$wall, $pcb_z-1]);
    }
    translate([$hole[1][0]-0.5*$stand_d, -0.5*$wall, -0.5])
        cube([$pcb_w+$dist+0.5*$wall-$hole[1][0]+0.5*$stand_d, $hole[1][1]+0.5*$wall, $pcb_z-1]);
    translate([$hole[1][0], -0.5*$wall, -0.5])
        cube([$pcb_w+$dist+0.5*$wall-$hole[1][0], $hole[1][1]+0.5*$wall+0.5*$stand_d, $pcb_z-1]);
    translate([$hole[2][0]-0.5*$stand_d, $hole[2][1], -0.5])
        cube([$pcb_w+$dist+0.5*$wall-$hole[2][0]+0.5*$stand_d, $pcb_l-$hole[2][1]+0.5*$wall, $pcb_z-1]);
    translate([$hole[2][0], $hole[2][1]-0.5*$stand_d, -0.5])
        cube([$pcb_w+$dist+0.5*$wall-$hole[1][0], $pcb_l-$hole[2][1]+0.5*$wall+0.5*$stand_d, $pcb_z-1]);
    translate([-0.5*$wall-$dist, $hole[3][1]-0.5*$stand_d, -0.5])
        cube([$hole[3][0]+0.5*$wall+$dist, $pcb_l-$hole[3][1]+0.5*$wall+0.5*$stand_d, $pcb_z-1]);
    translate([-0.5*$wall-$dist+0.5*$stand_d, $hole[3][1], -0.5])
        cube([$hole[3][0]+0.5*$wall+$dist, $pcb_l-$hole[3][1]+0.5*$wall, $pcb_z-1]);
    
    translate([$hole[0][0], $hole[0][1], -0.5]) {
        cylinder(d=$stand_d, h=$pcb_z+0.5, $fn=$fn);
        cylinder(d=$stand_d+$wall, h=$pcb_z-1, $fn=$fn);
    }
    translate([$hole[1][0], $hole[1][1], -0.5]) {
        cylinder(d=$stand_d, h=$pcb_z+0.5, $fn=$fn);
        cylinder(d=$stand_d+$wall, h=$pcb_z-1, $fn=$fn);
    }
    translate([$hole[2][0], $hole[2][1], -0.5]) {
        cylinder(d=$stand_d, h=$pcb_z+0.5, $fn=$fn);
        cylinder(d=$stand_d+$wall, h=$pcb_z-1, $fn=$fn);
    }
    translate([$hole[3][0], $hole[3][1], -0.5]) {
        cylinder(d=$stand_d, h=$pcb_z+0.5, $fn=$fn);
        cylinder(d=$stand_d+$wall, h=$pcb_z-1, $fn=$fn);
    }
}

module top() {
    // Walls and ceiling
    translate([-0.4 * $wall - $dist, -0.4 * $wall, $bottom_h+0.5]) {
        cube([2 * $dist + 0.8*$wall + $pcb_w, 0.4 * $wall, 3]);
        cube([0.5 * $wall, $dist + 0.8*$wall + $pcb_l, 3]);
    }
    translate([-$wall - $dist, -$wall, $bottom_h+2]) {
        cube([2 * $dist + 2 * $wall + $pcb_w, $wall, $wall + $top_h]);
        cube([$wall, $dist + 2 * $wall + $pcb_l, $wall + $top_h]);
    }
    translate([-$wall - $dist, -$wall, $bottom_h + $top_h + 2 + $wall - $wall*2/3]) {
        cube([2 * $dist + 2 * $wall + $pcb_w, $dist + 2 * $wall + $pcb_l, $wall*2/3]);
    }
    
    // Closing notch
    translate([$pcb_w + $dist + 0.4 * $wall, $pcb_l + $dist + 0.4 * $wall, $bottom_h+0.5]) {
        rotate([0,0,180]) {
            cube([2 * $dist + 0.8*$wall + $pcb_w, 0.4 * $wall, 3]);
            cube([0.4 * $wall, $dist + 0.8*$wall + $pcb_l, 3]);
        }
    }
    
    translate([$pcb_w + $dist + $wall, $pcb_l + $dist + $wall, $bottom_h+2]) {
        rotate([0,0,180]) {
        cube([2 * $dist + 2 * $wall + $pcb_w, $wall, $wall + $top_h]);
        cube([$wall, $dist + 2 * $wall + $pcb_l, $wall + $top_h]);
        }
    }

    // Stanoffs for PCB + case screws
    translate([0,0,$bottom_h+1]) {
        translate([-0.4*$wall-$dist, -0.4*$wall, -0.5]) {
            cube([$hole[0][0]+0.4*$wall+$dist, $hole[0][1]+0.4*$wall+0.5*$stand_d, $top_h+3]);
            cube([$hole[0][0]+0.4*$wall+$dist+0.5*$stand_d, $hole[0][1]+0.4*$wall, $top_h+3]);
        }
        translate([$hole[1][0]-0.5*$stand_d, -0.4*$wall, -0.5])
            cube([$pcb_w+$dist+0.4*$wall-$hole[1][0]+0.5*$stand_d, $hole[1][1]+0.4*$wall, $top_h+3]);
        translate([$hole[1][0], -0.4*$wall, -0.5])
            cube([$pcb_w+$dist+0.4*$wall-$hole[1][0], $hole[1][1]+0.4*$wall+0.5*$stand_d, $top_h+3]);
        translate([$hole[2][0]-0.5*$stand_d, $hole[2][1], -0.5])
            cube([$pcb_w+$dist+0.4*$wall-$hole[2][0]+0.5*$stand_d, $pcb_l-$hole[2][1]+0.5*$wall, $top_h+3]);
        translate([$hole[2][0], $hole[2][1]-0.5*$stand_d, -0.5])
            cube([$pcb_w+$dist+0.3*$wall-$hole[1][0], $pcb_l-$hole[2][1]+0.4*$wall+0.5*$stand_d, $top_h+3]);
        translate([-0.4*$wall-$dist, $hole[3][1]-0.5*$stand_d, -0.5])
            cube([$hole[3][0]+0.5*$wall+$dist, $pcb_l-$hole[3][1]+0.5*$wall+0.5*$stand_d, $top_h+3]);
        translate([-0.5*$wall-$dist+0.5*$stand_d, $hole[3][1], -0.5])
            cube([$hole[3][0]+0.5*$wall+$dist, $pcb_l-$hole[3][1]+0.5*$wall, $top_h+3]);
    }
    translate([$hole[0][0], $hole[0][1], $pcb_top+0.5])
        cylinder(d=$stand_d, h=$bottom_h + $top_h - $wall - 1, $fn=$fn);
    translate([$hole[1][0], $hole[1][1], $pcb_top+0.5])
        cylinder(d=$stand_d, h=$bottom_h + $top_h - $wall - 1, $fn=$fn);
    translate([$hole[2][0], $hole[2][1], $pcb_top+0.5])
        cylinder(d=$stand_d, h=$bottom_h + $top_h - $wall - 1, $fn=$fn);
    translate([$hole[3][0], $hole[3][1], $pcb_top+0.5])
        cylinder(d=$stand_d, h=$bottom_h + $top_h - $wall - 1, $fn=$fn);
}

module holes() {
    // Mounting holes
    translate([$pcb_w/2, $pcb_l/4, -$wall]) {
        translate([0,-$hole_d,0])
            cylinder(d=$screw_d, h=2*$wall, $fn=$fn);
        translate([0,$hole_d,0])
            cylinder(d=$hole_d, h=2*$wall, $fn=$fn);
        translate([-$hole_d/2,-$hole_d,0])
            cube([$hole_d, 2*$hole_d, 2*$wall]);
    }
    translate([$pcb_w/2, $pcb_l*3/4, -$wall]) {
        translate([0,-$hole_d,0])
            cylinder(d=$screw_d, h=2*$wall, $fn=$fn);
        translate([0,$hole_d,0])
            cylinder(d=$hole_d, h=2*$wall, $fn=$fn);
        translate([-$hole_d/2,-$hole_d,0])
            cube([$hole_d, 2*$hole_d, 2*$wall]);
    }
    
    // Case screws
    translate([$hole[0][0], $hole[0][1], -$wall]) {
        cylinder(d=$screw_d, h=$wall, $fn=$fn);
        cylinder(d=$hole_d, h=$wall+$pcb_z+0.5, $fn=$fn);
        cylinder(d=$hole_d*0.9, h=$screw_l, $fn=$fn);
    }
    translate([$hole[1][0], $hole[1][1], -$wall]) {
        cylinder(d=$screw_d, h=$wall, $fn=$fn);
        cylinder(d=$hole_d, h=$wall+$pcb_z+0.5, $fn=$fn);
        cylinder(d=$hole_d*0.9, h=$screw_l, $fn=$fn);
    }
    translate([$hole[2][0], $hole[2][1], -$wall]) {
        cylinder(d=$screw_d, h=$wall, $fn=$fn);
        cylinder(d=$hole_d, h=$wall+$pcb_z+0.5, $fn=$fn);
        cylinder(d=$hole_d*0.9, h=$screw_l, $fn=$fn);
    }
    translate([$hole[3][0], $hole[3][1], -$wall]) {
        cylinder(d=$screw_d, h=$wall, $fn=$fn);
        cylinder(d=$hole_d, h=$wall+$pcb_z+0.5, $fn=$fn);
        cylinder(d=$hole_d*0.9, h=$screw_l, $fn=$fn);
    }
    
    // DC Jack
    translate([7.2, -$wall-0.5, $pcb_top])
        cube([9.3, $wall+10, 11]);
    // RJ45
    translate([40, -$wall-0.5, $pcb_top])
        cube([16.8, $wall+1, 13.1]);
}

difference() {
    bottom();
    holes();
}

translate([2 * $pcb_w + 10, 0, $bottom_h + $top_h + 2 + $wall - $wall/2])
rotate([0, 180, 0])
difference() {
    top();
    holes();
}