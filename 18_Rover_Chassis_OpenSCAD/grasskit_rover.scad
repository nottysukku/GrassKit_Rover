// =====================================================================
//  GrassKit Rover v3 - enclosed, 3D-printed body for the final parts list
//  (documentation/00_Project_Status/GrassKit_Rover_Final_BOM_2026-11-04.xlsx)
//  OpenSCAD 2021.01 or newer.   Units: mm and grams.
//
//  Files:  grasskit_rover.scad        this file: settings, layout, report
//          rover_printed_parts.scad   every printed part
//          rover_bought_parts.scad    stand-ins for boards, motors, cells
//          v2_first_parts_list/       v2: 75 mm mecanum, JGA25, RS-775, 4 x 18650
//          grasskit_rover_v1_open_frame.scad   v1: open frame
//
//  Frame: origin on the ground under the middle of the wheelbase.
//         +X = forward, +Y = left, +Z = up.
//
//  MODE = "assembly" -> the whole rover, plus a weight / CG / wheel load /
//                       torque / battery / print-bed report in the console.
//  Any other MODE    -> one printed part in its print orientation.
//                       F6, then File > Export > STL.
//
//  [ST]  value from an ST document in documentation/
//  [web] value from the shop page linked in the BOM
//  [EST] estimate: measure or weigh your part and change the number.
//
//  ---------------------------------------------------------------------
//  BODY
//  ---------------------------------------------------------------------
//   The tub (floor + walls) and the lid each print as 4 quarters of about
//   220 x 140 mm, which fit a 250 x 210 bed. The quarters bolt together
//   through internal ribs on the x = 0 and y = 0 joints. Boards, plates
//   and the battery fix to bosses with M3 heat-set inserts (4.0 mm hole).
//
//   Wheel motors: each 36 mm gearmotor lies on the floor, bolted by its
//   gearbox face to a thickened side wall. Its lower edge drops through a
//   floor slot into a printed tray under the floor, and a printed clamp
//   holds it from above. The trays sit outside the trimmer-line circle.
//
//   Cutter: the line turns 12 mm under the floor, between the axles. The
//   D3536 outrunner hangs shaft-down from the top of the cutter tower,
//   over the hole in the floor. An aluminium hub carries the spool.
//
//   Lid = floating canopy. It rests on the tub rim with no screws,
//   located by its skirt and held by 2 magnet pairs and its own weight.
//   Two A3144 Hall sensors in the tub read two more lid magnets: lift the
//   lid and the G474 stops the cutter. The lidar mast, camera pod, E-stop,
//   vents, fenders and rain sensor ride on the lid. Leave slack in their
//   cables, or fit a plug, so the lid can come off for a battery swap.
//
//   Bumpers: 4 halves (front/rear x left/right), each on 2 sliding 8 mm
//   pins through the wall with a spring, and a roller micro-switch behind
//   one pin inside the tub.
//
//  ---------------------------------------------------------------------
//  POWER (BOM section 5)
//  ---------------------------------------------------------------------
//   4S4P 14.8 V 10 Ah - XT90-S - 30 A switch - INA226 shunt - bus (TVS SM15T22A)
//    bus -+- 30 A relay -+- 20 A fuse - B-G431B-ESC1 (1000 uF) - D3536 cutter
//         |              +- 10 A fuse - RBT01 VS (1000 uF) - 4 drive motors
//         |   relay coil runs through the E-stop NC contact; the G474 can drop it too
//         +-  5 A fuse -+- Pololu D36V50F5 5 V - N6 CN8 (CN9 on [3-4]), G474,
//                       |  Wi-Fi shield, LD19, ToF boards, ESC fan
//                       +- USB-C 5 V 3 A buck - STM32MP257F-DK CN21
//   * The E-stop and relay cut motor power only. Logic stays on and
//     reports the fault.
//   * The drive motors are 12 V parts on a 16.8 V pack: cap PWM at ~75 %.
//   * Keep each drive motor under the RBT01's 2.5 A with a firmware
//     current limit. The report shows the current needed on a slope.
//   * The IP65 box on the BOM is not needed: the printed body is the
//     enclosure. Glue 1 mm clear covers behind the ToF and camera windows.
// =====================================================================

/* [Render] */
MODE = "assembly"; // [assembly, tub_front_left, tub_front_right, tub_rear_left, tub_rear_right, lid_front_left, lid_front_right, lid_rear_left, lid_rear_right, motor_tray, motor_clamp, cutter_tower, guard_skirt_half, wheel_fender, deck_front, deck_rear, battery_cradle, power_plate, esc_mount, tof_holder_l8, tof_holder_l9, bumper_half, bumper_switch_bracket, camera_pod, camera_pod_cover, lidar_mast, vent_base, vent_cap]
VIEW = "closed"; // [closed, lid_lifted, no_lid]
INCLUDE_FENDERS = true;
INCLUDE_RAIN_SENSOR = true;   // FC-37 board on the lid (BOM row 16, optional)
INCLUDE_GNSS = false;         // X-NUCLEO-GNSS1A1 on the N6 Arduino header (BOM row 12, optional)
SHOW_CUT_DISC = true;         // translucent disc = area the trimmer line sweeps

/* [Printer] */
bed_x = 250;              // Prusa MK4 250 x 210, Bambu X1/P1/A1 256 x 256
bed_y = 210;
print_rho = 1.27;         // g/cm3: PETG 1.27, ASA 1.07, PLA 1.24
insert_d = 4.0;           // hole for M3 heat-set inserts
insert_depth = 6;

/* [Wheels and drive] */
wheel_d   = 130;   // [web] 130 x 60 off-road wheel, 12 mm hex hub
wheel_w   = 60;
wheel_g   = 158;   // [web]
wheelbase = 320;   // front axle to rear axle (X)
wheel_gap = 6;     // tub wall to wheel inner face
dm_d       = 36;   // 36 mm planetary gearmotor, 12 V, 100 rpm, 51:1
dm_gb_len  = 50;   // [EST] gearbox length (the shop page says 55)
dm_mot_len = 57;   // [web] motor can
dm_enc_len = 18;   // [EST] Hall encoder cap
dm_spigot  = [12, 4];   // [EST] output boss diameter, height
dm_shaft   = [8, 20];   // [web] 8 mm shaft, 20 mm long
dm_face_pcd = 28;  // [EST] face screw circle - measure your gearbox
dm_face_n  = 4;    // [EST] face screws (M3)
dm_g       = 380;  // [web] 350 g + [EST] 30 g encoder
dm_rpm     = 100;  // [web] at 12 V
dm_stall_kgcm = 32;   // [web]
dm_stall_A = 6.5;     // [web]
dm_noload_A = 0.23;   // [web]
coupling_len = 18;    // brass 12 mm hex coupling (BOM row 20)
rbt_limit_A = 2.5;    // X-STM32MP-RBT01 limit per motor (BOM row 6)

/* [Body] */
body_L     = 440;  // tub outside length
body_W     = 280;  // tub outside width
body_r     = 20;   // corner radius
tub_h      = 70;   // floor underside to tub rim
floor_t    = 3;
tub_wall   = 2.5;
motor_pad  = 6;    // wall thickness where a gearbox face bolts on
lid_wall   = 2.2;
lid_t      = 2.5;  // lid roof
lid_headroom = 5;  // tallest part inside to the lid roof
lid_overlap  = 10; // lid skirt hangs this far over the tub wall
lid_clear  = 0.4;  // lid skirt to tub wall
lid_chamfer = 10;
bot_chamfer = 3;
rib_t      = 5;    // joint rib thickness per quarter
rib_w      = 9;    // joint rib depth

/* [Cutter] */
cut_height   = 40;   // trimmer line height above the ground
line_floor_gap = 12; // trimmer line to floor underside
cut_d        = 200;  // line tip-to-tip diameter after trimming the line
cut_x        = 0;    // line centre on X
spool_d      = 80;   // [EST] bump spool diameter - measure yours
spool_h      = 40;   // [EST]
spool_line_z = 12;   // [EST] line eyelet height above the spool bottom
spool_g      = 110;  // [EST] spool, cap, spring and 1.6 mm line
adapter_len  = 18;   // [EST] aluminium hub: motor shaft end to spool top
cm_d     = 35;       // [EST] DYS D3536-8 1000 KV outrunner bell diameter
cm_len   = 36;       // [EST] base to shaft-end face of the bell
cm_mount = [16, 19]; // [EST] X-mount screw spacing (M3)
cm_g     = 102;      // [EST]
cm_kv    = 1000;
guard_clear  = 10;
guard_wall   = 2.5;
guard_drop   = 10;   // skirt ends this far below the line plane
dam_h        = 10;   // ring around the spool hole, stops water and clippings

/* [Boards] */
// STM32MP257F-DK: board frame x along the 100 mm side, y from the
// USB/RJ45 edge (y = 0). Holes from UM3385 fig. 6; the 4th is read off it.
mp2_size  = [100, 79.6];   // [ST]
mp2_holes = [[3.55, 76.05], [3.55, 4], [96, 4], [96, 62.2]];
mp2_pos   = [-123.5, 6];   // board-frame origin; turned 90 deg, USB edge faces forward
// NUCLEO-N657X0-Q: x across the 70 mm side, y from the USB/RJ45 edge (0)
// to the ST-LINK edge (133.35). UM3417 fig. 7, fitted to the Arduino pattern.
nuc_w = 70;  nuc_l = 133.35;
nuc_holes = [[10.85, 105.09], [59.11, 103.82], [43.87, 53.02]];
// NUCLEO-G474RE (Nucleo-64): x across 70, y to the ST-LINK end (82.5).
g4_size  = [70, 82.5];
g4_holes = [[10.87, 54.78], [59.13, 53.51], [43.89, 2.71]];   // [EST] check with calipers
rbt_size = [85, 56];   // [EST] X-STM32MP-RBT01 - measure yours
rbt_inset = 3.5;       // [EST]
esc_pcb  = [30, 41];   // [ST] UM2516 B-G431B-ESC1
board_standoff = 6;    // board boss height on the decks

/* [Layout] */
deck_standoff = 35;       // M3 hex standoffs: floor boss to board deck
deck_hw  = 118;           // deck half-width (Y)
bat_size = [90, 80, 70];  // [web] 4S4P pack L x W x H (Robu listing)
bat_g    = 903;           // [web]
bat_boss_h = 7;
pwr_size = [90, 70];
pwrA_pos = [-75, 93];     // relay, INA226, fuse holders, ground bus
pwrB_pos = [-75, -93];    // 5 V regulator, MP2 buck, XT60 branches
esc_plate = [60, 45];
esc_pos  = [85, 82.5];
plate_boss_h = 10;
tof_z_off   = 20;         // ToF window centre above the floor
tof_depth   = 24;         // ToF holder depth from the wall
side_tof_x  = 25;
cam_x       = 185;        // camera pod on the lid
cam_mast_h  = 10;
cam_tilt    = 20;         // camera looks down by this angle
lidar_pos   = [-30, 0];
lidar_mast_h = 50;
estop_pos   = [-80, -95];
vent_pos    = [[85, 82], [-80, 93]];
rain_pos    = [120, -85];
bump_gap    = 12;         // tub wall to bumper back face
bump_h      = 40;
bump_t      = 6;
bump_reach  = 206;        // bumper half length from the centre line
bump_pins   = [70, 112];  // pin positions from the centre line
bump_pin_d  = 8;

/* [Power and performance budget] */
bat_V   = 14.8;
bat_Ah  = 10;
P_cut   = 100;    // [EST] W while cutting (BOM design basis)
P_drive = 25;
P_mp2   = 8;
P_n6    = 2.5;    // N6 + camera
P_g4    = 1.5;    // G474 + Wi-Fi
P_lidar = 1.5;
P_misc  = 1.5;    // ToF, INA226, fan
crr     = 0.20;
slope   = 15;
accel   = 0.3;
mec_eff = 0.75;
skid    = 2.0;    // extra torque factor while skid-steering
v_mow   = 0.30;

/* [Hidden] */
$fn = 48;
m3 = 3.4;

// ---------- heights ----------
axle_z    = wheel_d / 2;
deck_z    = cut_height + line_floor_gap;     // floor underside
floor_top = deck_z + floor_t;
tub_top   = deck_z + tub_h;                  // tub rim
track     = body_W + 2 * wheel_gap + wheel_w;

// ---------- drive ----------
dm_len    = dm_gb_len + dm_mot_len + dm_enc_len;
dm_face_y = body_W / 2 - motor_pad;          // gearbox face, |y|
dm_end_y  = dm_face_y - dm_len;              // encoder end, |y|
axle_x    = [wheelbase / 2, -wheelbase / 2];
wheel_pos = [[axle_x[0],  track / 2], [axle_x[0], -track / 2],
             [axle_x[1],  track / 2], [axle_x[1], -track / 2]];   // FL FR RL RR
motor_spots = [[axle_x[0], 1], [axle_x[0], -1], [axle_x[1], 1], [axle_x[1], -1]];
slot_hw   = (axle_z - floor_top < dm_d / 2 + 0.5 ? sqrt(pow(dm_d / 2 + 0.5, 2) - pow(max(0, axle_z - floor_top), 2)) : 0) + 1;
tray_bot  = axle_z - dm_d / 2 - 3;
tray_hw   = slot_hw + 13;
tray_y0   = dm_end_y - 7;
tray_y1   = body_W / 2 - bot_chamfer - 0.5;
tray_screw_x = slot_hw + 9;
tray_screw_y = [dm_face_y - 15, dm_face_y - dm_len / 2, dm_end_y + 7];
clamp_y   = dm_face_y - dm_len / 2;

// ---------- cutter ----------
spool_bot  = cut_height - spool_line_z;
spool_top  = spool_bot + spool_h;
bell_bot   = spool_top + adapter_len;
cm_base    = bell_bot + cm_len;
tower_top  = cm_base + 5;
cut_hole_d = spool_d + 12;
dam_ro     = cut_hole_d / 2 + 2.4;
drum_ri    = dam_ro + 1.6;
drum_ro    = drum_ri + 2.4;
drum_top   = max(spool_top + 6, floor_top + dam_h + 4);
col_ri     = cm_d / 2 + 3;
col_ro     = col_ri + 2.4;
cone_top   = drum_top + (drum_ro - col_ro);   // 45 degree cone
tower_bolt_r = 60;
tower_lug_angles = [60, 120, 240, 300];
guard_r_in   = cut_d / 2 + guard_clear;
guard_r_out  = guard_r_in + guard_wall;
skirt_bot    = cut_height - guard_drop;
skirt_flange = 12;
skirt_hole_r = guard_r_in - skirt_flange / 2;

// ---------- battery, decks ----------
bat_clear  = 1;
cradle_wall = 2;
cradle     = [bat_size[1] + 2 * (bat_clear + cradle_wall), bat_size[0] + 2 * (bat_clear + cradle_wall)];  // X, Y
bat_x1     = cut_x - drum_ro - 2;          // cradle front edge
bat_x0     = bat_x1 - cradle[0];
bat_pos    = [(bat_x0 + bat_x1) / 2, 0];
bat_bot    = floor_top + bat_boss_h + 3;
bat_top    = bat_bot + bat_size[2];
deck_z0    = floor_top + 6 + deck_standoff;   // deck underside
deck_front = [axle_x[0] - 37, axle_x[0] + 37];
deck_rear  = [axle_x[1] - 43, axle_x[1] + 37];
deck_legs_f = [for (dx = [-32, 32], s = [-1, 1]) [axle_x[0] + dx, s * 112]];
deck_legs_r = [for (dx = [-32, 32], s = [-1, 1]) [axle_x[1] + dx, s * 112]];
parts_top  = deck_z0 + 3 + board_standoff + 22;   // tallest board (MP2) on a deck

// ---------- lid ----------
lid_in_top = max(bat_top, tower_top, parts_top) + lid_headroom;
lid_top    = lid_in_top + lid_t;
lid_bot    = tub_top - lid_overlap;
lid_off    = lid_clear + lid_wall;
lip_w      = 2.6;
lip_h      = 2.6;
tof_z      = floor_top + tof_z_off;

// ---------- ToF: [x, y on the inner wall face, angle, type] ----------
// type 0 = SATEL-VL53L8 breakout, 1 = STEVAL-VL53L9 board
tof_pcbs  = [[25, 20], [40, 30]];                   // [EST] (y, z)
tof_holes = [[[-10, 7], [10, 7], [-10, -7], [10, -7]],
             [[-16, 11], [16, 11], [-16, -11], [16, -11]]];   // [EST] M2
tof_wins  = [[20, 14], [24, 18]];
xi = body_L / 2 - tub_wall;
yi = body_W / 2 - tub_wall;
tof_spots = [[ xi,  30,   0, 1], [ xi, -25,   0, 0], [-xi, -30, 180, 0],
             [side_tof_x, yi, 90, 0], [side_tof_x, -yi, 270, 0]];

// ---------- camera pod ----------
cam_pcb = [30, 45];        // [EST] B-CAMS-IMX MB1854 board (width, height)
cam_lens_len = 20;         // [EST] M12 lens + holder height above the board
cam_lens_z = 10;           // [EST] lens centre above the board centre
cam_tof_z  = -13;          // [EST] ToF sensor below the board centre
cam_in   = [cam_lens_len + 8, cam_pcb[0] + 4, cam_pcb[1] + 4];   // inside x, y, z (top open)
head_o   = [cam_in[0] + 5.4, cam_in[1] + 6, cam_in[2] + 3];
head_c   = [head_o[0] / 2 - 4, 0, 4 + cam_mast_h + head_o[2] / 2];
cam_foot_holes = [for (sx = [-1, 1], sy = [-1, 1]) [sx * 15, sy * 13]];

// ---------- lidar (LDRobot LD19 drawing: 54 x 46.29 x 34.8) ----------
ld19_holes = [[-23.4, -8.4], [23.4, -8.4], [0, 23.5]];   // [web] 3 x M2.5 ears, scan centre at 0
ld19_plane = 28.5;          // [EST] laser plane above the base
lidar_ang  = -90;           // connector faces rearward
lidar_foot_holes = [for (sx = [-1, 1], sy = [-1, 1]) [sx * 18, sy * 18]];

// ---------- helpers ----------
function rot2(p, a) = [p[0] * cos(a) - p[1] * sin(a), p[0] * sin(a) + p[1] * cos(a)];
function corners(c, sz, inset) = [for (sx = [-1, 1], sy = [-1, 1]) [c[0] + sx * (sz[0] / 2 - inset), c[1] + sy * (sz[1] / 2 - inset)]];
function place(org, ang, list) = [for (p = list) org + rot2(p, ang)];
function sum_mass(t, i = 0) = i < len(t) ? t[i][1] * t[i][2] + sum_mass(t, i + 1) : 0;
function sum_moment(t, i = 0) = i < len(t) ? t[i][1] * t[i][2] * t[i][3] + sum_moment(t, i + 1) : [0, 0, 0];
function r1(x) = round(x * 10) / 10;
function fits(a, b) = (a <= bed_x && b <= bed_y) || (a <= bed_y && b <= bed_x);
function tof_boss_xy(t) = [[-tof_depth + 7, -(tof_pcbs[t][0] / 2 - 2)], [-tof_depth + 7, tof_pcbs[t][0] / 2 - 2]];

// ---------- board positions ----------
g4_org  = [deck_front[0], 32];                 // not turned
n6_org  = [deck_front[0] + nuc_w, 18];         // turned 180 deg: camera connector near the front
rbt_pos = [-163, -62];                         // turned 90 deg
g4_bosses  = place(g4_org, 0, g4_holes);
n6_bosses  = place(n6_org, 180, nuc_holes);
mp2_bosses = place(mp2_pos, 90, mp2_holes);
rbt_bosses = place(rbt_pos, 90, corners([0, 0], rbt_size, rbt_inset));

// ---------- floor bosses: [x, y, height, insert from below?, diameter] ----------
skirt_holes = [for (k = [0 : 7]) let (a = 22.5 + 45 * k) [cut_x + skirt_hole_r * cos(a), skirt_hole_r * sin(a)]];
tower_lugs  = [for (a = tower_lug_angles) [cut_x + tower_bolt_r * cos(a), tower_bolt_r * sin(a)]];
tray_holes  = [for (m = motor_spots, sx = [-1, 1], yy = tray_screw_y) [m[0] + sx * tray_screw_x, m[1] * yy]];
tof_bosses  = [for (t = tof_spots, b = tof_boss_xy(t[3])) [t[0], t[1]] + rot2(b, t[2])];
bat_bosses  = corners(bat_pos, cradle, 6);
pwr_bosses  = concat(corners(pwrA_pos, pwr_size, 5), corners(pwrB_pos, pwr_size, 5));
esc_bosses  = corners(esc_pos, esc_plate, 5);
leg_bosses  = concat(deck_legs_f, deck_legs_r);
// bumper pins: points on the outer wall [x, y, z, angle]; switch brackets behind the outer pins
bump_z      = floor_top + 13;
bump_sleeves = [for (sx = [-1, 1], sy = [-1, 1], p = bump_pins) [sx * body_L / 2, sy * p, bump_z, sx > 0 ? 0 : 180]];
// [x, y, angle, k]: k = -1 mirrors the bracket so its bosses face the centre line
bsw_spots   = [for (sx = [-1, 1], sy = [-1, 1]) [sx * (xi - 12 - 4), sy * bump_pins[1], sx > 0 ? 0 : 180, sx * sy]];
bsw_bosses  = [for (b = bsw_spots, dx = [-14, -4]) [b[0], b[1]] + rot2([dx, -9 * b[3]], b[2])];

floor_bosses = concat(
    [for (p = bat_bosses)  [p[0], p[1], bat_boss_h, 0, 8]],
    [for (p = pwr_bosses)  [p[0], p[1], plate_boss_h, 0, 8]],
    [for (p = esc_bosses)  [p[0], p[1], plate_boss_h, 0, 8]],
    [for (p = tof_bosses)  [p[0], p[1], 5, 0, 8]],
    [for (p = leg_bosses)  [p[0], p[1], 6, 0, 10]],
    [for (p = bsw_bosses)  [p[0], p[1], 4, 0, 8]],
    [for (p = skirt_holes) [p[0], p[1], 6, 1, 8]]);

// ---------- corner columns: [x, y, 0 = magnet / 1 = Hall sensor] ----------
lid_columns = [for (sx = [-1, 1], sy = [-1, 1])
                   [sx * (body_L / 2 - body_r + 9.2), sy * (body_W / 2 - body_r + 9.2), sx == sy ? 1 : 0]];

tof_windows = [for (t = tof_spots) [t[0] + tub_wall * cos(t[2]), t[1] + tub_wall * sin(t[2]), tof_z, t[2],
                                     tof_wins[t[3]][0], tof_wins[t[3]][1]]];
port_usb    = [-body_L / 2,  25, floor_top + 25, 180];   // USB-C panel extension [EST]
port_charge = [-body_L / 2,  50, floor_top + 25, 180];   // XT60 panel mount [EST]
port_switch = [-body_L / 2, -92, floor_top + 27, 180];   // 30 A rocker, 22 x 30 cutout [EST]

// ---------- lid bosses: [x, y, diameter, height] hanging under the roof ----------
vent_screws  = [for (v = vent_pos, a = [90, 210, 330]) v + [26 * cos(a), 26 * sin(a)]];
cam_screws   = [for (h = cam_foot_holes) [cam_x + h[0], h[1]]];
lidar_screws = [for (h = lidar_foot_holes) lidar_pos + h];
rain_screws  = INCLUDE_RAIN_SENSOR ? [rain_pos + [-22, 0], rain_pos + [22, 0]] : [];
lid_inserts  = concat(vent_screws, cam_screws, lidar_screws, rain_screws);
lid_bosses   = concat([[estop_pos[0], estop_pos[1], 34, 3]],
                      [for (p = lid_inserts) [p[0], p[1], 8, 6]]);
// fender screws into the lid skirt: [x, y on the outer lid face, z, angle]
fender_z = tub_top + 5;
lid_wall_bosses = INCLUDE_FENDERS ?
    [for (w = wheel_pos, dx = [-20, 20]) [w[0] + dx, sign(w[1]) * (body_W / 2 + lid_off), fender_z, w[1] > 0 ? 90 : 270]] : [];

// ---------- joint rib bolts ----------
tub_xrib_bolts = concat([for (y = [-115, -75, 75, 115]) [y, floor_top + rib_w / 2]],
                        [for (s = [-1, 1], z = [floor_top + 20, floor_top + 50]) [s * (yi - rib_w / 2), z]]);
tub_yrib_bolts = concat([for (x = [70, 105, 160, 205, -160, -205]) [x, floor_top + rib_w / 2]],
                        [for (s = [-1, 1], z = [floor_top + 20, floor_top + 50]) [s * (xi - rib_w / 2), z]]);
lid_xrib_bolts = concat([for (y = [-115, -75, -35, 35, 75, 115]) [y, lid_in_top - rib_w / 2]],
                        [for (s = [-1, 1]) [s * (body_W / 2 + lid_clear - rib_w / 2), (tub_top + lip_h + lid_in_top) / 2]]);
lid_yrib_bolts = concat([for (x = [40, 100, 150, -165, -200]) [x, lid_in_top - rib_w / 2]],
                        [for (s = [-1, 1]) [s * (body_L / 2 + lid_clear - rib_w / 2), (tub_top + lip_h + lid_in_top) / 2]]);

// ---------- printed-part masses (g) ----------
rho        = print_rho / 1000;
perim      = 2 * (body_L + body_W) - (8 - 2 * PI) * body_r;
floor_area = body_L * body_W - (4 - PI) * body_r * body_r - PI * pow(cut_hole_d / 2, 2)
             - 4 * 2 * slot_hw * (dm_face_y - dm_end_y + 2);
tub_mass   = rho * (floor_area * floor_t + perim * (tub_h - floor_t) * tub_wall) + 165;  // + ribs, pads, columns, bosses (STL: 833 g)
lid_mass   = rho * ((body_L + 2 * lid_off) * (body_W + 2 * lid_off) * lid_t
                    + (perim + 2 * PI * lid_off) * (lid_top - lid_bot) * lid_wall) + 50;
tray_mass  = rho * 2 * tray_hw * (tray_y1 - tray_y0) * (deck_z - tray_bot) * 0.45;
clamp_mass = rho * 16 * (PI * (pow(dm_d / 2 + 4, 2) - pow(dm_d / 2, 2)) / 2 + 2 * (slot_hw + 13) * 5);
tower_mass = rho * (2 * PI * drum_ro * 2.4 * (drum_top - floor_top) + PI * (drum_ro + col_ro) * 2.4 * (drum_ro - col_ro) * 1.41
                    + 2 * PI * col_ro * 2.4 * (cm_base - cone_top) + PI * col_ro * col_ro * 5) + 12;
skirt_mass = rho * (PI * (pow(guard_r_out, 2) - pow(guard_r_in, 2)) * (deck_z - skirt_bot)
                    + PI * (pow(guard_r_in, 2) - pow(guard_r_in - skirt_flange, 2)) * 3);
fender_mass = rho * (PI * (pow(74.4, 2) - pow(72, 2)) * 120 / 360 * (wheel_gap + wheel_w + 5 - lid_off - 1) + 64 * 25 * 2);
deck_mass  = rho * ((deck_front[1] - deck_front[0] + deck_rear[1] - deck_rear[0]) * 2 * deck_hw * 3 * 0.8) + 30;
bump_mass  = rho * (bump_reach * bump_h * bump_t + 2 * PI * 16 * 30);   // 6 mm prints solid with 5 walls

// ---------- weight budget: [name, qty, grams each, [x, y, z] of the group CG] ----------
cam_cg   = [cam_x + head_c[0], 0, lid_top + head_c[2]];
lidar_cg = [lidar_pos[0], lidar_pos[1], lid_top + lidar_mast_h + 20];
deck_f_z = deck_z0 + 3 + board_standoff + 5;
PARTS = [
    // compute and sensors
    ["STM32MP257F-DK + microSD [EST]",          1, 110, [deck_rear[0] + 40, 62, deck_f_z]],
    ["NUCLEO-N657X0-Q [EST]",                   1, 55,  [deck_front[0] + 35, -48, deck_f_z]],
    ["NUCLEO-G474RE [EST]",                     1, 35,  [deck_front[0] + 35, 73, deck_f_z]],
    ["X-NUCLEO-67W61M1 Wi-Fi shield [EST]",     1, 15,  [deck_front[0] + 35, 73, deck_f_z + 12]],
    ["X-STM32MP-RBT01 motor board [EST]",       1, 45,  [rbt_pos[0], rbt_pos[1], deck_f_z]],
    ["B-G431B-ESC1 [ST] + heatsink + 30 mm fan [EST]", 1, 9.2 + 25, [esc_pos[0], esc_pos[1], floor_top + plate_boss_h + 15]],
    ["B-CAMS-IMX camera + FFC [EST]",           1, 15,  cam_cg],
    ["LD19 lidar [EST]",                        1, 42,  lidar_cg],
    ["STEVAL-VL53L9 front ToF [EST]",           1, 10,  [xi - 4, 30, tof_z]],
    ["SATEL-VL53L8 ToF breakout [EST]",         4, 4,   [0, -10, tof_z]],
    ["X-NUCLEO-GNSS1A1 [EST]", INCLUDE_GNSS ? 1 : 0, 12, [deck_front[0] + 35, -48, deck_f_z + 15]],
    ["FC-37 rain sensor [EST]", INCLUDE_RAIN_SENSOR ? 1 : 0, 8, [rain_pos[0], rain_pos[1], lid_top + 3]],
    ["INA226 module [EST]",                     1, 3,   [pwrA_pos[0], pwrA_pos[1], floor_top + 15]],
    ["Micro switch + spring [EST]",             4, 4,   [0, 0, bump_z]],
    ["Hall sensor + 6 x 3 magnets [EST]",       1, 8,   [0, 0, tub_top]],
    // power
    ["4S4P 14.8 V 10 Ah pack [web]",            1, bat_g, [bat_pos[0], bat_pos[1], bat_bot + bat_size[2] / 2]],
    ["Relay, fuses, XT90-S, XT60, caps, TVS [EST]", 1, 110, [pwrA_pos[0], pwrA_pos[1], floor_top + plate_boss_h + 15]],
    ["D36V50F5 [web] + USB-C buck [EST]",       1, 7 + 15, [pwrB_pos[0], pwrB_pos[1], floor_top + plate_boss_h + 10]],
    ["Main switch [EST]",                       1, 15,  [-body_L / 2 + 10, port_switch[1], port_switch[2]]],
    ["E-stop [EST]",                            1, 30,  [estop_pos[0], estop_pos[1], lid_top - 10]],
    ["Wiring 12/18 AWG, signal, star bus [EST]", 1, 170, [0, 0, floor_top + 15]],
    // drive
    ["Wheel 130 x 60 [web]",                    4, wheel_g, [0, 0, axle_z]],
    ["36 mm gearmotor + encoder [web/EST]",     4, dm_g,    [0, 0, axle_z]],
    ["Hex coupling [EST]",                      4, 15,      [0, 0, axle_z]],
    // cutter
    ["DYS D3536-8 1000 KV [EST]",               1, cm_g,    [cut_x, 0, bell_bot + cm_len / 2]],
    ["Spool + 1.6 mm line [EST]",               1, spool_g, [cut_x, 0, spool_bot + spool_h / 2]],
    ["Aluminium spool hub [EST]",               1, 15,      [cut_x, 0, spool_top + adapter_len / 2]],
    // printed parts
    ["Tub, 4 quarters",                         1, tub_mass,   [0, 0, deck_z + tub_h * 0.3]],
    ["Lid, 4 quarters",                         1, lid_mass,   [0, 0, lid_top - (lid_top - lid_bot) * 0.3]],
    ["Motor tray",                              4, tray_mass,  [0, 0, (tray_bot + deck_z) / 2]],
    ["Motor clamp",                             4, clamp_mass, [0, 0, axle_z + 8]],
    ["Cutter tower",                            1, tower_mass, [cut_x, 0, (floor_top + tower_top) / 2]],
    ["Guard skirt, 2 halves",                   1, skirt_mass, [cut_x, 0, (skirt_bot + deck_z) / 2]],
    ["Wheel fender",   INCLUDE_FENDERS ? 4 : 0, fender_mass, [0, 0, axle_z + 60]],
    ["Board decks (2) + M3 x 35 standoffs (8)", 1, deck_mass + 8 * 4, [-20, 0, deck_z0]],
    ["Battery cradle",                          1, 45, [bat_pos[0], bat_pos[1], bat_bot]],
    ["Power plates (2) + ESC mount",            1, 70, [-30, 0, floor_top + plate_boss_h + 2]],
    ["ToF holders",                             5, 7,  [0, 0, tof_z - 8]],
    ["Bumper half + pins",                      4, bump_mass, [0, 0, bump_z]],
    ["Bumper switch bracket",                   4, 4,  [0, 0, bump_z]],
    ["Camera pod + cover",                      1, 53, [cam_x, 0, lid_top + head_c[2] * 0.6]],
    ["Lidar mast",                              1, 40, [lidar_pos[0], lidar_pos[1], lid_top + lidar_mast_h / 2]],
    ["Vent base + cap",                         2, 16, [0, 90, lid_top + 8]],
    ["Screws, inserts, nuts, springs [EST]",    1, 140, [0, 0, floor_top + 15]]
];

// =====================================================================
//  Assembly
// =====================================================================
c_tub = "SlateGray";
c_lid = "OliveDrab";
c_prt = "Orange";

module assembly() {
    lift = VIEW == "lid_lifted" ? 170 : 0;

    // body
    color(c_tub) for (sx = [-1, 1], sy = [-1, 1]) tub_quarter(sx, sy);
    if (VIEW != "no_lid") translate([0, 0, lift]) {
        color(c_lid) for (sx = [-1, 1], sy = [-1, 1]) lid_quarter(sx, sy);
        color(c_prt) for (v = vent_pos) translate([v[0], v[1], lid_top]) {
            vent_base();
            translate([0, 0, vent_cap_z]) mirror([0, 0, 1]) vent_cap();
        }
        translate([cam_x, 0, lid_top]) { color(c_prt) camera_pod(); color("Gold") camera_pod_cover_placed(); camera_board(); }
        translate([lidar_pos[0], lidar_pos[1], lid_top]) {
            color(c_prt) lidar_mast();
            translate([0, 0, lidar_mast_h + 4]) rotate(lidar_ang) ld19();
        }
        translate([estop_pos[0], estop_pos[1], lid_top]) estop();
        if (INCLUDE_RAIN_SENSOR) translate([rain_pos[0], rain_pos[1], lid_top]) rain_sensor();
        if (INCLUDE_FENDERS) for (w = wheel_pos) let (s = sign(w[1]))
            translate([w[0], s * (body_W / 2 + lid_off), axle_z]) rotate(s > 0 ? 0 : 180) color(c_prt) wheel_fender();
    }

    // wheels, motors, trays, clamps
    for (i = [0 : 3]) let (w = wheel_pos[i], s = sign(w[1])) {
        translate([w[0], w[1], axle_z]) treaded_wheel();
        translate([w[0], s * dm_face_y, axle_z]) rotate([-s * 90, 0, 0]) gearmotor36();
        translate([w[0], 0, 0]) mirror([0, s > 0 ? 0 : 1, 0]) color(c_prt) {
            motor_tray();
            translate([0, clamp_y, 0]) motor_clamp();
        }
    }

    // cutter
    translate([cut_x, 0, 0]) {
        color(c_prt) translate([0, 0, floor_top]) cutter_tower();
        color("DarkOrange") guard_skirt(0);
        cutter_motor_and_spool();
    }

    // boards on the decks
    color(c_prt) { translate([0, 0, deck_z0]) deck_front(); translate([0, 0, deck_z0]) deck_rear(); }
    for (p = leg_bosses) color("Gold") translate([p[0], p[1], floor_top + 6]) cylinder(d = 5.5, h = deck_standoff, $fn = 6);
    bz = deck_z0 + 3 + board_standoff;
    translate([g4_org[0], g4_org[1], bz]) nucleo64_g474();
    translate([n6_org[0], n6_org[1], bz]) rotate(180) nucleo_n6();
    translate([mp2_pos[0], mp2_pos[1], bz]) rotate(90) mp2_dk();
    translate([rbt_pos[0], rbt_pos[1], bz]) rotate(90) rbt01();

    // power and cutter drive
    translate([bat_pos[0], bat_pos[1], floor_top + bat_boss_h]) {
        color(c_prt) battery_cradle();
        translate([0, 0, 3]) battery_pack();
    }
    for (p = [pwrA_pos, pwrB_pos]) translate([p[0], p[1], floor_top + plate_boss_h]) color(c_prt) mount_plate(pwr_size);
    translate([pwrA_pos[0], pwrA_pos[1], floor_top + plate_boss_h + 3]) power_parts_a();
    translate([pwrB_pos[0], pwrB_pos[1], floor_top + plate_boss_h + 3]) power_parts_b();
    translate([esc_pos[0], esc_pos[1], floor_top + plate_boss_h]) { color(c_prt) esc_mount(); esc_block(); }
    wall_ports();

    // ToF, lid columns, bumpers
    for (t = tof_spots) translate([t[0], t[1], floor_top]) rotate(t[2]) {
        color(c_prt) tof_holder(t[3]);
        tof_board(t[3]);
    }
    for (b = bump_sleeves) if (abs(b[1]) == bump_pins[0])
        translate([b[0], 0, b[2]]) rotate(b[3]) mirror([0, b[1] * cos(b[3]) > 0 ? 0 : 1, 0])
            translate([bump_gap, 0, 0]) color("DimGray") bumper_half();
    for (b = bsw_spots) translate([b[0], b[1], floor_top + 4]) rotate(b[2]) mirror([0, b[3] > 0 ? 0 : 1, 0]) {
        color(c_prt) bumper_switch_bracket();
        micro_switch_placed();
    }
}

// =====================================================================
//  Report (console)
// =====================================================================
M_total = sum_mass(PARTS);
CG      = sum_moment(PARTS) / M_total;
f_front = (wheelbase / 2 + CG[0]) / wheelbase;
f_left  = (track / 2 + CG[1]) / track;
r_m     = wheel_d / 2000;
F_slope = M_total / 1000 * (9.81 * (crr * cos(slope) + sin(slope)) + accel);
T_line  = F_slope / mec_eff / 4 * r_m;              // N.m per wheel, driving straight up the slope
T_turn  = T_line * skid;                            // skid-steering on the slope
F_flat  = M_total / 1000 * 9.81 * crr;
T_flat  = F_flat / mec_eff / 4 * r_m;
kgcm    = 10.197;
function motor_A(T) = dm_noload_A + (dm_stall_A - dm_noload_A) * (T * kgcm / dm_stall_kgcm);
rpm_need = v_mow / (PI * wheel_d / 1000) * 60;
v_top   = dm_rpm / 60 * PI * wheel_d / 1000;
pack_Wh = bat_V * bat_Ah;
P_total = P_cut + P_drive + P_mp2 + P_n6 + P_g4 + P_lidar + P_misc;
quarter_xy = [body_L / 2, body_W / 2];
lid_q_xy   = [body_L / 2 + lid_off, body_W / 2 + lid_off];
lidar_plane = lid_top + lidar_mast_h + 4 + ld19_plane;
cam_top    = lid_top + head_c[2] + (head_o[2] / 2) * cos(cam_tilt) + (head_o[0] / 2) * sin(cam_tilt) + 2;
tray_inner = axle_x[0] - tray_hw;                   // front tray edge facing the cutter

// pairs of floor items closer than their radii + 1 mm (screw heads count as 6 mm)
clash_items = concat(floor_bosses,
                     [for (p = tray_holes) [p[0], p[1], 0, 0, 6]],
                     [for (p = tower_lugs) [p[0], p[1], 0, 0, 6]]);
function boss_clash(b, i = 0, j = 1) =
    i >= len(b) - 1 ? [] :
    j >= len(b) ? boss_clash(b, i + 1, i + 2) :
    concat(norm([b[i][0] - b[j][0], b[i][1] - b[j][1]]) < (b[i][4] + b[j][4]) / 2 + 1 ? [[r1(b[i][0]), r1(b[i][1])]] : [],
           boss_clash(b, i, j + 1));
clash = boss_clash(clash_items);
deck_clash = concat(boss_clash(concat([for (p = concat(g4_bosses, n6_bosses)) [p[0], p[1], 0, 0, 7]], [for (p = deck_legs_f) [p[0], p[1], 0, 0, 10]])),
                    boss_clash(concat([for (p = concat(mp2_bosses, rbt_bosses)) [p[0], p[1], 0, 0, 7]], [for (p = deck_legs_r) [p[0], p[1], 0, 0, 10]])));
// floor bosses that touch the inside of a wall (CGAL then makes a non-manifold edge)
wall_touch = [for (b = floor_bosses)
                  if (abs(b[1]) + b[4] / 2 > yi - 1 || abs(b[0]) + b[4] / 2 > xi - 1)
                      [r1(b[0]), r1(b[1])]];

module report() {
    echo("=================== GRASSKIT ROVER v3 - WEIGHT BUDGET ===================");
    for (p = PARTS) if (p[1] > 0) echo(str(p[1], " x ", p[0], "  ", r1(p[2]), " g  = ", r1(p[1] * p[2]), " g"));
    echo(str("TOTAL MASS        : ", round(M_total), " g  (", r1(M_total / 100) / 10, " kg)"));
    echo(str("Centre of gravity : x ", r1(CG[0]), "  y ", r1(CG[1]), "  z ", r1(CG[2]), " mm"));
    echo(str("Wheel load (g)    : FL ", round(M_total * f_front * f_left), "  FR ", round(M_total * f_front * (1 - f_left)),
             "  RL ", round(M_total * (1 - f_front) * f_left), "  RR ", round(M_total * (1 - f_front) * (1 - f_left))));
    echo(str("Tip-over angle    : sideways ", round(atan((track / 2 - abs(CG[1])) / CG[2])), " deg, fore/aft ",
             round(atan((wheelbase / 2 - abs(CG[0])) / CG[2])), " deg"));
    echo("--------------------------- DRIVE ---------------------------");
    echo(str("Flat, straight    : ", r1(T_flat * kgcm), " kg.cm per wheel, about ", r1(motor_A(T_flat)), " A per motor"));
    echo(str(slope, " deg slope, Crr ", crr, " : ", r1(T_line * kgcm), " kg.cm per wheel, about ", r1(motor_A(T_line)), " A per motor"));
    echo(str("Skid turn on it   : ", r1(T_turn * kgcm), " kg.cm per wheel, about ", r1(motor_A(T_turn)), " A per motor (RBT01 limit ", rbt_limit_A, " A)"));
    echo(str("Wheel rpm for ", v_mow, " m/s : ", round(rpm_need), " rpm (motor ", dm_rpm, " rpm at 12 V = ", r1(v_top * 100) / 100, " m/s top)"));
    echo("--------------------------- BATTERY ---------------------------");
    echo(str("Pack              : ", bat_V, " V ", bat_Ah, " Ah = ", r1(pack_Wh), " Wh"));
    echo(str("Load while mowing : ", P_total, " W -> ", r1(P_total / bat_V), " A (cutter alone ", r1(P_cut / bat_V), " A)"));
    echo(str("Runtime (80 % DoD): ", round(pack_Wh * 0.8 / P_total * 60), " min"));
    echo("--------------------------- GEOMETRY ---------------------------");
    echo(str("Footprint         : ", body_L + 2 * (bump_gap + bump_t), " x ", track + wheel_w,
             " mm (body ", body_L, " x ", body_W, "), lid top ", round(lid_top), " mm"));
    echo(str("Lidar plane       : ", round(lidar_plane), " mm; camera pod top ~", round(cam_top), " mm"));
    echo(str("Cut height ", cut_height, " mm, swath ", cut_d, " mm, line to floor ", line_floor_gap,
             " mm, ground clearance: floor ", deck_z, " mm, motor trays ", tray_bot, " mm"));
    echo("--------------------------- PRINT BED ---------------------------");
    echo(str("Tub quarter ", quarter_xy[0], " x ", quarter_xy[1], " x ", tub_h, " mm : ",
             fits(quarter_xy[0], quarter_xy[1]) ? "fits" : "DOES NOT FIT", " a ", bed_x, " x ", bed_y, " bed"));
    echo(str("Lid quarter ", r1(lid_q_xy[0]), " x ", r1(lid_q_xy[1]), " x ", r1(lid_top - lid_bot), " mm : ",
             fits(lid_q_xy[0], lid_q_xy[1]) ? "fits" : "DOES NOT FIT"));
    echo(str("Board decks ", deck_front[1] - deck_front[0], " and ", deck_rear[1] - deck_rear[0], " x ", 2 * deck_hw, " mm : ",
             fits(deck_rear[1] - deck_rear[0], 2 * deck_hw) ? "fit" : "DO NOT FIT"));
    echo(str("Guard skirt half ", 2 * guard_r_out, " x ", guard_r_out, " mm : ",
             fits(2 * guard_r_out, guard_r_out) ? "fits" : "DOES NOT FIT"));
    echo(str("Bumper half ", bump_reach, " x ", bump_h, " mm : ", fits(bump_reach, bump_h) ? "fits" : "DOES NOT FIT"));
    if (len(clash) > 0) echo(str("WARNING: floor bosses / screws overlap at ", clash));
    if (len(deck_clash) > 0) echo(str("WARNING: deck bosses overlap at ", deck_clash));
    if (len(wall_touch) > 0) echo(str("WARNING: floor bosses touch a wall at ", wall_touch));
    if (tray_inner < cut_x + guard_r_out + 2) echo("WARNING: the front motor trays hit the guard skirt - raise wheelbase.");
    if (axle_x[1] + tray_hw > cut_x - guard_r_out - 2) echo("WARNING: the rear motor trays hit the guard skirt.");
    if (bat_x0 < axle_x[1] + dm_d / 2 + 1) echo("WARNING: the battery cradle hits the rear motors.");
    if (dm_end_y - 2 < rib_t + 2) echo("WARNING: the motor ends reach the y = 0 joint rib - raise body_W.");
    if (lidar_plane < cam_top + 5) echo("WARNING: the camera pod shadows the lidar - raise lidar_mast_h.");
    if (spool_top > drum_top - 3) echo("WARNING: the spool reaches the cone of the cutter tower.");
    if (drum_ri - (bell_bot - (drum_top - 1)) < cm_d / 2 + 2) echo("WARNING: the cutter bell touches the tower cone.");
    if (tof_z + tof_pcbs[1][1] / 2 + 2 > deck_z0 - 1) echo("WARNING: the front ToF holder hits the board deck.");
    if (deck_z0 < axle_z + dm_d / 2 + 4) echo("WARNING: the board decks sit on the motors.");
    if (skirt_bot < 8) echo("WARNING: guard skirt drags on the ground.");
}

// =====================================================================
//  Render
// =====================================================================
report();

if (MODE == "assembly") assembly();
else if (MODE == "tub_front_left")   translate([0, 0, -deck_z]) tub_quarter(1, 1);           // floor on the bed
else if (MODE == "tub_front_right")  translate([0, 0, -deck_z]) tub_quarter(1, -1);
else if (MODE == "tub_rear_left")    translate([0, 0, -deck_z]) tub_quarter(-1, 1);
else if (MODE == "tub_rear_right")   translate([0, 0, -deck_z]) tub_quarter(-1, -1);
else if (MODE == "lid_front_left")   translate([0, 0, lid_top]) mirror([0, 0, 1]) lid_quarter(1, 1);   // roof on the bed
else if (MODE == "lid_front_right")  translate([0, 0, lid_top]) mirror([0, 0, 1]) lid_quarter(1, -1);
else if (MODE == "lid_rear_left")    translate([0, 0, lid_top]) mirror([0, 0, 1]) lid_quarter(-1, 1);
else if (MODE == "lid_rear_right")   translate([0, 0, lid_top]) mirror([0, 0, 1]) lid_quarter(-1, -1);
else if (MODE == "motor_tray")       translate([0, 0, deck_z]) mirror([0, 0, 1]) motor_tray();       // print 4, top face down
else if (MODE == "motor_clamp")      translate([0, 0, 8]) rotate([90, 0, 0]) translate([0, 0, -floor_top]) motor_clamp();   // print 4, on its side
else if (MODE == "cutter_tower")     cutter_tower();
else if (MODE == "guard_skirt_half") translate([0, 0, deck_z]) mirror([0, 0, 1]) guard_skirt(1);    // print 2
else if (MODE == "wheel_fender")     rotate([90, 0, 0]) wheel_fender();                             // print 4, web on the bed
else if (MODE == "deck_front")       translate([-axle_x[0], 0, 0]) deck_front();
else if (MODE == "deck_rear")        translate([-axle_x[1], 0, 0]) deck_rear();
else if (MODE == "battery_cradle")   battery_cradle();
else if (MODE == "power_plate")      mount_plate(pwr_size);                                         // print 2
else if (MODE == "esc_mount")        esc_mount();
else if (MODE == "tof_holder_l8")    translate([0, 0, -5]) tof_holder(0);                           // print 4
else if (MODE == "tof_holder_l9")    translate([0, 0, -5]) tof_holder(1);
else if (MODE == "bumper_half")      translate([0, 0, bump_t]) rotate([0, 90, 0]) bumper_half();    // print 4, face down
else if (MODE == "bumper_switch_bracket") bumper_switch_bracket();                                  // print 4
else if (MODE == "camera_pod")       translate([0, 0, head_o[1] / 2]) rotate([90, 0, 0]) camera_pod();   // on its side
else if (MODE == "camera_pod_cover") camera_pod_cover();
else if (MODE == "lidar_mast")       lidar_mast();
else if (MODE == "vent_base")        vent_base();                                                   // print 2
else if (MODE == "vent_cap")         vent_cap();                                                    // print 2, roof on the bed

include <rover_printed_parts.scad>
include <rover_bought_parts.scad>
