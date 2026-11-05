// =====================================================================
//  GrassKit Rover v3 - printed parts. Included by grasskit_rover.scad,
//  which holds every setting. Open that file, not this one.
//  Suggested material: PETG or ASA, 0.4 mm nozzle, 5 walls, 20 % gyroid.
//  Print the bumper halves in TPU 95A if you want them to give a little.
// =====================================================================

vent_hole_d = 36;
vent_tube   = [36, 46, 12];    // inner, outer diameter, height above the flange
vent_flange = [64, 3];
vent_leg_h  = 6;
vent_cap_r  = 34;
vent_cap_z  = vent_flange[1] + vent_tube[2] + vent_leg_h + 2.4;   // cap roof top above the lid
wall_boss_len = 7;
sleeve_len  = 12;                                                  // bumper pin sleeve inside the wall
pin_len     = bump_gap + tub_wall + sleeve_len + 1.5;

module body_outline(d = 0) {
    offset(r = d) offset(r = body_r) square([body_L - 2 * body_r, body_W - 2 * body_r], center = true);
}

// One quarter of space: sx = 1 front / -1 rear, sy = 1 left / -1 right.
module quadrant(sx, sy) {
    translate([sx > 0 ? 0 : -1000, sy > 0 ? 0 : -1000, -10]) cube([1000, 1000, 1000]);
}

// Bolts along X through the x = 0 joint ribs: list of [y, z]. Nut traps on the rear quarters.
module xrib_bolt_holes(list, sx) {
    for (p = list) translate([0, p[0], p[1]]) {
        rotate([0, 90, 0]) cylinder(d = m3, h = 4 * rib_t, center = true, $fn = 16);
        if (sx < 0) translate([-rib_t - 0.01, 0, 0]) rotate([0, 90, 0]) cylinder(d = 6.4, h = 2.6, $fn = 6);
    }
}

// Bolts along Y through the y = 0 joint ribs: list of [x, z]. Nut traps on the right quarters.
module yrib_bolt_holes(list, sy) {
    for (p = list) translate([p[0], 0, p[1]]) {
        rotate([90, 0, 0]) cylinder(d = m3, h = 4 * rib_t, center = true, $fn = 16);
        if (sy < 0) translate([0, -rib_t - 0.01, 0]) rotate([-90, 0, 0]) cylinder(d = 6.4, h = 2.6, $fn = 6);
    }
}

// =====================================================================
//  Tub: floor + walls. Print floor-down. Four quarters.
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

// Thick disc on the inner side wall where a gearbox face bolts on.
module motor_pad(m) {
    y0 = body_W / 2 - motor_pad;
    y1 = body_W / 2 - tub_wall / 2;
    intersection() {
        translate([m[0], m[1] * (y0 + y1) / 2, axle_z]) rotate([90, 0, 0])
            cylinder(d = dm_d + 12, h = y1 - y0, center = true, $fn = 64);
        translate([-1000, -1000, floor_top - 0.01]) cube([2000, 2000, 1000]);
    }
}

// Spigot hole and countersunk face screws (heads outside, under the coupling).
module motor_pad_holes(m) {
    mirror([0, m[1] < 0 ? 1 : 0, 0]) translate([m[0], body_W / 2, axle_z]) {
        rotate([90, 0, 0]) cylinder(d = dm_spigot[0] + 1, h = 2 * motor_pad + 2, center = true, $fn = 48);
        for (k = [0 : dm_face_n - 1]) let (a = 45 + k * 360 / dm_face_n)
            translate([dm_face_pcd / 2 * cos(a), 0, dm_face_pcd / 2 * sin(a)]) {
                rotate([90, 0, 0]) cylinder(d = m3, h = 2 * motor_pad + 2, center = true, $fn = 16);
                translate([0, 0.01, 0]) rotate([90, 0, 0]) cylinder(d1 = 6.6, d2 = m3, h = 1.7, $fn = 16);
            }
    }
}

// The lower edge of the motor drops through this slot into the tray.
module motor_slot(m) {
    y0 = dm_end_y - 2;
    y1 = dm_face_y;
    translate([m[0] - slot_hw, m[1] > 0 ? y0 : -y1, deck_z - 1]) cube([2 * slot_hw, y1 - y0, floor_t + 2]);
}

// Corner column up to the rim: magnet pocket (type 0) or A3144 Hall pocket (type 1).
module tub_column(c) {
    translate([c[0], c[1], floor_top - 0.01]) cylinder(d = 12, h = tub_top - floor_top + 0.01, $fn = 32);
}

module tub_column_cut(c) {
    translate([c[0], c[1], tub_top]) {
        if (c[2] == 0) translate([0, 0, -3.2]) cylinder(d = 6.3, h = 4, $fn = 24);   // 6 x 3 magnet, glued
        else {
            translate([-2.6, -2.3, -1.8]) cube([5.2, 4.6, 2]);                       // A3144 lying face-up
            translate([0, 0, -(tub_top - floor_top - 6)]) cylinder(d = 3.2, h = tub_top - floor_top - 6 + 0.01, $fn = 12);
        }
    }
    if (c[2] == 1) translate([c[0], c[1], floor_top + 8]) rotate(atan2(-c[1], -c[0])) rotate([0, 90, 0])
        cylinder(d = 3.2, h = 12, $fn = 12);                                           // wires out, towards the middle
}

// Pedestal with a sleeve for one bumper pin. b = [x, y on the outer wall, z, angle].
module bump_sleeve(b) {
    translate([b[0], b[1], b[2]]) rotate(b[3]) hull() {
        translate([-tub_wall - sleeve_len, 0, 0]) rotate([0, 90, 0]) cylinder(d = 16, h = sleeve_len + tub_wall / 2, $fn = 32);
        translate([-tub_wall - sleeve_len, -8, floor_top - b[2] - 0.01]) cube([sleeve_len + tub_wall / 2, 16, 1]);
    }
}

module bump_sleeve_hole(b) {
    translate([b[0], b[1], b[2]]) rotate(b[3]) rotate([0, 90, 0]) cylinder(d = bump_pin_d + 0.6, h = 50, center = true, $fn = 24);
}

// ToF window: aperture plus a recess inside for a 1 mm clear cover.
// p = [x, y on the outer wall, z, angle, width, height]
module wall_window(p) {
    translate([p[0], p[1], p[2]]) rotate(p[3]) {
        translate([-10, -p[4] / 2, -p[5] / 2]) cube([20, p[4], p[5]]);
        translate([-tub_wall - 1, -p[4] / 2 - 3, -p[5] / 2 - 3]) cube([1 + tub_wall - 0.8, p[4] + 6, p[5] + 6]);
    }
}

module wall_port_holes() {
    translate([port_switch[0], port_switch[1], port_switch[2]]) rotate(port_switch[3]) cube([20, 22, 30], center = true);
    translate([port_charge[0], port_charge[1], port_charge[2]]) rotate(port_charge[3]) {
        cube([20, 16, 8.5], center = true);
        for (s = [-1, 1]) translate([0, s * 12.5, 0]) rotate([0, 90, 0]) cylinder(d = 3.2, h = 20, center = true, $fn = 16);
    }
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
            translate([cut_x, 0, floor_top - 0.01]) difference() {             // dam around the spool hole
                cylinder(r = dam_ro, h = dam_h, $fn = 96);
                translate([0, 0, -1]) cylinder(d = cut_hole_d, h = dam_h + 2, $fn = 96);
            }
            for (b = floor_bosses) translate([b[0], b[1], floor_top - 0.01]) cylinder(d = b[4], h = b[2] + 0.01, $fn = 24);
            for (m = motor_spots) motor_pad(m);
            for (c = lid_columns) tub_column(c);
            for (b = bump_sleeves) bump_sleeve(b);
        }
        translate([cut_x, 0, deck_z - 1]) cylinder(d = cut_hole_d, h = floor_t + 2, $fn = 96);
        for (b = floor_bosses) floor_boss_hole(b);
        for (m = motor_spots) { motor_slot(m); motor_pad_holes(m); }
        for (p = concat(tray_holes, tower_lugs)) translate([p[0], p[1], deck_z - 1]) cylinder(d = m3, h = floor_t + 2, $fn = 16);
        for (c = lid_columns) tub_column_cut(c);
        for (b = bump_sleeves) bump_sleeve_hole(b);
        for (t = tof_windows) wall_window(t);
        wall_port_holes();
    }
}

// U-shaped rib along the floor and walls at the x = 0 joint, on the sx side.
module tub_xrib(sx) {
    difference() {
        intersection() {
            translate([sx > 0 ? 0 : -rib_t, -body_W, floor_top - 0.01]) cube([rib_t, 2 * body_W, tub_h - floor_t - 1]);
            translate([0, 0, floor_top - 0.02]) linear_extrude(tub_h) body_outline(-tub_wall + 0.01);
        }
        translate([0, 0, floor_top + rib_w]) linear_extrude(tub_h) body_outline(-tub_wall - rib_w);
        translate([cut_x, 0, floor_top - 1]) cylinder(r = drum_ro + 3.6, h = tub_h, $fn = 96);   // clear of the tower
    }
}

// Same at the y = 0 joint. The battery cradle splices the floor where the rib stops.
module tub_yrib(sy) {
    difference() {
        intersection() {
            translate([-body_L, sy > 0 ? 0 : -rib_t, floor_top - 0.01]) cube([2 * body_L, rib_t, tub_h - floor_t - 1]);
            translate([0, 0, floor_top - 0.02]) linear_extrude(tub_h) body_outline(-tub_wall + 0.01);
        }
        translate([0, 0, floor_top + rib_w]) linear_extrude(tub_h) body_outline(-tub_wall - rib_w);
        translate([cut_x, 0, floor_top - 1]) cylinder(r = drum_ro + 3.6, h = tub_h, $fn = 96);
        translate([bat_x0 - 2, -20, floor_top - 1]) cube([bat_x1 - bat_x0 + 4, 40, tub_h]);
    }
}

module tub_quarter(sx, sy) {
    difference() {
        union() {
            intersection() { tub_full(); quadrant(sx, sy); }
            intersection() { union() { tub_xrib(sx); tub_yrib(sy); } quadrant(sx, sy); }
        }
        xrib_bolt_holes(tub_xrib_bolts, sx);
        yrib_bolt_holes(tub_yrib_bolts, sy);
    }
}

// =====================================================================
//  Lid = floating canopy. Print roof-down. Four quarters.
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

// Block inside the lid skirt for a fender screw. b = [x, y on the outer lid face, z, angle].
module lid_wall_boss(b) {
    translate([b[0], b[1], 0]) rotate(b[3])
        translate([-lid_off - wall_boss_len, -5, b[2] - 4.5]) cube([wall_boss_len + lid_wall / 2, 10, lid_in_top - b[2] + 5]);
}

module lid_wall_boss_hole(b) {
    translate([b[0], b[1], b[2]]) rotate(b[3]) translate([1, 0, 0]) rotate([0, -90, 0])
        cylinder(d = insert_d, h = 1 + lid_wall + 5.5, $fn = 16);
}

module lid_solid() {
    difference() { lid_outer(); lid_cavity(0); }
    lid_lip();
    intersection() {                       // keep the bosses inside the chamfered roof
        union() {
            for (b = lid_bosses) translate([b[0], b[1], lid_in_top - b[3]]) cylinder(d = b[2], h = b[3] + 0.5, $fn = 32);
            for (c = lid_columns) translate([c[0], c[1], tub_top + 0.5]) cylinder(d = 12, h = lid_in_top - tub_top, $fn = 32);
            for (b = lid_wall_bosses) lid_wall_boss(b);
        }
        lid_outer();
    }
}

module lid_holes() {
    translate([estop_pos[0], estop_pos[1], lid_bot]) cylinder(d = 22.5, h = 100);                  // 22 mm E-stop
    for (v = vent_pos) translate([v[0], v[1], lid_bot]) cylinder(d = vent_hole_d, h = 100, $fn = 64);
    translate([cam_x - 4, -10, lid_in_top - 15]) cube([8, 20, 100]);                                // camera FFC slot
    translate([lidar_pos[0], lidar_pos[1], lid_in_top - 15]) cylinder(d = 14, h = 100);             // lidar cable
    if (INCLUDE_RAIN_SENSOR) translate([rain_pos[0], rain_pos[1], lid_in_top - 15]) cylinder(d = 8, h = 100);
    for (p = lid_inserts) translate([p[0], p[1], lid_top - insert_depth - 0.5]) cylinder(d = insert_d, h = insert_depth + 1, $fn = 16);
    for (c = lid_columns) translate([c[0], c[1], tub_top + 0.5 - 0.01]) cylinder(d = 6.3, h = 3.2, $fn = 24);   // magnets
    for (b = lid_wall_bosses) lid_wall_boss_hole(b);
}

module lid_xrib(sx) {
    difference() {
        intersection() {
            translate([sx > 0 ? 0 : -rib_t, -body_W, tub_top + lip_h + 0.5]) cube([rib_t, 2 * body_W, lid_top]);
            lid_cavity(0);
        }
        lid_cavity(rib_w);
    }
}

module lid_yrib(sy) {
    difference() {
        intersection() {
            translate([-body_L, sy > 0 ? 0 : -rib_t, tub_top + lip_h + 0.5]) cube([2 * body_L, rib_t, lid_top]);
            lid_cavity(0);
        }
        lid_cavity(rib_w);
        translate([bat_x0 - 3, -10, 0]) cube([bat_x1 - bat_x0 + 6, 20, lid_in_top - 3]);   // shallow over the battery
    }
}

module lid_quarter(sx, sy) {
    difference() {
        union() {
            intersection() { lid_solid(); quadrant(sx, sy); }
            intersection() { union() { lid_xrib(sx); lid_yrib(sy); } quadrant(sx, sy); }
        }
        lid_holes();
        xrib_bolt_holes(lid_xrib_bolts, sx);
        yrib_bolt_holes(lid_yrib_bolts, sy);
    }
}

// =====================================================================
//  Motor tray: closes the floor slot under one gearmotor.
//  Local frame: X across the motor (0 = motor axis), Y along the motor
//  (outward = +Y, the same as world |y|), Z = world height.
//  M3 screws come down through the floor (and the clamp) into its inserts.
// =====================================================================
module motor_tray() {
    difference() {
        translate([-tray_hw, tray_y0, tray_bot]) cube([2 * tray_hw, tray_y1 - tray_y0, deck_z - tray_bot]);
        translate([0, tray_y0 + 3, axle_z]) rotate([-90, 0, 0]) cylinder(r = dm_d / 2 + 0.4, h = 500, $fn = 72);
        for (sx = [-1, 1], y = tray_screw_y) translate([sx * tray_screw_x, y, deck_z - insert_depth - 0.5])
            cylinder(d = insert_d, h = insert_depth + 1, $fn = 16);
    }
}

// Strap over the motor inside the tub, 16 mm wide, centred on y = 0 locally.
// Its feet share the middle tray screws (M3 x 20).
module motor_clamp() {
    w = 16;
    fw = slot_hw + 13;
    difference() {
        union() {
            translate([0, 0, axle_z]) rotate([90, 0, 0]) cylinder(r = dm_d / 2 + 4, h = w, center = true, $fn = 72);
            translate([-fw, -w / 2, floor_top]) cube([2 * fw, w, 5]);
        }
        translate([-50, -w, floor_top - 50]) cube([100, 2 * w, 50]);      // nothing below the floor top
        translate([0, 0, axle_z]) rotate([90, 0, 0]) cylinder(r = dm_d / 2 + 0.3, h = w + 2, center = true, $fn = 72);
        for (sx = [-1, 1]) translate([sx * tray_screw_x, 0, floor_top - 1]) cylinder(d = m3, h = 10, $fn = 16);
    }
}

// =====================================================================
//  Cutter tower: drum over the spool dam, 45 degree cone, column round
//  the D3536 bell. The motor hangs shaft-down from the top plate
//  (X-mount, 4 x M3). z = 0 is the floor top. M3 screws come up through
//  the floor into the inserts in the 4 lugs.
// =====================================================================
module cutter_tower() {
    hd = drum_top - floor_top;
    hc = cone_top - floor_top;
    hp = cm_base - floor_top;
    ht = tower_top - floor_top;
    lug_h = 12;
    difference() {
        union() {
            cylinder(r = drum_ro, h = hd, $fn = 96);
            translate([0, 0, hd - 0.01]) cylinder(r1 = drum_ro, r2 = col_ro, h = hc - hd + 0.01, $fn = 96);
            translate([0, 0, hc - 0.02]) cylinder(r = col_ro, h = ht - hc + 0.02, $fn = 64);
            for (a = tower_lug_angles) hull() {
                rotate(a) translate([tower_bolt_r, 0, 0]) cylinder(r = 6, h = lug_h, $fn = 24);
                rotate(a) translate([drum_ri + 1, 0, 0]) cylinder(r = 8, h = lug_h, $fn = 24);
            }
        }
        translate([0, 0, -1]) cylinder(r = drum_ri, h = hd, $fn = 96);
        translate([0, 0, hd - 1.01]) cylinder(r1 = drum_ri, r2 = col_ri, h = hc - hd + 0.02, $fn = 96);
        translate([0, 0, hc - 1.02]) cylinder(r = col_ri, h = hp - hc + 1.02, $fn = 64);
        translate([0, 0, hp - 1]) cylinder(d = 10, h = 10);                                          // shaft end
        for (k = [0 : 3]) rotate(45 + 90 * k) translate([cm_mount[k % 2] / 2, 0, hp - 1]) cylinder(d = m3, h = 10, $fn = 16);
        for (k = [0 : 5]) rotate(30 + 60 * k) translate([col_ri - 1, -3, hc + 2]) cube([6, 6, max(2, hp - hc - 5)]);   // vents
        for (a = tower_lug_angles) rotate(a) translate([tower_bolt_r, 0, -0.01]) cylinder(d = insert_d, h = insert_depth + 0.5, $fn = 16);
    }
}

// Ring round the line, screwed up into the floor inserts.
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
        for (k = [0 : 7]) rotate(22.5 + 45 * k) translate([skirt_hole_r, 0, deck_z - 5]) cylinder(d = m3, h = 10, $fn = 16);
        if (half == 1) translate([-200, -400, 0]) cube([400, 400, 200]);
    }
}

// =====================================================================
//  Wheel fender, carried by the lid. Local frame: wheel axis along +Y,
//  origin on the outer lid face at axle height. The web screws to the lid
//  skirt (countersunk M3); the arc stays clear of the tub.
// =====================================================================
module wheel_fender() {
    ri = wheel_d / 2 + 7;
    ro = ri + 2.4;
    y0 = 1;
    y1 = wheel_gap + wheel_w + 5 - lid_off;
    zw = lid_bot - axle_z;
    difference() {
        union() {
            intersection() {
                translate([0, y0, 0]) rotate([-90, 0, 0]) difference() {
                    cylinder(r = ro, h = y1 - y0, $fn = 120);
                    translate([0, 0, -1]) cylinder(r = ri, h = y1 - y0 + 2, $fn = 120);
                }
                translate([-ro - 1, -1, ri * sin(30)]) cube([2 * ro + 2, y1 + 2, ro]);    // 30 to 150 degrees
            }
            intersection() {
                translate([-32, 0, zw]) cube([64, 2.4, ro]);
                rotate([-90, 0, 0]) cylinder(r = ro, h = 2.4, $fn = 120);
            }
        }
        for (s = [-1, 1]) translate([s * 20, 0, fender_z - axle_z]) {
            translate([0, -1, 0]) rotate([-90, 0, 0]) cylinder(d = m3, h = 5, $fn = 16);
            translate([0, 2.41, 0]) rotate([90, 0, 0]) cylinder(d1 = 6.6, d2 = m3, h = 1.7, $fn = 16);
        }
    }
}

// =====================================================================
//  Board decks: plates on 4 M3 x 35 hex standoffs above the motors.
//  z = 0 is the deck underside. Boards sit on 6 mm insert bosses.
// =====================================================================
module board_deck(xr, bosses, legs) {
    w = xr[1] - xr[0];
    difference() {
        union() {
            translate([xr[0], -deck_hw, 0]) cube([w, 2 * deck_hw, 3]);
            difference() {                                     // stiffening lip round the edge
                translate([xr[0], -deck_hw, 0]) cube([w, 2 * deck_hw, 7]);
                translate([xr[0] + 2.4, -deck_hw + 2.4, 1]) cube([w - 4.8, 2 * deck_hw - 4.8, 10]);
            }
            for (p = bosses) translate([p[0], p[1], 3 - 0.01]) cylinder(d = 7, h = board_standoff + 0.01, $fn = 24);
            for (l = legs) translate([l[0], l[1], 0]) cylinder(d = 10, h = 5, $fn = 24);
        }
        for (p = bosses) translate([p[0], p[1], 3 + board_standoff - insert_depth]) cylinder(d = insert_d, h = insert_depth + 1, $fn = 16);
        for (l = legs) translate([l[0], l[1], -1]) cylinder(d = m3, h = 10, $fn = 16);
        for (y = [-deck_hw / 2, 0, deck_hw / 2]) translate([(xr[0] + xr[1]) / 2 - 12, y - 4, -1]) cube([24, 8, 5]);   // wire slots
    }
}

module deck_front() { board_deck(deck_front, concat(g4_bosses, n6_bosses), deck_legs_f); }
module deck_rear()  { board_deck(deck_rear, concat(mp2_bosses, rbt_bosses), deck_legs_r); }

// =====================================================================
//  Battery cradle for the 4S4P pack, on 4 floor bosses. It also splices
//  the floor across the y = 0 joint. One hook-and-loop strap runs through
//  the two base slots and over the pack.
// =====================================================================
module battery_cradle() {
    base = 3;
    wall = cradle_wall;
    h = base + 22;
    difference() {
        translate([-cradle[0] / 2, -cradle[1] / 2, 0]) cube([cradle[0], cradle[1], h]);
        translate([-cradle[0] / 2 + wall, -cradle[1] / 2 + wall, base]) cube([cradle[0] - 2 * wall, cradle[1] - 2 * wall, h]);
        for (p = corners([0, 0], cradle, 6)) translate([p[0], p[1], 0]) {
            translate([0, 0, -1]) cylinder(d = m3, h = 10, $fn = 16);
            translate([0, 0, base - 1.6]) cylinder(d1 = m3, d2 = 6.6, h = 1.61, $fn = 16);
        }
        for (s = [-1, 1]) translate([0, s * (cradle[1] / 2 - wall - 3), base / 2]) cube([30, 3, base + 2], center = true);
        translate([cradle[0] / 2 - wall - 1, -10, base + 6]) cube([wall + 2, 20, h]);   // XT90 lead exit, front
    }
}

// Plate on 4 floor bosses. The 2.5 mm grid takes M3 self-tappers or zip ties.
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

// Plate with a pocket for the B-G431B-ESC1 (41 mm along X) and a frame for
// a 30 mm fan that blows along the board and its heatsink.
esc_cx = 6;
module esc_mount() {
    pcb = [esc_pcb[1], esc_pcb[0]];
    mount_plate(esc_plate);
    translate([esc_cx, 0, 3 - 0.01]) difference() {
        translate([-pcb[0] / 2 - 2, -pcb[1] / 2 - 2, 0]) cube([pcb[0] + 4, pcb[1] + 4, 7]);
        translate([-pcb[0] / 2 - 0.3, -pcb[1] / 2 - 0.3, 3]) cube([pcb[0] + 0.6, pcb[1] + 0.6, 10]);   // board rests on a ledge
        translate([-pcb[0] / 2 + 2, -pcb[1] / 2 + 2, -1]) cube([pcb[0] - 4, pcb[1] - 4, 10]);          // open: air and wires
        for (s = [-1, 1]) translate([-6, s * (pcb[1] / 2 + 1) - 3, -1]) cube([12, 6, 10]);              // zip-tie gaps
    }
    translate([-esc_plate[0] / 2 + 1, 0, 3 - 0.01]) difference() {
        translate([0, -16, 0]) cube([3, 32, 32]);
        translate([-1, 0, 16]) rotate([0, 90, 0]) cylinder(d = 28, h = 5);
        for (sy = [-1, 1], sz = [-1, 1]) translate([-1, sy * 12, 16 + sz * 12]) rotate([0, 90, 0]) cylinder(d = 2.8, h = 5, $fn = 12);
    }
}

// =====================================================================
//  ToF holder. Local frame: +X outward, origin on the inner wall face at
//  floor-top height. Holds the board 1.5 mm behind the window.
//  t = 0: SATEL-VL53L8 breakout, t = 1: STEVAL-VL53L9.
// =====================================================================
module tof_holder(t) {
    pcb = tof_pcbs[t];
    bz = 5;                      // floor boss height
    zc = tof_z_off;
    px = -1.5 - 1.6 - 2;         // upright front face (2 mm board standoffs)
    hw = pcb[0] / 2 + 3;
    difference() {
        union() {
            translate([-tof_depth, -hw, bz]) cube([tof_depth + px, 2 * hw, 3]);
            translate([px - 3, -hw, bz]) cube([3, 2 * hw, zc + pcb[1] / 2 + 2 - bz]);
            for (s = [-1, 1]) hull() {
                translate([-tof_depth + 10, s * (hw - 1.5) - 1.5, bz]) cube([4, 3, 3]);
                translate([px - 4, s * (hw - 1.5) - 1.5, bz]) cube([1, 3, zc - bz]);
            }
            for (h = tof_holes[t]) translate([px - 0.01, h[0], zc + h[1]]) rotate([0, 90, 0]) cylinder(d = 5, h = 2.01, $fn = 16);
        }
        for (b = tof_boss_xy(t)) translate([b[0], b[1], bz - 1]) cylinder(d = m3, h = 5, $fn = 16);
        for (h = tof_holes[t]) translate([px - 5, h[0], zc + h[1]]) rotate([0, 90, 0]) cylinder(d = 1.8, h = 10, $fn = 12);
        translate([px - 5, -6, bz + 3.5]) cube([10, 12, 5]);    // cable slot
    }
}

// =====================================================================
//  Bumper half. Local frame: back face at x = 0 (+X outward), the bar
//  runs from the centre line (y = 0) to y = bump_reach, z = 0 on the pin
//  axis. The pins slide through the wall sleeves; an M3 self-tapper and a
//  washer on each pin end keep it in. A spring over each pin pushes it out.
// =====================================================================
module bumper_half() {
    difference() {
        union() {
            hull() for (y = [7, bump_reach - 6], z = [-bump_h / 2 + 6, bump_h / 2 - 6])
                translate([0, y, z]) rotate([0, 90, 0]) cylinder(r = 6, h = bump_t, $fn = 24);
            for (p = bump_pins) translate([0.01, p, 0]) rotate([0, -90, 0]) cylinder(d = bump_pin_d, h = pin_len, $fn = 24);
        }
        for (p = bump_pins) translate([-pin_len - 0.01, p, 0]) rotate([0, 90, 0]) cylinder(d = 2.5, h = 10, $fn = 12);
    }
}

// Holds a KW12 roller micro-switch upright behind the outer pin.
// Local frame: origin on the floor-boss tops, +X towards the wall, y = 0
// on the pin line; the sleeve end is at x = 4.
module bumper_switch_bracket() {
    zp = bump_z - floor_top - 4;      // pin axis height in this frame
    difference() {
        union() {
            translate([-22, -13, 0]) cube([22, 13 - 3.2, 4]);
            translate([-14, -7.2, 0]) cube([14, 4, zp + 13]);
        }
        for (dx = [-14, -4]) translate([dx, -9, -1]) {
            cylinder(d = m3, h = 10, $fn = 16);
            translate([0, 0, 2.4]) cylinder(d1 = m3, d2 = 6.6, h = 1.61, $fn = 16);
        }
        for (dz = [-4.75, 4.75]) translate([-7, -10, zp + dz]) rotate([-90, 0, 0]) cylinder(d = 2.2, h = 10, $fn = 12);   // M2 through the switch
    }
}

// =====================================================================
//  Camera pod for the B-CAMS-IMX. Local frame: origin on the lid roof,
//  +X forward. The FFC runs down the hollow mast and through the lid slot.
//  The board hangs on 4 posts behind the front wall (M2 self-tappers).
// =====================================================================
cam_fx = head_o[0] / 2 - 2.4;          // inside face of the front wall
cam_bx = cam_fx - cam_lens_len + 1;    // board front face
cam_bz = 1.5;                          // board centre height in the head frame

module camera_pod() {
    W  = head_o[1];
    mt = 4 + cam_mast_h + head_o[2] * 0.6;
    posts = [for (hy = [-1, 1], hz = [-1, 1]) [hy * (cam_pcb[0] / 2 - 3), cam_bz + hz * (cam_pcb[1] / 2 - 3)]];
    difference() {
        union() {
            difference() {                 // the cavity is cut from the mast too, so the board fits
                union() {
                    translate([-22, -W / 2, 0]) cube([44, W, 4]);
                    translate([-8, -W / 2, 0]) cube([16, W, mt]);
                    translate(head_c) rotate([0, cam_tilt, 0]) translate(-head_o / 2) cube(head_o);
                }
                translate(head_c) rotate([0, cam_tilt, 0])
                    translate([-head_o[0] / 2 + 3, -cam_in[1] / 2, -head_o[2] / 2 + 3]) cube([cam_in[0], cam_in[1], cam_in[2] + 1]);
            }
            translate(head_c) rotate([0, cam_tilt, 0])
                for (p = posts) translate([cam_bx, p[0], p[1]]) rotate([0, 90, 0]) cylinder(d = 5, h = cam_fx - cam_bx + 1, $fn = 16);   // 1 mm into the front wall
        }
        translate([-4, -10, -1]) cube([8, 20, 4 + cam_mast_h + 14]);    // FFC slot, opens into the head
        translate(head_c) rotate([0, cam_tilt, 0]) {
            translate([cam_fx - 1, 0, cam_bz + cam_lens_z]) rotate([0, 90, 0]) cylinder(d = 16, h = 5);          // lens
            translate([head_o[0] / 2 - 1, 0, cam_bz + cam_lens_z]) rotate([0, 90, 0]) cylinder(d = 26, h = 2);   // clear cover
            translate([cam_fx - 1, -5, cam_bz + cam_tof_z - 3.5]) cube([5, 10, 7]);                                // ToF window
            translate([head_o[0] / 2 - 1, -8, cam_bz + cam_tof_z - 6]) cube([2, 16, 12]);
            for (p = posts) translate([cam_bx - 1, p[0], p[1]]) rotate([0, 90, 0]) cylinder(d = 1.8, h = cam_fx - cam_bx + 1, $fn = 12);
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
//  Lidar mast: foot on the lid (rubber grommets under it if the lid
//  buzzes), hollow tube, top plate shaped like the LD19 base with its
//  3 ears. The lead drops through the window under the plate.
// =====================================================================
module lidar_mast() {
    tp = 4;
    difference() {
        union() {
            translate([-25, -25, 0]) cube([50, 50, 4]);
            cylinder(d = 30, h = lidar_mast_h, $fn = 48);
            for (a = [0 : 90 : 270]) rotate(a) hull() {
                translate([14, -1.5, 0]) cube([10, 3, 4]);
                translate([14, -1.5, 0]) cube([1, 3, 20]);
            }
            translate([0, 0, lidar_mast_h - 0.01]) rotate(lidar_ang) linear_extrude(tp) hull() {
                square(38.6, center = true);
                for (h = ld19_holes) translate(h) circle(d = 7);
            }
        }
        translate([0, 0, -1]) cylinder(d = 22, h = lidar_mast_h + 0.5, $fn = 40);
        translate([-20, -6, lidar_mast_h - 14]) cube([10, 12, 13]);
        translate([0, 0, lidar_mast_h]) rotate(lidar_ang) for (h = ld19_holes) translate([h[0], h[1], -1]) cylinder(d = 2.2, h = tp + 2, $fn = 12);
        for (p = lidar_foot_holes) translate([p[0], p[1], -1]) cylinder(d = m3, h = 10, $fn = 16);
    }
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
