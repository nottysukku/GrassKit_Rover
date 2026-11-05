// =====================================================================
//  GrassKit Rover v2 - printed parts. Included by grasskit_rover.scad,
//  which holds every setting. Open that file, not this one.
//  Suggested material: PETG or ASA, 0.4 mm nozzle, 5 walls, 20 % gyroid.
// =====================================================================

vent_hole_d = 36;
vent_tube   = [36, 46, 12];    // inner, outer diameter, height above the flange
vent_flange = [64, 3];
vent_leg_h  = 6;
vent_cap_r  = 34;
vent_cap_z  = vent_flange[1] + vent_tube[2] + vent_leg_h + 2.4;   // cap roof top above the lid
wall_boss_len = 7;

module body_outline(d = 0) {
    offset(r = d) offset(r = body_r) square([body_L - 2 * body_r, body_W - 2 * body_r], center = true);
}

module half_space(s) {
    translate([s > 0 ? 0 : -1000, -1000, -10]) cube([1000, 2000, 1000]);
}

// Bolts through the joint ribs at x = 0. Nut traps on the rear half.
module rib_bolt_holes(list, s) {
    for (p = list) translate([0, p[0], p[1]]) {
        rotate([0, 90, 0]) cylinder(d = m3, h = 4 * rib_t, center = true, $fn = 16);
        if (s < 0) translate([-rib_t - 0.01, 0, 0]) rotate([0, 90, 0]) cylinder(d = 6.4, h = 2.6, $fn = 6);
    }
}

// =====================================================================
//  Tub: floor + walls. Print floor-down. Two halves, joined at x = 0.
// =====================================================================
module tub_shell() {
    difference() {
        hull() {
            translate([0, 0, deck_z]) linear_extrude(0.01) body_outline(-bot_chamfer);
            translate([0, 0, deck_z + bot_chamfer]) linear_extrude(tub_h - bot_chamfer) body_outline(0);
        }
        translate([0, 0, floor_top]) linear_extrude(tub_h) body_outline(-tub_wall);
    }
}

// Insert boss on the inside of a wall, with a 45 degree gusset under it.
module wall_boss(b) {
    translate([b[0], b[1], b[2]]) rotate(b[3]) hull() {
        translate([-tub_wall - wall_boss_len, 0, 0]) rotate([0, 90, 0]) cylinder(d = 9, h = wall_boss_len + tub_wall / 2, $fn = 24);
        translate([-tub_wall - 0.5, -4.5, -wall_boss_len - 4.5]) cube([0.5 + tub_wall / 2, 9, 0.5]);
    }
}

// Heat-set insert goes in from outside the wall.
module wall_boss_hole(b) {
    translate([b[0], b[1], b[2]]) rotate(b[3]) translate([1, 0, 0]) rotate([0, -90, 0])
        cylinder(d = insert_d, h = 1 + tub_wall + 5.5, $fn = 16);
}

// ToF window: aperture plus a recess inside for a 1 mm clear cover.
module wall_window(p) {
    translate([p[0], p[1], p[2]]) rotate(p[3]) {
        translate([-10, -tof_win[0] / 2, -tof_win[1] / 2]) cube([20, tof_win[0], tof_win[1]]);
        translate([-tub_wall - 1, -tof_win[0] / 2 - 3, -tof_win[1] / 2 - 3]) cube([1 + tub_wall - 0.8, tof_win[0] + 6, tof_win[1] + 6]);
    }
}

module wall_round(p, d) {
    translate([p[0], p[1], p[2]]) rotate(p[3]) rotate([0, 90, 0]) cylinder(d = d, h = 20, center = true);
}

module wall_port_holes() {
    wall_round(port_charge, 12);
    wall_round(port_switch, 16);
    translate([port_usb[0], port_usb[1], port_usb[2]]) rotate(port_usb[3]) {
        cube([20, 13, 7], center = true);
        for (s = [-1, 1]) translate([0, s * 12.5, 0]) rotate([0, 90, 0]) cylinder(d = 3.2, h = 20, center = true, $fn = 16);
    }
}

module floor_boss_hole(b) {
    if (b[3] == 1) translate([b[0], b[1], deck_z - 0.01]) cylinder(d = insert_d, h = insert_depth + 0.5, $fn = 16);
    else translate([b[0], b[1], floor_top + b[2] - insert_depth]) cylinder(d = insert_d, h = insert_depth + 1, $fn = 16);
}

module tub_full() {
    difference() {
        union() {
            tub_shell();
            translate([0, 0, floor_top - 0.01]) difference() {           // dam around the spool hole
                cylinder(r = dam_ro, h = dam_h, $fn = 96);
                translate([0, 0, -1]) cylinder(d = cut_hole_d, h = dam_h + 2, $fn = 96);
            }
            for (b = floor_bosses) translate([b[0], b[1], floor_top - 0.01]) cylinder(d = b[4], h = b[2] + 0.01, $fn = 24);
            for (b = wall_bosses) wall_boss(b);
        }
        translate([0, 0, deck_z - 1]) cylinder(d = cut_hole_d, h = floor_t + 2, $fn = 96);
        for (b = floor_bosses) floor_boss_hole(b);
        for (p = wire_holes) translate([p[0], p[1], deck_z - 1]) cylinder(d = 8, h = floor_t + 2, $fn = 24);  // under the pods
        for (b = wall_bosses) wall_boss_hole(b);
        for (t = tof_windows) wall_window(t);
        wall_port_holes();
    }
}

// U-shaped rib along the floor and walls at the joint.
module tub_rib(s) {
    difference() {
        intersection() {
            translate([s > 0 ? 0 : -rib_t, -body_W, floor_top - 0.01]) cube([rib_t, 2 * body_W, tub_h - floor_t - 1]);
            translate([0, 0, floor_top - 0.02]) linear_extrude(tub_h) body_outline(-tub_wall + 0.01);
        }
        translate([0, 0, floor_top + rib_w]) linear_extrude(tub_h) body_outline(-tub_wall - rib_w);
        translate([0, 0, floor_top - 1]) cylinder(r = 66, h = tub_h);   // clear of the dam and tower
    }
}

module tub_half(s) {   // s = 1 front, -1 rear
    difference() {
        union() {
            intersection() { tub_full(); half_space(s); }
            tub_rib(s);
        }
        rib_bolt_holes(tub_rib_bolts, s);
    }
}

// =====================================================================
//  Lid. Print roof-down (chamfers print as 45 degree slopes).
// =====================================================================
module lid_outer() {
    hull() {
        translate([0, 0, lid_bot]) linear_extrude(lid_top - lid_chamfer - lid_bot) body_outline(lid_off);
        translate([0, 0, lid_top - 0.01]) linear_extrude(0.01) body_outline(lid_off - lid_chamfer);
    }
}

module lid_cavity(e = 0) {
    hull() {
        translate([0, 0, lid_bot - 1]) linear_extrude(lid_in_top - e - (lid_chamfer - 1) - lid_bot + 1) body_outline(lid_clear - e);
        translate([0, 0, lid_in_top - e - 0.01]) linear_extrude(0.01) body_outline(lid_clear - e - (lid_chamfer - 1));
    }
}

// Sloped ledge that rests on the tub rim.
module lid_lip() {
    difference() {
        translate([0, 0, tub_top]) linear_extrude(lip_h) body_outline(lid_clear + 0.01);
        hull() {
            translate([0, 0, tub_top - 0.01]) linear_extrude(0.01) body_outline(lid_clear - lip_w);
            translate([0, 0, tub_top + lip_h]) linear_extrude(0.01) body_outline(lid_clear + 0.02);
        }
    }
}

module lid_full() {
    difference() {
        union() {
            difference() { lid_outer(); lid_cavity(0); }
            lid_lip();
            for (b = lid_bosses) translate([b[0], b[1], lid_in_top - b[3]]) cylinder(d = b[2], h = b[3] + 0.5, $fn = 32);
        }
        translate([estop_pos[0], estop_pos[1], lid_bot]) cylinder(d = 22.5, h = 100);          // 22 mm E-stop
        for (v = vent_pos) translate([v[0], v[1], lid_bot]) cylinder(d = vent_hole_d, h = 100, $fn = 64);
        translate([cam_x - 4, -10, lid_bot]) cube([8, 20, 100]);                                // camera ribbon slot
        for (p = lid_inserts) translate([p[0], p[1], lid_top - insert_depth - 0.5]) cylinder(d = insert_d, h = insert_depth + 1, $fn = 16);
        for (b = lid_screws) translate([b[0], b[1], b[2]]) rotate(b[3]) rotate([0, 90, 0])
            cylinder(d = m3, h = 2 * lid_off + 6, center = true, $fn = 16);
    }
}

module lid_rib(s) {
    difference() {
        intersection() {
            translate([s > 0 ? 0 : -rib_t, -body_W, tub_top + lip_h + 0.5]) cube([rib_t, 2 * body_W, lid_top]);
            lid_cavity(0);
        }
        lid_cavity(rib_w);
    }
}

module lid_half(s) {
    difference() {
        union() {
            intersection() { lid_full(); half_space(s); }
            lid_rib(s);
        }
        rib_bolt_holes(lid_rib_bolts, s);
    }
}

// =====================================================================
//  Motor pod: closed housing for one drive gearmotor, under the floor.
//  Local frame: motor axis along +Y (outward) through the origin,
//  floor underside at z = pod_top. Part 0 = top half, 1 = bottom half.
//  4 x M3 x 40 go up through both halves into the floor inserts.
// =====================================================================
module motor_pod_part(part) {
    difference() {
        if (part == 0) translate([-pod_w / 2, -pod_len / 2, split_gap / 2]) cube([pod_w, pod_len, pod_top - split_gap / 2]);
        else translate([-pod_w / 2, -pod_len / 2, -pod_bot]) cube([pod_w, pod_len, pod_bot - split_gap / 2]);
        translate([0, -pod_len / 2 + pod_rear, 0]) rotate([-90, 0, 0]) cylinder(d = dm_d + 0.3, h = pod_len, $fn = 64);
        translate([-3, -pod_len / 2 + pod_rear, 0]) cube([6, pod_wire_dy + pod_len / 2 - pod_rear + 4, dm_d / 2 + 2.5]);  // wire groove
        translate([0, pod_wire_dy, 0]) cylinder(d = 8, h = 50, $fn = 24);                                                  // up into the tub
        for (sx = [-1, 1], sy = [-1, 1]) translate([sx * pod_screw[0], sy * pod_screw[1], -50]) cylinder(d = m3, h = 100, $fn = 16);
        if (part == 1) for (sx = [-1, 1], sy = [-1, 1])
            translate([sx * pod_screw[0], sy * pod_screw[1], -pod_bot - 0.01]) cylinder(d = 6.5, h = 3.5, $fn = 24);
    }
}

// =====================================================================
//  Cutter tower: hat over the spool dam, holds the cutter motor
//  shaft-down. z = 0 is the floor top. Lugs sit on the floor bosses.
// =====================================================================
module cutter_tower() {
    wall_ri = dam_ro + 1.6;
    wall_ro = wall_ri + 2.4;
    disc_r  = wall_ro + 0.5;
    col_ri  = cm_d / 2 + 0.25;
    col_ro  = col_ri + 4.5;
    top     = post_h + tower_t;
    lug_z   = 6;
    difference() {
        union() {
            cylinder(r = wall_ro, h = post_h, $fn = 96);
            translate([0, 0, post_h]) cylinder(r = disc_r, h = tower_t, $fn = 96);
            for (a = [45 : 90 : 315]) hull() {
                rotate(a) translate([tower_bolt_r, 0, lug_z]) cylinder(r = 6, h = 5);
                rotate(a) translate([wall_ri, 0, lug_z]) cylinder(r = 8, h = 5);
            }
            translate([0, 0, top]) cylinder(r = col_ro, h = collar_h, $fn = 96);
            translate([col_ri - 1, -7, top]) cube([col_ro - col_ri + 10, 14, collar_h]);   // clamp ears
        }
        translate([0, 0, -1]) cylinder(r = wall_ri, h = post_h + 1, $fn = 96);
        translate([0, 0, top]) cylinder(r = col_ri, h = collar_h + 1, $fn = 96);
        translate([col_ri - 2, -1, top]) cube([col_ro - col_ri + 14, 2, collar_h + 1]);
        translate([col_ro + 5, 0, top + collar_h / 2]) rotate([90, 0, 0]) cylinder(d = m3, h = 30, center = true, $fn = 16);
        translate([0, 0, post_h - 1]) cylinder(d = cm_boss + 1, h = tower_t + 2);
        if (cm_hole_sp > 0)
            for (s = [-1, 1]) translate([s * cm_hole_sp / 2, 0, post_h - 1]) cylinder(d = m4, h = tower_t + 2, $fn = 16);
        for (p = tower_lugs) translate([p[0], p[1], -1]) cylinder(d = m3, h = 20, $fn = 16);
    }
}

// Ring around the line, screwed up into the floor inserts.
// half = 1 keeps the +Y half (print two).
module guard_skirt(half = 0) {
    h = deck_z - skirt_bot;
    difference() {
        union() {
            translate([0, 0, skirt_bot]) difference() {
                cylinder(r = guard_r_out, h = h, $fn = 160);
                translate([0, 0, -1]) cylinder(r = guard_r_in, h = h + 2, $fn = 160);
            }
            translate([0, 0, deck_z - 3]) difference() {
                cylinder(r = guard_r_in + 0.1, h = 3, $fn = 160);
                translate([0, 0, -1]) cylinder(r = guard_r_in - skirt_flange, h = 5, $fn = 160);
            }
        }
        for (p = skirt_holes) translate([p[0], p[1], deck_z - 5]) cylinder(d = m3, h = 10, $fn = 16);
        if (half == 1) translate([-200, -400, 0]) cube([400, 400, 200]);
    }
}

// =====================================================================
//  Wheel fender. Local frame: wheel axis along +Y, origin on the outer
//  tub wall at axle height. The web screws to the wall.
// =====================================================================
module wheel_fender() {
    zc = deck_z - axle_z;
    difference() {
        union() {
            intersection() {
                rotate([-90, 0, 0]) difference() {
                    cylinder(r = fender_ro, h = fender_wy, $fn = 96);
                    translate([0, 0, -1]) cylinder(r = fender_ri, h = fender_wy + 2, $fn = 96);
                }
                translate([-fender_ro - 1, -1, zc]) cube([2 * fender_ro + 2, fender_wy + 2, fender_ro]);
            }
            intersection() {
                rotate([-90, 0, 0]) cylinder(r = fender_ro, h = 2.4, $fn = 96);
                translate([-fender_ro - 1, -1, zc]) cube([2 * fender_ro + 2, 4, fender_ro]);
            }
        }
        for (s = [-1, 1]) translate([s * fender_dx, -1, fender_screw_z]) rotate([-90, 0, 0]) cylinder(d = m3, h = 5, $fn = 16);
    }
}

// =====================================================================
//  ToF holder. Local frame: +X outward, origin on the inner wall face
//  at floor-top height. Holds the board 1.5 mm behind the window.
// =====================================================================
module tof_holder() {
    bz = 5;                    // floor boss height
    zc = tof_z_off;
    px = -1.5 - 1.6 - 2;       // upright front face (2 mm board standoffs)
    difference() {
        union() {
            translate([-26, -14, bz]) cube([26 + px, 28, 3]);
            translate([px - 3, -14, bz]) cube([3, 28, zc + tof_pcb[1] / 2 + 2 - bz]);
            for (s = [-1, 1]) hull() {
                translate([px - 15, s * 12.5 - 1.5, bz]) cube([12, 3, 3]);
                translate([px - 4, s * 12.5 - 1.5, bz]) cube([1, 3, 20]);
            }
            for (h = tof_holes) translate([px - 0.01, h[0], zc + h[1]]) rotate([0, 90, 0]) cylinder(d = 5, h = 2.01, $fn = 16);
        }
        for (b = tof_boss_xy) translate([b[0], b[1], bz - 1]) cylinder(d = m3, h = 5, $fn = 16);
        for (h = tof_holes) translate([px - 5, h[0], zc + h[1]]) rotate([0, 90, 0]) cylinder(d = 1.8, h = 10, $fn = 12);
        translate([px - 5, -6, zc - tof_pcb[1] / 2 - 9]) cube([10, 12, 5]);    // cable slot
    }
}

// =====================================================================
//  Camera pod. Local frame: origin on the lid roof, +X forward.
//  The ribbon runs down the hollow mast and through the lid slot.
//  Camera board: M2 screws through the front wall, 5 mm spacers.
// =====================================================================
module camera_pod() {
    W  = head_o[1];
    mt = 4 + cam_mast_h + head_o[2] * 0.6;
    difference() {
        union() {
            translate([-22, -W / 2, 0]) cube([44, W, 4]);
            translate([-8, -W / 2, 0]) cube([16, W, mt]);
            translate(head_c) rotate([0, cam_tilt, 0]) translate(-head_o / 2) cube(head_o);
        }
        translate([-4, -10, -1]) cube([8, 20, 4 + cam_mast_h + 9]);
        translate(head_c) rotate([0, cam_tilt, 0]) {
            translate([-head_o[0] / 2 + 3, -cam_in[1] / 2, -head_o[2] / 2 + 3]) cube([cam_in[0], cam_in[1], cam_in[2] + 1]);
            translate([head_o[0] / 2 - 3, 0, 0]) rotate([0, 90, 0]) cylinder(d = 14, h = 5);          // lens
            translate([head_o[0] / 2 - 1, 0, 0]) rotate([0, 90, 0]) cylinder(d = 24, h = 2);          // 1 mm clear cover
            if (CAMERA == "MIPI_CSI")
                for (hy = [-10.5, 10.5], hz = [0, -12.5])                                              // [EST] Pi camera holes
                    translate([head_o[0] / 2 - 4, hy, hz]) rotate([0, 90, 0]) cylinder(d = 2.2, h = 6, $fn = 12);
            for (s = [-1, 1]) translate([0, s * (cam_in[1] / 2 + 1.5), head_o[2] / 2 - 6]) cylinder(d = 1.8, h = 7, $fn = 12);
        }
        for (h = cam_foot_holes) translate([h[0], h[1], -1]) cylinder(d = m3, h = 6, $fn = 16);
    }
}

// Head-frame cover (print plate-down). Lip fits the open top of the head.
module camera_pod_cover() {
    difference() {
        union() {
            translate([-head_o[0] / 2, -head_o[1] / 2, 0]) cube([head_o[0], head_o[1], 2]);
            translate([-head_o[0] / 2 + 3.2, -cam_in[1] / 2 + 0.2, 2]) cube([cam_in[0] - 0.4, cam_in[1] - 0.4, 3]);
        }
        for (s = [-1, 1]) translate([0, s * (cam_in[1] / 2 + 1.5), -1]) cylinder(d = 2.2, h = 4, $fn = 12);
    }
}

module camera_pod_cover_placed() {
    translate(head_c) rotate([0, cam_tilt, 0]) translate([0, 0, head_o[2] / 2 + 2]) mirror([0, 0, 1]) camera_pod_cover();
}

// =====================================================================
//  Vent: base screws onto the lid over a 36 mm hole, cap sits on
//  3 pins 6 mm above it. Rain hits the cap, air leaves under the skirt.
// =====================================================================
module vent_base() {
    difference() {
        union() {
            cylinder(d = vent_flange[0], h = vent_flange[1], $fn = 64);
            cylinder(d = vent_tube[1], h = vent_flange[1] + vent_tube[2], $fn = 64);
        }
        translate([0, 0, -1]) cylinder(d = vent_tube[0], h = 50, $fn = 64);
        for (a = [90, 210, 330]) rotate(a) translate([26, 0, 0]) {
            translate([0, 0, -1]) cylinder(d = m3, h = 10, $fn = 16);
            translate([0, 0, vent_flange[1] - 1.6]) cylinder(d1 = m3, d2 = 6.6, h = 1.61, $fn = 16);
        }
        for (a = [30, 150, 270]) rotate(a) translate([20, 0, vent_flange[1] + vent_tube[2] - 4]) cylinder(d = 3.1, h = 5, $fn = 12);
    }
}

module vent_cap() {   // print orientation: roof on the bed
    cylinder(r = vent_cap_r, h = 2.4, $fn = 96);
    difference() {
        cylinder(r = vent_cap_r, h = 2.4 + 10, $fn = 96);
        translate([0, 0, -1]) cylinder(r = vent_cap_r - 2.4, h = 20, $fn = 96);
    }
    for (a = [30, 150, 270]) rotate(a) translate([20, 0, 0]) {
        cylinder(d = 7, h = 2.4 + vent_leg_h, $fn = 24);
        cylinder(d = 2.8, h = 2.4 + vent_leg_h + 3.5, $fn = 12);
    }
}

// =====================================================================
//  Battery cradle for a 4 x 18650 holder, on 4 floor bosses.
//  One hook-and-loop strap runs through the two slots, over the cells.
// =====================================================================
module battery_cradle() {
    wall = 2;
    base = 3;
    h = base + 14;
    difference() {
        translate([-cradle[0] / 2, -cradle[1] / 2, 0]) cube([cradle[0], cradle[1], h]);
        translate([-cradle[0] / 2 + wall, -cradle[1] / 2 + wall, base]) cube([cradle[0] - 2 * wall, cradle[1] - 2 * wall, h]);
        for (p = corners([0, 0], cradle, 6)) translate([p[0], p[1], 0]) {
            translate([0, 0, -1]) cylinder(d = m3, h = 10, $fn = 16);
            translate([0, 0, base - 1.6]) cylinder(d1 = m3, d2 = 6.6, h = 1.61, $fn = 16);
        }
        for (s = [-1, 1]) translate([s * (cradle[0] / 2 - wall - 3), 0, base / 2]) cube([3, 24, base + 2], center = true);
        translate([-cradle[0] / 2 - 1, -8, base + 4]) cube([wall + 2, 16, h]);    // wire exit, rear
    }
}

// Plate for the power module or the cutter driver, on 4 floor bosses.
// The 2.5 mm grid takes M3 self-tappers or zip ties.
module mount_plate(sz) {
    difference() {
        translate([-sz[0] / 2, -sz[1] / 2, 0]) cube([sz[0], sz[1], 3]);
        for (p = corners([0, 0], sz, 5)) translate([p[0], p[1], 0]) {
            translate([0, 0, -1]) cylinder(d = m3, h = 5, $fn = 16);
            translate([0, 0, 1.4]) cylinder(d1 = m3, d2 = 6.6, h = 1.61, $fn = 16);
        }
        for (x = [-sz[0] / 2 + 12 : 8 : sz[0] / 2 - 12], y = [-sz[1] / 2 + 9 : 8 : sz[1] / 2 - 9])
            translate([x, y, -1]) cylinder(d = 2.5, h = 5, $fn = 12);
    }
}
