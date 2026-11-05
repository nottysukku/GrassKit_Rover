// =====================================================================
//  GrassKit Rover - chassis, cutter mount and full component layout
//  OpenSCAD 2021.01 or newer.   Units: mm and grams.
//
//  Frame: origin on the ground under the middle of the rover.
//         +X = forward, +Y = left, +Z = up.
//
//  MODE = "assembly" -> the whole rover with every component, plus a
//                       weight / centre of gravity / wheel load / torque /
//                       battery report in the OpenSCAD console.
//  Any other MODE    -> one part on its own, laid flat for printing.
//                       Press F6, then File > Export > STL
//                       (the *_2d modes export DXF/SVG for laser or CNC).
//
//  [ST]  value taken from the ST documents in documentation/
//  [EST] estimate: weigh or measure your part and change the number.
//
//  ---------------------------------------------------------------------
//  POWER: one 4S 18650 pack can run everything
//  ---------------------------------------------------------------------
//   4S pack 12.0-16.8 V -- main switch -- E-stop --+-- 20 A fuse -- cutter driver -- cutter motor
//                                                  +-- 10 A fuse -- RBT01 VS (5.2-48 V) -- 4 wheel motors
//                                                  +--  3 A fuse -- 5 V / 3 A buck -- Nucleo E5V (JP5 on E5V)
//                                                                    +-- USB camera, ToF boards
//   * Level shifters convert 3.3 V / 5 V logic signals, not power.
//     Power needs buck (step-down) converters, as above.
//   * Never wire the pack to Nucleo VIN: VIN is 7-12 V [ST UM1724] and a
//     full 4S pack is 16.8 V.
//   * The AA pack is not needed. Set INCLUDE_AA_PACK = false to drop it.
//   * The GIGAWATTS 250 W trimmer is a corded 230 V AC motor. It cannot
//     run from these batteries. Keep its spool head and 1 mm line and spin
//     them with a 12-18 V DC motor (CUT_MOTOR = "775_DC").
//   * 12 V wheel motors on a 16.8 V pack: cap the PWM duty at about 70 %.
//   * Put a 1000 uF low-ESR capacitor at the cutter driver input and
//     soft-start the cutter so its inrush does not brown out the Nucleo.
// =====================================================================

/* [Render] */
MODE = "assembly"; // [assembly, deck_2d, deck_front_half, deck_rear_half, deck_splice, canopy_2d, motor_bracket, cutter_tower, guard_skirt_half, tof_bracket, camera_mast, nucleo_tray, rbt01_tray]
CUT_MOTOR = "775_DC"; // [775_DC, TRIMMER_AC_250W]
DECK_MATERIAL = "PETG_PRINT"; // [PETG_PRINT, PLYWOOD_6, ALU_3]
SHOW_CANOPY = true;
SHOW_CUT_DISC = true;     // translucent disc = area the trimmer line sweeps
INCLUDE_AA_PACK = true;   // the 4 x AA pack from the parts list (not needed)
INCLUDE_SBC = false;      // Linux board to host the USB camera (see notes)

/* [Wheels and drive] */
wheel_d   = 75;    // mecanum wheel diameter
wheel_w   = 35;    // mecanum wheel width
wheel_g   = 75;    // [EST] grams per wheel (plastic hub, rubber rollers)
wheelbase = 240;   // front axle to rear axle (X)
wheel_gap = 5;     // deck side edge to wheel inner face
dm_d      = 25;    // drive gearmotor diameter (JGA25-370 class) [EST]
dm_len    = 55;    // gearbox + motor length [EST]
dm_shaft  = 4;     // output shaft diameter
dm_g      = 95;    // [EST] grams per gearmotor
dm_wall   = 3;     // clamp wall above and below the motor

/* [Deck] */
deck_L     = 340;
deck_W     = 240;
deck_r     = 14;   // corner radius
grid_pitch = 20;   // M3 mounting grid in the front and rear bays
grid_x0    = 70;
grid_x1    = 150;
grid_y     = 80;

/* [Cutter] */
cut_height   = 40;   // trimmer line height above the ground
cut_d        = 200;  // line tip-to-tip diameter after trimming the line
spool_d      = 80;   // [EST] bump spool diameter - measure yours
spool_h      = 40;   // [EST] bump spool height
spool_line_z = 12;   // [EST] line eyelet height above the spool bottom
spool_g      = 110;  // [EST] spool, cap, spring and 1 mm line
coupler_len  = 15;   // tower plate underside to spool top (shaft adapter)
guard_clear  = 10;   // line tips to skirt wall
guard_wall   = 2.5;
guard_drop   = 10;   // skirt ends this far below the line plane
collar_h     = 20;   // motor clamp collar height

/* [Boards] */
nuc_w = 70;          // [ST] UM1724 fig. 5
nuc_l = 82.5;        // [ST] UM1724 fig. 5
// [ST] read off UM1724 fig. 5 (+-0.5 mm), board frame: x across the 70 mm
// side, y along the 82.5 mm side, ST-LINK end at y = 82.5. Check with calipers.
nuc_holes = [[10.9, 54.4], [59.1, 53.6], [43.9, 3.0]];
nuc_standoff = 12;   // clears the morpho pins under the board
rbt_w = 56;          // [EST] measure your X-STM32MP-RBT01
rbt_l = 85;          // [EST]
rbt_holes = [[3.5, 3.5], [52.5, 3.5], [3.5, 81.5], [52.5, 81.5]];  // [EST]
rbt_standoff = 8;

/* [Layout on the deck] */
nuc_pos = [105, 50];     // Nucleo + Wi-Fi shield, front-left bay
rbt_pos = [105, -55];    // RBT01 motor board, front-right bay
bat_pos = [-105, 58];    // 4 x 18650 holder, rear-left
aa_pos  = [-105, -60];   // 4 x AA holder, rear-right
pwr_pos = [-118, -103];  // BMS + 5 V buck + fuses
esc_pos = [-105, -5];    // cutter motor driver
canopy_gap = 115;        // deck top to canopy underside
mast_h   = 110;          // camera mast height above the canopy
cam_tilt = 25;           // camera looks down by this angle
tof_tilt = 8;            // ToF sensors look down by this angle

/* [Power and performance budget] */
cell_mAh = 2500;   // per 18650 cell
cells_s  = 4;      // 4 cells in series
cell_V   = 3.6;    // nominal
P_cut    = 100;    // [EST] W while cutting with 1 mm line
P_drive  = 15;     // [EST] W, 4 wheels on grass at mowing speed
P_logic  = 5;      // [EST] W, Nucleo + Wi-Fi + 4 ToF + camera
crr      = 0.20;   // rolling resistance, small wheels on grass
slope    = 15;     // worst lawn slope, degrees
accel    = 0.3;    // m/s^2
mec_eff  = 0.70;   // mecanum roller losses
v_mow    = 0.30;   // m/s mowing speed

/* [Hidden] */
$fn = 48;
m3 = 3.4;
m4 = 4.5;
tray_t = 3;

// ---------- cutter motor presets ----------
is775   = CUT_MOTOR == "775_DC";
cm_name = is775 ? "Cutter motor RS-775 DC 12-18 V [EST]"
                : "Stock GIGAWATTS 250 W motor, 230 V AC [EST]";
cm_d       = is775 ? 42   : 58;   // body diameter (AC value [EST], measure)
cm_len     = is775 ? 67   : 80;   // body length
cm_boss    = is775 ? 17.5 : 22;   // front bearing boss
cm_hole_sp = is775 ? 29   : 0;    // 2 x M4 face screws (0 = collar clamp only)
cm_g       = is775 ? 330  : 480;

// ---------- deck material ----------
deck_t   = DECK_MATERIAL == "ALU_3" ? 3 : DECK_MATERIAL == "PLYWOOD_6" ? 6 : 5;
deck_rho = DECK_MATERIAL == "ALU_3" ? 2.68 : DECK_MATERIAL == "PLYWOOD_6" ? 0.68 : 0.75; // g/cm3, print = ~60 % fill
deck_col = DECK_MATERIAL == "ALU_3" ? "Silver" : DECK_MATERIAL == "PLYWOOD_6" ? "Tan" : "LightSlateGray";

// ---------- heights ----------
axle_z   = wheel_d / 2;
brk_top  = dm_d / 2 + dm_wall;          // motor axis to deck underside
brk_bot  = dm_d / 2 + dm_wall;          // motor axis to clamp bottom
deck_z   = axle_z + brk_top;            // deck underside
deck_top = deck_z + deck_t;
track    = deck_W + 2 * wheel_gap + wheel_w;   // wheel centre to wheel centre (Y)

spool_bot  = cut_height - spool_line_z;
spool_top  = spool_bot + spool_h;
tower_t    = 6;
tower_bot  = spool_top + coupler_len;   // underside of the tower plate
motor_face = tower_bot + tower_t;       // cutter motor face rests here
post_h     = tower_bot - deck_top;      // tower wall height above the deck
tower_bolt_r = 57;
cut_hole_d = spool_d + 12;

guard_r_in   = cut_d / 2 + guard_clear;
guard_r_out  = guard_r_in + guard_wall;
skirt_bot    = cut_height - guard_drop;
skirt_flange = 12;
skirt_hole_r = guard_r_in - skirt_flange / 2;

canopy_z   = deck_top + canopy_gap;     // canopy underside
canopy_t   = 2;
canopy_top = canopy_z + canopy_t;
canopy_L   = deck_L - 10;
canopy_W   = deck_W - 10;
mast_x     = canopy_L / 2 - 25;
estop_x    = -(canopy_L / 2 - 20);
sbc_x      = -70;
cam_z      = canopy_top + mast_h + 5;

// ---------- drive layout ----------
wheel_pos  = [[ wheelbase / 2,  track / 2], [ wheelbase / 2, -track / 2],
              [-wheelbase / 2,  track / 2], [-wheelbase / 2, -track / 2]];  // FL FR RL RR
wheel_hand = [1, -1, -1, 1];   // roller direction (visual only)
dm_front_y = deck_W / 2 - 2;   // gearbox output face, |y|
brk_len    = 30;               // clamp length along the motor
brk_w      = 50;               // clamp width along X
brk_hole_dx = 19.5;
brk_y      = dm_front_y - brk_len / 2;
split_gap  = 1.0;              // clamp halves close on the motor

// ---------- battery / box sizes ----------
bat_lx = 80;  bat_ly = 77;  bat_h = 21;   // 4 x 18650 side by side
aa_lx  = 62;  aa_ly  = 58;  aa_h  = 16;   // 4 x AA side by side
pwr_lx = 60;  pwr_ly = 22;  pwr_h = 12;
esc_lx = 45;  esc_ly = 25;  esc_h = 15;

// ---------- ToF sensors (front, rear, left, right) ----------
tof_poses  = [[ deck_L / 2 - 10, 0, 0], [-(deck_L / 2 - 10), 0, 180],
              [0,  deck_W / 2 - 10, 90], [0, -(deck_W / 2 - 10), -90]];
tof_hole_x = -3;
tof_hole_y = 7;
tof_pcb_w  = 25;   // [EST] VL53L9 board size
tof_pcb_h  = 20;   // [EST]

// ---------- helpers ----------
function rot2(p, a) = [p[0] * cos(a) - p[1] * sin(a), p[0] * sin(a) + p[1] * cos(a)];
function near_any(p, pts) = len([for (q = pts) if (norm([p[0] - q[0], p[1] - q[1]]) < q[2]) 1]) > 0;
function board_to_deck(pos, w, l, h) = [pos[0] + (h[1] - l / 2), pos[1] - (h[0] - w / 2)];
function sum_mass(t, i = 0) = i < len(t) ? t[i][1] * t[i][2] + sum_mass(t, i + 1) : 0;
function sum_moment(t, i = 0) = i < len(t) ? t[i][1] * t[i][2] * t[i][3] + sum_moment(t, i + 1) : [0, 0, 0];
function r1(x) = round(x * 10) / 10;

// ---------- deck hole lists ----------
tof_holes    = [for (t = tof_poses, s = [-1, 1]) [t[0], t[1]] + rot2([tof_hole_x, s * tof_hole_y], t[2])];
skirt_angles = [for (k = [0 : 7]) 22.5 + 45 * k];
skirt_holes  = [for (a = skirt_angles) [skirt_hole_r * cos(a), skirt_hole_r * sin(a)]];
tower_lugs   = [for (a = [45 : 90 : 315]) [tower_bolt_r * cos(a), tower_bolt_r * sin(a)]];
brk_holes    = [for (w = wheel_pos, s = [-1, 1]) [w[0] + s * brk_hole_dx, sign(w[1]) * brk_y]];
canopy_posts = concat([for (sx = [-1, 1], sy = [-1, 1]) [sx * (deck_L / 2 - 15), sy * (deck_W / 2 - 20)]],
                      [for (sx = [-1, 1], sy = [-1, 1]) [sx * 60, sy * (deck_W / 2 - 15)]]);
splice_holes = [for (s = [-1, 1], hx = [-12, 12], hy = [-14, 14]) [hx, s * 74 + hy]];
wire_holes   = [for (sx = [-1, 1], sy = [-1, 1]) [sx * (wheelbase / 2 - 40), sy * (deck_W / 2 - 8)]];

keepout = concat(
    [for (p = tof_holes)    [p[0], p[1], 9]],
    [for (p = skirt_holes)  [p[0], p[1], 9]],
    [for (p = tower_lugs)   [p[0], p[1], 12]],
    [for (p = brk_holes)    [p[0], p[1], 9]],
    [for (p = canopy_posts) [p[0], p[1], 9]],
    [for (p = splice_holes) [p[0], p[1], 9]],
    [for (p = wire_holes)   [p[0], p[1], 13]]);

deck_grid = [for (x = [grid_x0 : grid_pitch : grid_x1], y = [-grid_y : grid_pitch : grid_y], sx = [-1, 1])
                 if (!near_any([sx * x, y], keepout)) [sx * x, y]];

// ---------- printed-part masses ----------
deck_mass   = (deck_L * deck_W - PI * pow(cut_hole_d / 2, 2)) * deck_t * deck_rho / 1000 * 0.97;
canopy_mass = canopy_L * canopy_W * canopy_t * 1.2 / 1000 * 0.95;   // 2 mm PC / PETG sheet
skirt_mass  = (PI * (pow(guard_r_out, 2) - pow(guard_r_in, 2)) * (deck_z - skirt_bot)
             + PI * (pow(guard_r_in, 2) - pow(guard_r_in - skirt_flange, 2)) * 3) * 1.25 / 1000;

// ---------- weight budget: [name, qty, grams each, [x, y, z] of the group's CG] ----------
cg_bat = [bat_pos[0], bat_pos[1], deck_top + bat_h / 2];
cg_aa  = [aa_pos[0],  aa_pos[1],  deck_top + aa_h / 2];
PARTS = [
    // electronics
    ["NUCLEO-L073RZ [EST]",                     1, 32, [nuc_pos[0], nuc_pos[1], deck_top + tray_t + nuc_standoff + 3]],
    ["X-NUCLEO-67W61M1 Wi-Fi shield [EST]",     1, 15, [nuc_pos[0], nuc_pos[1], deck_top + tray_t + nuc_standoff + 14]],
    ["X-STM32MP-RBT01 motor board [EST]",       1, 45, [rbt_pos[0], rbt_pos[1], deck_top + tray_t + rbt_standoff + 5]],
    ["VL53L9 ToF board + cable [EST]",          4,  8, [0, 0, deck_top + 19]],
    ["USB camera [EST]",                        1, 45, [mast_x, 0, cam_z]],
    ["Linux board for the camera [EST]", INCLUDE_SBC ? 1 : 0, 120, [sbc_x, 0, canopy_top + 15]],
    // power
    ["18650 Li-ion cell",                       4, 47, cg_bat],
    ["4 x 18650 holder",                        1, 35, cg_bat],
    ["AA alkaline cell",     INCLUDE_AA_PACK ? 4 : 0, 23, cg_aa],
    ["4 x AA holder",        INCLUDE_AA_PACK ? 1 : 0, 15, cg_aa],
    ["4S BMS + 5 V buck + fuses [EST]",         1, 30, [pwr_pos[0], pwr_pos[1], deck_top + pwr_h / 2]],
    ["Cutter motor driver [EST]",               1, 40, [esc_pos[0], esc_pos[1], deck_top + esc_h / 2]],
    ["E-stop + main switch [EST]",              1, 35, [estop_x, 0, canopy_z]],
    ["Wiring and connectors [EST]",             1, 60, [0, 0, deck_top + 15]],
    // drive
    ["Mecanum wheel 75 x 35 [EST]",             4, wheel_g, [0, 0, axle_z]],
    ["Drive gearmotor JGA25-370 class [EST]",   4, dm_g,    [0, 0, axle_z]],
    ["Wheel hub coupler [EST]",                 4, 8,       [0, 0, axle_z]],
    // cutter
    [cm_name,                                   1, cm_g,    [0, 0, motor_face + cm_len / 2]],
    ["Trimmer spool + 1 mm line [EST]",         1, spool_g, [0, 0, spool_bot + spool_h / 2]],
    ["Shaft-to-spool adapter [EST]",            1, 15,      [0, 0, spool_top + coupler_len / 2]],
    // structure
    [str("Deck plate ", DECK_MATERIAL),         1, deck_mass,   [0, 0, deck_z + deck_t / 2]],
    ["Canopy sheet 2 mm",                       1, canopy_mass, [0, 0, canopy_z + canopy_t / 2]],
    ["Guard skirt, 2 halves",                   1, skirt_mass,  [0, 0, (skirt_bot + deck_z) / 2]],
    ["Motor clamp, saddle + cap",               4, 24, [0, 0, axle_z]],
    ["Cutter tower",                            1, 60, [0, 0, deck_top + post_h]],
    ["Board trays",                             2, 17, [105, -2.5, deck_top + tray_t / 2]],
    ["ToF brackets",                            4,  4, [0, 0, deck_top + 12]],
    ["Camera mast",                             1, 30, [mast_x, 0, canopy_top + mast_h / 2]],
    ["Deck splice plates", DECK_MATERIAL == "PETG_PRINT" ? 2 : 0, 9, [0, 0, deck_top + 2]],
    ["Canopy posts",                            8,  6, [0, 0, deck_top + canopy_gap / 2]],
    ["Screws, nuts, standoffs [EST]",           1, 50, [0, 0, deck_top]]
];

// =====================================================================
//  2D profiles
// =====================================================================
module rounded_rect(L, W, r) {
    offset(r = r) square([L - 2 * r, W - 2 * r], center = true);
}

module deck_2d() {
    difference() {
        rounded_rect(deck_L, deck_W, deck_r);
        circle(d = cut_hole_d, $fn = 96);                       // spool passes through
        for (p = deck_grid)    translate(p) circle(d = m3, $fn = 16);
        for (p = brk_holes)    translate(p) circle(d = m3, $fn = 16);
        for (p = skirt_holes)  translate(p) circle(d = m3, $fn = 16);
        for (p = tower_lugs)   translate(p) circle(d = m4, $fn = 16);
        for (p = canopy_posts) translate(p) circle(d = m3, $fn = 16);
        for (p = tof_holes)    translate(p) circle(d = m3, $fn = 16);
        for (p = splice_holes) translate(p) circle(d = m3, $fn = 16);
        for (p = wire_holes)   translate(p) circle(d = 12, $fn = 32);  // drive motor wires
    }
}

module canopy_2d() {
    difference() {
        rounded_rect(canopy_L, canopy_W, deck_r);
        for (p = canopy_posts) translate(p) circle(d = m3, $fn = 16);
        for (i = [-2 : 2]) translate([i * 10, 0]) square([5, cm_d + 10], center = true);  // motor vents
        translate([estop_x, 0]) circle(d = 22.5);                                         // 22 mm E-stop
        for (sx = [-1, 1], sy = [-1, 1]) translate([mast_x + sx * 12, sy * 12]) circle(d = m3, $fn = 16);
        for (p = [[mast_x - 25, 0], [60, 0], [-60, 0]]) translate(p) circle(d = 14);     // cable holes
    }
}

// =====================================================================
//  Printed / cut parts
// =====================================================================

// Upper clamp half. Local frame: motor axis along Y through the origin,
// deck underside at z = brk_top.
module motor_saddle() {
    difference() {
        translate([-brk_w / 2, -brk_len / 2, split_gap / 2]) cube([brk_w, brk_len, brk_top - split_gap / 2]);
        rotate([90, 0, 0]) cylinder(d = dm_d + 0.2, h = brk_len + 2, center = true);
        for (s = [-1, 1]) translate([s * brk_hole_dx, 0, -50]) cylinder(d = m3, h = 100, $fn = 16);
    }
}

// Lower clamp half with M3 nut traps underneath.
module motor_cap() {
    difference() {
        translate([-brk_w / 2, -brk_len / 2, -brk_bot]) cube([brk_w, brk_len, brk_bot - split_gap / 2]);
        rotate([90, 0, 0]) cylinder(d = dm_d + 0.2, h = brk_len + 2, center = true);
        for (s = [-1, 1]) {
            translate([s * brk_hole_dx, 0, -50]) cylinder(d = m3, h = 100, $fn = 16);
            translate([s * brk_hole_dx, 0, -brk_bot - 1]) cylinder(d = 6.4, h = 3.5, $fn = 6);
        }
    }
}

// Hat over the spool hole: holds the cutter motor shaft-down and stops
// clippings flying up onto the electronics. z = 0 is the deck top.
module cutter_tower() {
    wall_ri = cut_hole_d / 2 + 1;
    wall_ro = wall_ri + 2.5;
    disc_r  = wall_ro + 0.5;
    col_ri  = cm_d / 2 + 0.25;
    col_ro  = col_ri + 4.5;
    top     = post_h + tower_t;
    difference() {
        union() {
            cylinder(r = wall_ro, h = post_h, $fn = 96);
            translate([0, 0, post_h]) cylinder(r = disc_r, h = tower_t, $fn = 96);
            for (a = [45 : 90 : 315]) hull() {
                rotate(a) translate([tower_bolt_r, 0, 0]) cylinder(r = 7, h = 5);
                rotate(a) translate([wall_ri, 0, 0]) cylinder(r = 9, h = 5);
            }
            translate([0, 0, top]) cylinder(r = col_ro, h = collar_h, $fn = 96);
            translate([col_ri - 1, -7, top]) cube([col_ro - col_ri + 10, 14, collar_h]);   // clamp ears
        }
        translate([0, 0, -1]) cylinder(r = wall_ri, h = post_h + 1, $fn = 96);         // hollow hat
        translate([0, 0, top]) cylinder(r = col_ri, h = collar_h + 1, $fn = 96);       // motor collar
        translate([col_ri - 2, -1, top]) cube([col_ro - col_ri + 14, 2, collar_h + 1]); // clamp slot
        translate([col_ro + 5, 0, top + collar_h / 2]) rotate([90, 0, 0]) cylinder(d = m3, h = 30, center = true, $fn = 16);
        translate([0, 0, post_h - 1]) cylinder(d = cm_boss + 1, h = tower_t + 2);       // boss + shaft
        if (cm_hole_sp > 0)
            for (s = [-1, 1]) translate([s * cm_hole_sp / 2, 0, post_h - 1]) cylinder(d = m4, h = tower_t + 2, $fn = 16);
        for (p = tower_lugs) translate([p[0], p[1], -1]) cylinder(d = m4, h = 10, $fn = 16);
    }
}

// Ring around the line. half = 1 keeps only the +Y half (print two).
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

// ToF bracket. z = 0 is the deck top, +X looks outward.
module tof_bracket() {
    difference() {
        union() {
            translate([-8, -11, 0]) cube([16, 22, 3]);
            translate([8, 0, 3]) rotate([0, tof_tilt, 0]) translate([-3, -11, 0]) cube([3, 22, 28]);
            translate([2, -11, 3]) cube([6, 22, 5]);   // fillet block
        }
        for (s = [-1, 1]) translate([tof_hole_x, s * tof_hole_y, -1]) cylinder(d = m3, h = 5, $fn = 16);
        translate([8, 0, 3]) rotate([0, tof_tilt, 0]) {
            for (s = [-1, 1]) translate([-5, s * 10, 19]) rotate([0, 90, 0]) cylinder(d = 2.2, h = 8, $fn = 12); // M2.5 pilot [EST]
            translate([-5, 0, 10]) rotate([0, 90, 0]) cylinder(d = 7, h = 8);   // cable
        }
    }
}

// Camera mast. z = 0 is the canopy top, camera looks forward and down.
module camera_mast() {
    difference() {
        union() {
            translate([-18, -18, 0]) cube([36, 36, 4]);
            translate([-7, -7, 0]) cube([14, 14, mast_h]);
            hull() {
                translate([-7, -7, mast_h - 25]) cube([14, 14, 1]);
                translate([7, 0, mast_h]) rotate([0, cam_tilt, 0]) translate([-3, -20, -15]) cube([3, 40, 40]);
            }
        }
        for (sx = [-1, 1], sy = [-1, 1]) translate([sx * 12, sy * 12, -1]) cylinder(d = m3, h = 6, $fn = 16);
        translate([-4, -4, -1]) cube([8, 8, mast_h - 30]);   // weight saving
        translate([7, 0, mast_h]) rotate([0, cam_tilt, 0])
            for (sy = [-1, 1], sz = [-1, 1]) translate([-9, sy * 17, 5 + sz * 17]) rotate([0, 90, 0]) cylinder(d = 1.8, h = 10, $fn = 12);
    }
}

// Splice plate joining the two printed deck halves (sits on the deck top).
module splice_plate() {
    difference() {
        translate([-20, -22, 0]) cube([40, 44, 4]);
        for (hx = [-12, 12], hy = [-14, 14]) translate([hx, hy, -1]) cylinder(d = m3, h = 6, $fn = 16);
    }
}

// Tray that carries a board on standoffs and screws to the deck grid.
// Deck coordinates, z = 0 is the deck top.
module board_tray(pos, w, l, holes, standoff) {
    m  = 3;
    x0 = pos[0] - l / 2 - m;  x1 = pos[0] + l / 2 + m;
    y0 = pos[1] - w / 2 - m;  y1 = pos[1] + w / 2 + m;
    bh = [for (h = holes) board_to_deck(pos, w, l, h)];
    screws = [for (g = deck_grid)
                  if (g[0] > x0 + 5 && g[0] < x1 - 5 && g[1] > y0 + 5 && g[1] < y1 - 5
                      && !near_any(g, [for (b = bh) [b[0], b[1], 8]])) g];
    difference() {
        union() {
            translate([x0, y0, 0]) cube([x1 - x0, y1 - y0, tray_t]);
            for (b = bh) translate([b[0], b[1], 0]) cylinder(d = 7, h = tray_t + standoff);
        }
        for (b = bh) translate([b[0], b[1], -1]) cylinder(d = 2.7, h = tray_t + standoff + 2, $fn = 16);  // M3 self-tap
        for (g = screws) translate([g[0], g[1], -1]) cylinder(d = m3, h = tray_t + 2, $fn = 16);
    }
}

// =====================================================================
//  Bought-in parts (visual stand-ins)
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

module nucleo_stack() {   // board frame, z = 0 at the PCB underside
    color("WhiteSmoke") difference() {
        cube([nuc_w, nuc_l, 1.6]);
        for (h = nuc_holes) translate([h[0], h[1], -1]) cylinder(d = 3.2, h = 4, $fn = 16);
    }
    color("Black") for (x = [2, nuc_w - 7]) translate([x, 3, -8]) cube([5, 48, 19]);   // morpho
    color("Silver") translate([nuc_w / 2 - 4, nuc_l - 8, 1.6]) cube([8, 9, 4]);        // ST-LINK USB
    color("Black") for (x = [9, 58]) translate([x, 8, 1.6]) cube([2.5, 52, 11]);       // Arduino headers
    translate([8, 4, 12.6]) {                                                          // X-NUCLEO-67W61M1
        color("SteelBlue") cube([53.3, 68.6, 1.6]);
        color("Silver") translate([18, 40, 1.6]) cube([15, 22, 3]);                    // ST67W61 module [EST]
    }
}

module rbt01() {          // board frame, z = 0 at the PCB underside
    color("DarkGreen") difference() {
        cube([rbt_w, rbt_l, 1.6]);
        for (h = rbt_holes) translate([h[0], h[1], -1]) cylinder(d = 3.2, h = 4, $fn = 16);
    }
    color("LimeGreen") for (y = [12, rbt_l - 32]) translate([rbt_w - 12, y, 1.6]) cube([10, 20, 10]);  // CN4 / CN5 [EST]
    color("LimeGreen") translate([2, rbt_l / 2 - 6, 1.6]) cube([10, 12, 10]);                          // CN3 VS [EST]
    color("Black") translate([rbt_w / 2 - 26, 4, 1.6]) cube([52, 5, 8.5]);                             // 40-pin [EST]
    color("DimGray") for (y = [rbt_l / 2 - 20, rbt_l / 2 + 8]) translate([rbt_w / 2 - 4, y, 1.6]) cube([8, 8, 1]);
}

module on_deck_board(pos, w, l, z) {
    translate([pos[0], pos[1], z]) rotate([0, 0, -90]) translate([-w / 2, -l / 2, 0]) children();
}

module cell_pack(lx, ly, cell_d, cell_l, col) {
    color("Black") translate([-lx / 2, -ly / 2, 0]) cube([lx, ly, 3]);
    color(col) for (i = [0 : 3]) translate([-lx / 2 + lx / 8 + i * lx / 4, 0, 3 + cell_d / 2])
        rotate([90, 0, 0]) cylinder(d = cell_d, h = cell_l, center = true);
}

module tof_board() {      // on the bracket face
    translate([8, 0, 3]) rotate([0, tof_tilt, 0]) translate([0, 0, 16]) {
        color("Purple") translate([0, -tof_pcb_w / 2, -tof_pcb_h / 2]) cube([1.6, tof_pcb_w, tof_pcb_h]);
        color("Black") translate([1.6, -5, -3]) cube([2.5, 10, 6]);
    }
}

module usb_camera() {     // on the mast plate
    translate([7, 0, mast_h]) rotate([0, cam_tilt, 0]) translate([0, 0, 5]) {
        color("Black") translate([0, -19, -19]) cube([4, 38, 38]);
        color("DimGray") translate([4, 0, 0]) rotate([0, 90, 0]) cylinder(d = 14, h = 14);
    }
}

// =====================================================================
//  Assembly
// =====================================================================
module assembly() {
    // deck and guard
    color(deck_col) translate([0, 0, deck_z]) linear_extrude(deck_t) deck_2d();
    color("DarkOrange") guard_skirt(0);
    if (DECK_MATERIAL == "PETG_PRINT")
        color("Orange") for (s = [-1, 1]) translate([0, s * 74, deck_top]) splice_plate();

    // wheels, drive motors, clamps
    for (i = [0 : 3]) let (w = wheel_pos[i], s = sign(w[1])) {
        translate([w[0], w[1], axle_z]) mecanum(wheel_hand[i]);
        translate([w[0], s * brk_y, axle_z]) color("Orange") { motor_saddle(); motor_cap(); }
        color("Gold") translate([w[0], s * (dm_front_y - dm_len / 2), axle_z])
            rotate([90, 0, 0]) cylinder(d = dm_d, h = dm_len, center = true);
        color("Silver") translate([w[0], s * (dm_front_y + 6), axle_z])
            rotate([90, 0, 0]) cylinder(d = dm_shaft, h = 12, center = true);
    }

    // cutter
    color("Orange") translate([0, 0, deck_top]) cutter_tower();
    color("DimGray") translate([0, 0, motor_face]) cylinder(d = cm_d, h = cm_len);
    color("Silver") translate([0, 0, spool_top]) cylinder(d = 12, h = motor_face - spool_top);
    color("LimeGreen") translate([0, 0, spool_bot]) cylinder(d = spool_d, h = spool_h);
    color("Yellow") for (a = [0, 180]) rotate(a) translate([spool_d / 2 - 2, 0, cut_height])
        rotate([0, 90, 0]) cylinder(d = 1.5, h = cut_d / 2 - spool_d / 2 + 2, $fn = 8);
    if (SHOW_CUT_DISC)
        color([1, 0.2, 0.2, 0.18]) translate([0, 0, cut_height - 0.25]) cylinder(d = cut_d, h = 0.5, $fn = 96);

    // boards
    color("LightGray") translate([0, 0, deck_top]) board_tray(nuc_pos, nuc_w, nuc_l, nuc_holes, nuc_standoff);
    on_deck_board(nuc_pos, nuc_w, nuc_l, deck_top + tray_t + nuc_standoff) nucleo_stack();
    color("LightGray") translate([0, 0, deck_top]) board_tray(rbt_pos, rbt_w, rbt_l, rbt_holes, rbt_standoff);
    on_deck_board(rbt_pos, rbt_w, rbt_l, deck_top + tray_t + rbt_standoff) rbt01();

    // power
    translate([bat_pos[0], bat_pos[1], deck_top]) cell_pack(bat_lx, bat_ly, 18.6, 65, "RoyalBlue");
    if (INCLUDE_AA_PACK) translate([aa_pos[0], aa_pos[1], deck_top]) cell_pack(aa_lx, aa_ly, 14.5, 50.5, "Gold");
    color("MediumPurple") translate([pwr_pos[0] - pwr_lx / 2, pwr_pos[1] - pwr_ly / 2, deck_top]) cube([pwr_lx, pwr_ly, pwr_h]);
    color("RoyalBlue") translate([esc_pos[0] - esc_lx / 2, esc_pos[1] - esc_ly / 2, deck_top]) cube([esc_lx, esc_ly, esc_h]);

    // ToF sensors
    for (t = tof_poses) translate([t[0], t[1], deck_top]) rotate(t[2]) {
        color("Orange") tof_bracket();
        tof_board();
    }

    // canopy, mast, camera, E-stop
    color("Orange") for (p = canopy_posts) translate([p[0], p[1], deck_top]) cylinder(d = 8, h = canopy_gap);
    if (SHOW_CANOPY) color([0.75, 0.85, 1, 0.35]) translate([0, 0, canopy_z]) linear_extrude(canopy_t) canopy_2d();
    translate([mast_x, 0, canopy_top]) { color("Orange") camera_mast(); usb_camera(); }
    translate([estop_x, 0, 0]) {
        color("DimGray") translate([0, 0, canopy_z - 35]) cylinder(d = 28, h = 35);
        color("Yellow") translate([0, 0, canopy_top]) cylinder(d = 34, h = 6);
        color("Red") translate([0, 0, canopy_top + 6]) cylinder(d = 40, h = 12);
    }
    if (INCLUDE_SBC) color("MidnightBlue") translate([sbc_x - 45, -33, canopy_top + 8]) cube([90, 66, 18]);
}

// =====================================================================
//  Report (console)
// =====================================================================
M_total = sum_mass(PARTS);
CG      = sum_moment(PARTS) / M_total;
f_front = (wheelbase / 2 + CG[0]) / wheelbase;
f_left  = (track / 2 + CG[1]) / track;
F_need  = M_total / 1000 * (9.81 * (crr * cos(slope) + sin(slope)) + accel);   // N
T_wheel = F_need / mec_eff / 4 * wheel_d / 2000;                                // N.m per wheel
rpm_need = v_mow / (PI * wheel_d / 1000) * 60;
pack_V  = cells_s * cell_V;
pack_Wh = pack_V * cell_mAh / 1000;
P_total = P_cut + P_drive + P_logic;

module report() {
    echo("=================== GRASSKIT ROVER - WEIGHT BUDGET ===================");
    for (p = PARTS) if (p[1] > 0) echo(str(p[1], " x ", p[0], "  ", r1(p[2]), " g  = ", r1(p[1] * p[2]), " g"));
    echo(str("TOTAL MASS        : ", round(M_total), " g  (", r1(M_total / 100) / 10, " kg)"));
    echo(str("Centre of gravity : x ", r1(CG[0]), "  y ", r1(CG[1]), "  z ", r1(CG[2]), " mm"));
    echo(str("Wheel load (g)    : FL ", round(M_total * f_front * f_left), "  FR ", round(M_total * f_front * (1 - f_left)),
             "  RL ", round(M_total * (1 - f_front) * f_left), "  RR ", round(M_total * (1 - f_front) * (1 - f_left))));
    echo(str("Tip-over angle    : sideways ", round(atan((track / 2 - abs(CG[1])) / CG[2])), " deg, fore/aft ",
             round(atan((wheelbase / 2 - abs(CG[0])) / CG[2])), " deg"));
    echo("--------------------------- DRIVE ---------------------------");
    echo(str("Push needed (", slope, " deg slope, Crr ", crr, ") : ", r1(F_need), " N"));
    echo(str("Torque per wheel  : ", r1(T_wheel * 100) / 100, " N.m = ", r1(T_wheel * 10.197), " kg.cm  (buy >= 2x this)"));
    echo(str("Wheel rpm for ", v_mow, " m/s : ", round(rpm_need), " rpm"));
    echo("--------------------------- BATTERY ---------------------------");
    echo(str("Pack              : ", cells_s, "S ", cell_mAh, " mAh = ", r1(pack_V), " V nominal, ", r1(pack_Wh), " Wh"));
    echo(str("Load while mowing : ", P_total, " W -> ", r1(P_total / pack_V), " A (cutter alone ", r1(P_cut / pack_V), " A)"));
    echo(str("Runtime (80 % DoD): ", round(pack_Wh * 0.8 / P_total * 60), " min"));
    echo("--------------------------- GEOMETRY ---------------------------");
    echo(str("Footprint         : ", max(deck_L, wheelbase + wheel_d), " x ", track + wheel_w, " mm, top of camera ~", round(cam_z + 20), " mm"));
    echo(str("Cut height ", cut_height, " mm, swath ", cut_d, " mm, clearance under motor clamps ", axle_z - brk_bot, " mm"));
    if (post_h < 3) echo("WARNING: cut_height too low - the spool hits the deck. Raise cut_height or coupler_len.");
    if (motor_face + cm_len + 5 > canopy_z) echo("WARNING: cutter motor hits the canopy. Raise canopy_gap.");
    if (norm([wheelbase / 2 - dm_d / 2, dm_front_y - dm_len]) < guard_r_out + 3) echo("WARNING: drive motors inside the guard ring.");
    if (skirt_bot < 8) echo("WARNING: guard skirt drags on the ground.");
}

// =====================================================================
//  Render
// =====================================================================
report();

if (MODE == "assembly") assembly();
else if (MODE == "deck_2d") deck_2d();
else if (MODE == "deck_front_half")
    linear_extrude(deck_t) intersection() { deck_2d(); translate([0, -deck_W]) square([deck_L, 2 * deck_W]); }
else if (MODE == "deck_rear_half")
    linear_extrude(deck_t) intersection() { deck_2d(); translate([-deck_L, -deck_W]) square([deck_L, 2 * deck_W]); }
else if (MODE == "deck_splice") splice_plate();
else if (MODE == "canopy_2d") canopy_2d();
else if (MODE == "motor_bracket") {                       // print 4 sets
    translate([0, 0, brk_top]) rotate([180, 0, 0]) motor_saddle();
    translate([brk_w + 10, 0, brk_bot]) motor_cap();
}
else if (MODE == "cutter_tower") cutter_tower();          // print upright, tree supports under the top plate
else if (MODE == "guard_skirt_half") translate([0, 0, deck_z]) mirror([0, 0, 1]) guard_skirt(1);  // print 2, TPU or PETG
else if (MODE == "tof_bracket") tof_bracket();            // print 4
else if (MODE == "camera_mast") camera_mast();
else if (MODE == "nucleo_tray") translate([-nuc_pos[0], -nuc_pos[1], 0]) board_tray(nuc_pos, nuc_w, nuc_l, nuc_holes, nuc_standoff);
else if (MODE == "rbt01_tray")  translate([-rbt_pos[0], -rbt_pos[1], 0]) board_tray(rbt_pos, rbt_w, rbt_l, rbt_holes, rbt_standoff);
