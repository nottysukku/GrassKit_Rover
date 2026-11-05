// =====================================================================
//  GrassKit Rover v3 - stand-ins for bought parts (visual only).
//  Included by grasskit_rover.scad.
// =====================================================================

// 130 x 60 treaded wheel, axis along Y, centred.
module treaded_wheel() {
    color("DimGray") rotate([90, 0, 0]) cylinder(d = wheel_d - 8, h = wheel_w, center = true, $fn = 64);
    color("Black") for (i = [0 : 17]) rotate([0, i * 20, 0]) for (s = [-1, 1])
        translate([wheel_d / 2 - 5, s * wheel_w / 4, 0]) rotate([0, 0, s * 12]) cube([10, wheel_w / 2 - 2, 9], center = true);
    color("Gold") rotate([90, 0, 0]) cylinder(d = 40, h = wheel_w + 1, center = true);
}

// 36 mm planetary gearmotor with encoder. Gearbox face at z = 0, body
// towards -z, spigot, shaft and hex coupling towards +z.
module gearmotor36() {
    color("Gold")   translate([0, 0, -dm_gb_len]) cylinder(d = dm_d, h = dm_gb_len, $fn = 48);
    color("Silver") translate([0, 0, -dm_gb_len - dm_mot_len]) cylinder(d = dm_d - 0.5, h = dm_mot_len, $fn = 48);
    color("Black")  translate([0, 0, -dm_len]) cylinder(d = dm_d - 1, h = dm_enc_len, $fn = 48);
    color("Gold")   cylinder(d = dm_spigot[0], h = dm_spigot[1], $fn = 32);
    color("Silver") translate([0, 0, dm_spigot[1]]) cylinder(d = dm_shaft[0], h = dm_shaft[1], $fn = 24);
    color("Goldenrod") translate([0, 0, motor_pad + 0.5]) cylinder(d = 12 / cos(30), h = coupling_len, $fn = 6);
}

// Cutter in the cutter frame (x = cut_x already applied).
module cutter_motor_and_spool() {
    color("DimGray") translate([0, 0, cm_base - 6]) cylinder(d = cm_d - 3, h = 6, $fn = 48);            // stator base, X-mount
    color("DarkRed") translate([0, 0, bell_bot]) cylinder(d = cm_d, h = cm_len - 6, $fn = 48);          // spinning bell
    color("Silver")  translate([0, 0, spool_top]) cylinder(d = 16, h = adapter_len, $fn = 32);          // aluminium hub
    color("LimeGreen") translate([0, 0, spool_bot]) cylinder(d = spool_d, h = spool_h, $fn = 64);
    color("Yellow") for (a = [0, 180]) rotate(a) translate([spool_d / 2 - 2, 0, cut_height])
        rotate([0, 90, 0]) cylinder(d = 1.6, h = cut_d / 2 - spool_d / 2 + 2, $fn = 8);
    if (SHOW_CUT_DISC)
        color([1, 0.2, 0.2, 0.18]) translate([0, 0, cut_height - 0.25]) cylinder(d = cut_d, h = 0.5, $fn = 96);
}

// NUCLEO-N657X0-Q. Board frame: x across (0..70), y from the USB/RJ45
// edge (0) to the ST-LINK edge (133.35), z = 0 at the PCB underside.
// Connector spots read off UM3417 fig. 7 (approx).
module nucleo_n6() {
    color("WhiteSmoke") difference() {
        cube([nuc_w, nuc_l, 1.6]);
        for (h = nuc_holes) translate([h[0], h[1], -1]) cylinder(d = 3.2, h = 4, $fn = 16);
    }
    color("Black") for (x = [1.5, nuc_w - 6.5]) translate([x, 54, 1.6]) cube([5, 49, 8.5]);   // ST morpho
    color("Black") for (x = [8.3, 57.8]) translate([x, 53, 1.6]) cube([2.6, 49, 8.5]);        // ARDUINO
    color("Silver") translate([42.9 - 4.5, nuc_l - 6, 1.6]) cube([9, 8.4, 3.3]);              // CN10 ST-LINK USB-C
    color("Silver") translate([27.4 - 4.5, -5.5, 1.6]) cube([9, 8.5, 3.3]);                   // CN8 user USB-C (5 V in)
    color("Silver") translate([47.2 - 8, -4.4, 1.6]) cube([16, 21, 13.5]);                    // CN11 RJ45
    color("Ivory")  translate([21 - 8, 29.4 - 2.5, 1.6]) cube([16, 5, 2.2]);                  // CN6 camera
    if (INCLUDE_GNSS) translate([8.3, 40, 1.6 + 11]) {                                         // X-NUCLEO-GNSS1A1 [EST]
        color("MidnightBlue") cube([53.3, 68.6, 1.6]);
        color("Silver") translate([15, 30, 1.6]) cube([16, 16, 6]);
    }
}

// NUCLEO-G474RE with the X-NUCLEO-67W61M1 on its Arduino header.
// Board frame: x across (0..70), y to the ST-LINK end (82.5).
module nucleo64_g474() {
    color("WhiteSmoke") difference() {
        cube([g4_size[0], g4_size[1], 1.6]);
        for (h = g4_holes) translate([h[0], h[1], -1]) cylinder(d = 3.2, h = 4, $fn = 16);
    }
    color("Black") for (x = [1.5, g4_size[0] - 6.5]) translate([x, 4, 1.6]) cube([5, 48, 8.5]);    // morpho
    color("Silver") translate([30, g4_size[1] - 6, 1.6]) cube([8, 7, 3]);                          // ST-LINK USB
    translate([8.3, 2, 1.6 + 11]) {                                                                 // Wi-Fi shield [EST]
        color("SteelBlue") cube([53.3, 55, 1.6]);
        color("Silver") translate([18, 30, 1.6]) cube([15, 22, 3]);
    }
}

// STM32MP257F-DK. Board frame per UM3385 fig. 6: x along the 100 mm side,
// y from the connector edge (0). Connector spots approx.
module mp2_dk() {
    color("DarkSlateBlue") difference() {
        cube([mp2_size[0], mp2_size[1], 1.6]);
        for (h = mp2_holes) translate([h[0], h[1], -1]) cylinder(d = 3.5, h = 4, $fn = 16);
    }
    color("Silver") translate([33, -2, 1.6]) cube([16, 21, 13.5]);                // RJ45
    color("Silver") translate([60, -2, 1.6]) cube([14, 17, 15]);                  // USB-A
    color("Silver") for (x = [19, 81]) translate([x, -1, 1.6]) cube([9, 8, 3.3]); // USB-C (CN21 = 5 V in)
    color("Black")  translate([93, 8, 1.6]) cube([5.2, 51, 8.5]);                 // 40-pin GPIO
    color("DimGray") translate([55, 33, 1.6]) cube([16, 16, 8]);                  // SoC heatsink [EST]
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

// ESC, heatsink and fan in the esc_mount frame.
module esc_block() {
    translate([esc_cx, 0, 3 + 3]) {
        color("Purple") translate([-esc_pcb[1] / 2, -esc_pcb[0] / 2, 0]) cube([esc_pcb[1], esc_pcb[0], 1.6]);
        color("Silver") for (i = [0 : 6]) translate([-esc_pcb[1] / 2 + 2 + i * 6, -esc_pcb[0] / 2 + 2, 1.6]) cube([1.2, esc_pcb[0] - 4, 10]);
    }
    color("Black") translate([-esc_plate[0] / 2 + 4, -15, 3]) difference() {
        cube([10, 30, 30]);
        translate([-1, 15, 15]) rotate([0, 90, 0]) cylinder(d = 26, h = 12);
    }
}

// 4S4P pack, z = 0 on the cradle base. 80 mm side along X.
module battery_pack() {
    color("RoyalBlue") translate([-bat_size[1] / 2, -bat_size[0] / 2, 0]) cube([bat_size[1], bat_size[0], bat_size[2]]);
    color("Yellow") translate([bat_size[1] / 2 - 2, -6, bat_size[2] - 20]) cube([14, 12, 8]);   // XT90-S
}

module module_box(sz, h) {
    translate([-sz[0] / 2, -sz[1] / 2, 0]) cube([sz[0], sz[1], h]);
}

// Plate A: 30 A relay, INA226, 3 inline fuse holders, star-ground bus.
module power_parts_a() {
    color("SteelBlue") translate([-18, 10, 0]) module_box([50, 40], 30);       // relay module [EST]
    color("Purple")    translate([25, 20, 0]) module_box([20, 25], 5);         // INA226
    color("Red") for (i = [0 : 2]) translate([-30 + i * 22, -24, 6]) rotate([0, 90, 0]) cylinder(d = 11, h = 18, center = true);
    color("Gold")      translate([28, -10, 0]) module_box([12, 40], 10);       // ground bus
}

// Plate B: Pololu D36V50F5, USB-C buck for the MP2, XT60 branches, capacitor.
module power_parts_b() {
    color("ForestGreen") translate([-25, 15, 0]) module_box([25.4, 25.4], 9.5);
    color("DarkBlue")    translate([15, 15, 0]) module_box([45, 25], 12);       // [EST]
    color("Yellow") for (i = [0 : 2]) translate([-30 + i * 20, -20, 0]) module_box([16, 8], 16);
    color("Black")       translate([30, -18, 0]) cylinder(d = 12.5, h = 25);
}

module wall_ports() {
    translate([port_switch[0], port_switch[1], port_switch[2]]) rotate(port_switch[3]) {
        color("Black") translate([-20, -11, -15]) cube([22, 22, 30]);
        color("Red") translate([0, -9, -13]) cube([4, 18, 26]);
    }
    translate([port_charge[0], port_charge[1], port_charge[2]]) rotate(port_charge[3])
        color("Yellow") translate([-15, -8, -4.25]) cube([18, 16, 8.5]);
    translate([port_usb[0], port_usb[1], port_usb[2]]) rotate(port_usb[3])
        color("Silver") translate([-15, -16, -4]) cube([17, 32, 8]);
}

// In the ToF holder frame.
module tof_board(t) {
    pcb = tof_pcbs[t];
    color(t == 1 ? "Indigo" : "Purple") translate([-3.1, -pcb[0] / 2, tof_z_off - pcb[1] / 2]) cube([1.6, pcb[0], pcb[1]]);
    color("Black") translate([-1.5, -3.2, tof_z_off - 1.5]) cube([1.2, 6.4, 3]);
}

// B-CAMS-IMX in the camera pod frame.
module camera_board() {
    translate(head_c) rotate([0, cam_tilt, 0]) {
        color("RoyalBlue") translate([cam_bx - 1.6, -cam_pcb[0] / 2, cam_bz - cam_pcb[1] / 2]) cube([1.6, cam_pcb[0], cam_pcb[1]]);
        color("Black") translate([cam_bx, 0, cam_bz + cam_lens_z]) rotate([0, 90, 0]) cylinder(d = 14, h = cam_lens_len - 1, $fn = 32);
        color("Black") translate([cam_bx, -8, cam_bz + cam_lens_z - 8]) cube([6, 16, 16]);
        color("Black") translate([cam_bx, -2, cam_bz + cam_tof_z - 1.5]) cube([1.2, 4, 3]);
    }
}

// LD19, z = 0 on its base, scan centre at the origin.
module ld19() {
    color("Black") linear_extrude(2.5) hull() {
        square(38.6, center = true);
        for (h = ld19_holes) translate(h) circle(d = 7);
    }
    color("Black") translate([-19.3, -19.3, 0]) cube([38.6, 38.6, 34.8 - 12.6]);
    color([0.15, 0.15, 0.15]) translate([0, 0, 34.8 - 12.6]) cylinder(d = 35.3, h = 12.6, $fn = 48);
    color("Red") translate([0, 0, ld19_plane - 0.25]) cylinder(d = 36, h = 0.5, $fn = 48);
    color("White") translate([-3, -19.8, 3]) cube([6, 1, 4]);    // connector
}

// Origin on the lid roof.
module estop() {
    color("DimGray") translate([0, 0, -lid_t - 30]) cylinder(d = 28, h = 30);
    color("Yellow") cylinder(d = 34, h = 5);
    color("Red") translate([0, 0, 5]) cylinder(d = 40, h = 12);
}

// FC-37 rain board on 2 short standoffs, origin on the lid roof.
module rain_sensor() {
    color("White") for (s = [-1, 1]) translate([s * 22, 0, 0]) cylinder(d = 5, h = 4, $fn = 16);
    color("SteelBlue") translate([-27, -20, 4]) cube([54, 40, 1.6]);
}

// KW12 roller micro-switch in the bumper switch bracket frame.
module micro_switch_placed() {
    zp = bump_z - floor_top - 4;
    color("Black") translate([-12, -3.2, zp - 10]) cube([10, 6.4, 20]);
    color("Silver") translate([-2, -0.5, zp + 7]) rotate([0, 160, 0]) cube([1, 1, 15]);
    color("Silver") translate([2, 0, zp]) rotate([90, 0, 0]) cylinder(d = 4.8, h = 3, center = true, $fn = 16);
}
