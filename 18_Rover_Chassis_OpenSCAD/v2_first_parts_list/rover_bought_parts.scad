// =====================================================================
//  GrassKit Rover v2 - stand-ins for bought parts (visual only).
//  Included by grasskit_rover.scad.
// =====================================================================

module mecanum(hand = 1) {
    n  = 9;
    rd = wheel_d * 0.26;
    rl = wheel_w * 0.95;
    rr = wheel_d / 2 - rd / 2;
    color("DimGray") rotate([90, 0, 0]) cylinder(d = wheel_d * 0.62, h = wheel_w - 4, center = true);
    color("Silver") for (s = [-1, 1]) translate([0, s * (wheel_w / 2 - 1.5), 0])
        rotate([90, 0, 0]) cylinder(d = wheel_d * 0.82, h = 3, center = true);
    color("Black") for (i = [0 : n - 1]) rotate([0, i * 360 / n, 0]) translate([rr, 0, 0])
        rotate([hand * 45, 0, 0]) rotate([-90, 0, 0]) scale([1, 1, rl / rd]) sphere(d = rd, $fn = 16);
}

// Axis along Z, output shaft at +Z.
module gearmotor() {
    color("Gold") cylinder(d = dm_d, h = dm_len, center = true);
    color("Silver") translate([0, 0, dm_len / 2]) cylinder(d = dm_shaft, h = 10);
    color("DimGray") translate([0, 0, dm_len / 2 + 4]) cylinder(d = 12, h = 4);   // hub coupler
}

module cutter_motor_and_spool() {
    color("DimGray") translate([0, 0, motor_face]) cylinder(d = cm_d, h = cm_len);
    color("Silver") translate([0, 0, spool_top]) cylinder(d = 12, h = motor_face - spool_top);
    color("LimeGreen") translate([0, 0, spool_bot]) cylinder(d = spool_d, h = spool_h);
    color("Yellow") for (a = [0, 180]) rotate(a) translate([spool_d / 2 - 2, 0, cut_height])
        rotate([0, 90, 0]) cylinder(d = 1.5, h = cut_d / 2 - spool_d / 2 + 2, $fn = 8);
    if (SHOW_CUT_DISC)
        color([1, 0.2, 0.2, 0.18]) translate([0, 0, cut_height - 0.25]) cylinder(d = cut_d, h = 0.5, $fn = 96);
}

// NUCLEO-N657X0-Q + X-NUCLEO-67W61M1. Board frame: x across (0..70),
// y from the USB/RJ45 edge (0) to the ST-LINK edge (133.35), z = 0 at
// the PCB underside. Connector spots read off UM3417 fig. 7 (approx).
module nucleo_n6_stack() {
    color("WhiteSmoke") difference() {
        cube([nuc_w, nuc_l, 1.6]);
        for (h = nuc_holes) translate([h[0], h[1], -1]) cylinder(d = 3.2, h = 4, $fn = 16);
    }
    color("Black") for (x = [1.5, nuc_w - 6.5]) translate([x, 54, 1.6]) cube([5, 49, 8.5]);   // ST morpho
    color("Black") for (x = [8.3, 57.8]) translate([x, 53, 1.6]) cube([2.6, 49, 8.5]);        // ARDUINO
    color("Silver") translate([42.9 - 4.5, nuc_l - 6, 1.6]) cube([9, 8.4, 3.3]);              // CN10 ST-LINK USB-C
    color("Silver") translate([27.4 - 4.5, -5.5, 1.6]) cube([9, 8.5, 3.3]);                   // CN8 user USB-C (power in)
    color("Silver") translate([47.2 - 8, -4.4, 1.6]) cube([16, 21, 13.5]);                    // CN11 RJ45
    color("Ivory")  translate([21 - 8, 29.4 - 2.5, 1.6]) cube([16, 5, 2.2]);                  // CN6 camera
    translate([8.3, 40, 1.6 + 11]) {                                                           // Wi-Fi shield [EST]
        color("SteelBlue") cube([53.3, 68.6, 1.6]);
        color("Silver") translate([18, 40, 1.6]) cube([15, 22, 3]);
    }
}

// X-STM32MP-RBT01, centred, z = 0 at the PCB underside. [EST] layout.
module rbt01() {
    color("DarkGreen") difference() {
        translate([-rbt_size[0] / 2, -rbt_size[1] / 2, 0]) cube([rbt_size[0], rbt_size[1], 1.6]);
        for (p = corners([0, 0], rbt_size, rbt_inset)) translate([p[0], p[1], -1]) cylinder(d = 3.2, h = 4, $fn = 16);
    }
    color("LimeGreen") for (x = [-30, 10]) translate([x, -rbt_size[1] / 2 + 2, 1.6]) cube([20, 10, 10]);
    color("LimeGreen") translate([-rbt_size[0] / 2 + 2, -6, 1.6]) cube([12, 12, 10]);
    color("Black") translate([-26, rbt_size[1] / 2 - 9, 1.6]) cube([52, 5, 8.5]);
}

module cell_pack(lx, ly, cell_d, cell_l, col) {
    color("Black") translate([-lx / 2, -ly / 2, 0]) cube([lx, ly, 3]);
    color(col) for (i = [0 : 3]) translate([-lx / 2 + lx / 8 + i * lx / 4, 0, 3 + cell_d / 2])
        rotate([90, 0, 0]) cylinder(d = cell_d, h = cell_l, center = true);
}

module module_box(sz, h) {
    translate([-sz[0] / 2, -sz[1] / 2, 0]) cube([sz[0], sz[1], h]);
}

module wall_ports() {
    translate([port_charge[0], port_charge[1], port_charge[2]]) rotate(port_charge[3])
        color("Black") rotate([0, 90, 0]) translate([0, 0, -15]) cylinder(d = 11.5, h = 18);
    translate([port_switch[0], port_switch[1], port_switch[2]]) rotate(port_switch[3])
        color("Red") rotate([0, 90, 0]) translate([0, 0, -18]) cylinder(d = 15.5, h = 22);
    translate([port_usb[0], port_usb[1], port_usb[2]]) rotate(port_usb[3])
        color("Silver") translate([-15, -16, -4]) cube([17, 32, 8]);
}

// In the ToF holder frame.
module tof_board() {
    color("Purple") translate([-3.1, -tof_pcb[0] / 2, tof_z_off - tof_pcb[1] / 2]) cube([1.6, tof_pcb[0], tof_pcb[1]]);
    color("Black") translate([-1.5, -3.2, tof_z_off - 1.5]) cube([1.2, 6.4, 3]);
}

// In the camera pod frame.
module camera_board() {
    fx = head_o[0] / 2 - 2.4;   // inside face of the front wall
    translate(head_c) rotate([0, cam_tilt, 0]) {
        if (CAMERA == "USB_UVC") color("Black") translate([fx - 25, -19, -19]) cube([25, 38, 38]);
        else {
            color("ForestGreen") translate([fx - 6.6, -12.5, -14.5]) cube([1.6, 25, 24]);
            color("Black") translate([fx - 5, -4.25, -4.25]) cube([5, 8.5, 8.5]);
        }
    }
}

// Origin on the lid roof.
module estop() {
    color("DimGray") translate([0, 0, -lid_t - 30]) cylinder(d = 28, h = 30);
    color("Yellow") cylinder(d = 34, h = 5);
    color("Red") translate([0, 0, 5]) cylinder(d = 40, h = 12);
}
