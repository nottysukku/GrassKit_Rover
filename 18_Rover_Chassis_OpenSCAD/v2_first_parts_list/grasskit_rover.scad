// =====================================================================
//  GrassKit Rover v2 - enclosed, 3D-printable body
//  OpenSCAD 2021.01 or newer.   Units: mm and grams.
//
//  Files:  grasskit_rover.scad        this file: settings, layout, report
//          rover_printed_parts.scad   every printed part
//          rover_bought_parts.scad    stand-ins for boards, motors, cells
//          grasskit_rover_v1_open_frame.scad   the earlier open-frame design
//
//  Frame: origin on the ground under the middle of the rover.
//         +X = forward, +Y = left, +Z = up.
//
//  MODE = "assembly" -> the whole rover, plus a weight / CG / wheel load /
//                       torque / battery / print-bed report in the console.
//  Any other MODE    -> one printed part, already in its print orientation.
//                       F6, then File > Export > STL.
//
//  [ST]  value from the ST documents in documentation/
//  [EST] estimate: measure or weigh your part and change the number.
//
//  ---------------------------------------------------------------------
//  BODY
//  ---------------------------------------------------------------------
//   Tub (floor + walls) and lid each print as a front half and a rear
//   half (about 173 x 245 mm, fits a 250 x 210 bed). The halves bolt
//   together through internal ribs. Every board, the battery and the
//   motors fix to bosses with M3 heat-set inserts (4.0 mm hole).
//   The lid sits on the tub rim and is held by 12 side screws.
//   The wheel motors sit in closed pods under the floor; their wires
//   come up through the pod into the tub, so the floor has no open holes.
//
//  ---------------------------------------------------------------------
//  POWER: one 4S 18650 pack runs everything
//  ---------------------------------------------------------------------
//   4S 12.0-16.8 V - main switch - E-stop -+- 20 A fuse - cutter driver - cutter motor
//                                          +- 10 A fuse - RBT01 VS (5.2-48 V) - 4 wheel motors
//                                          +-  3 A fuse - 5 V / 3 A buck with USB-C output
//                                                         - NUCLEO-N657X0-Q user USB-C (CN8)
//   * CN9 jumper on [3-4] (5V_USB_SNK, up to 1.5 A) [ST UM3417 7.4.1].
//     Power the board first, then plug the PC into the ST-LINK port CN10.
//   * Never wire the pack to VIN: VIN takes 7-12 V and only 250-800 mA
//     [ST UM3417 table 6]. A full 4S pack is 16.8 V.
//   * Level shifters convert 3.3 V / 5 V signals, not power.
//   * The GIGAWATTS 250 W trimmer motor is 230 V AC. Keep its spool and
//     1 mm line, spin them with a 12-18 V DC motor (CUT_MOTOR = "775_DC").
//   * 1000 uF low-ESR capacitor at the cutter driver; soft-start the cutter.
//   * The 4 x AA pack is gone: it has no job on this power plan.
//
//  CAMERA: the N6 board has a 22-pin MIPI CSI-2 camera connector (CN6) for
//  Raspberry Pi Zero style camera modules [ST UM3417 8.3]. A ribbon-cable
//  camera replaces the USB camera. CN8 is the power input, so a USB camera
//  would need the board powered from VIN instead (CAMERA = "USB_UVC").
// =====================================================================

/* [Render] */
MODE = "assembly"; // [assembly, tub_front_half, tub_rear_half, lid_front_half, lid_rear_half, motor_pod, cutter_tower, guard_skirt_half, wheel_fender, tof_holder, camera_pod, camera_pod_cover, vent_base, vent_cap, battery_cradle, power_plate, driver_plate]
VIEW = "closed"; // [closed, lid_lifted, no_lid]
CUT_MOTOR = "775_DC"; // [775_DC, TRIMMER_AC_250W]
CAMERA = "MIPI_CSI"; // [MIPI_CSI, USB_UVC]
INCLUDE_FENDERS = true;
SHOW_CUT_DISC = true;     // translucent disc = area the trimmer line sweeps

/* [Printer] */
bed_x = 250;              // Prusa MK4 250 x 210, Bambu X1/P1/A1 256 x 256
bed_y = 210;
print_rho = 1.27;         // g/cm3: PETG 1.27, ASA 1.07, PLA 1.24
insert_d = 4.0;           // hole for M3 heat-set inserts
insert_depth = 6;

/* [Wheels and drive] */
wheel_d   = 75;    // mecanum wheel diameter
wheel_w   = 35;    // mecanum wheel width
wheel_g   = 75;    // [EST] grams per wheel
wheelbase = 250;   // front axle to rear axle (X)
wheel_gap = 5;     // body side wall to wheel inner face
dm_d      = 25;    // drive gearmotor diameter (JGA25-370 class) [EST]
dm_len    = 55;    // gearbox + motor length [EST]
dm_shaft  = 4;
dm_g      = 95;    // [EST]

/* [Body] */
body_L     = 340;  // tub outside length
body_W     = 240;  // tub outside width
body_r     = 20;   // corner radius
tub_h      = 60;   // floor underside to tub rim
floor_t    = 3;
tub_wall   = 2.0;
lid_wall   = 2.0;
lid_t      = 2.0;  // lid roof
lid_headroom = 8;  // cutter motor top to lid roof
lid_overlap  = 10; // lid skirt hangs this far over the tub wall
lid_clear  = 0.3;  // lid skirt to tub wall
lid_chamfer = 8;
bot_chamfer = 3;
rib_t      = 5;    // joint rib thickness per half
rib_w      = 9;    // joint rib depth

/* [Cutter] */
cut_height   = 40;   // trimmer line height above the ground
cut_d        = 200;  // line tip-to-tip diameter after trimming the line
spool_d      = 80;   // [EST] bump spool diameter - measure yours
spool_h      = 40;   // [EST]
spool_line_z = 12;   // [EST] line eyelet height above the spool bottom
spool_g      = 110;  // [EST] spool, cap, spring and 1 mm line
coupler_len  = 15;   // tower plate underside to spool top
guard_clear  = 10;
guard_wall   = 2.5;
guard_drop   = 10;   // skirt ends this far below the line plane
collar_h     = 20;
dam_h        = 10;   // ring around the spool hole, stops water and clippings

/* [Boards] */
nuc_w = 70;          // [ST] UM3417 fig. 7
nuc_l = 133.35;      // [ST] UM3417 fig. 7
// Board frame: x across the 70 mm side, y from the USB/RJ45 edge (y = 0)
// to the ST-LINK edge (y = 133.35). Measured from UM3417 fig. 7 and fitted
// to the Arduino Uno hole pattern, +-0.3 mm. Check with calipers.
nuc_holes = [[10.85, 105.09], [59.11, 103.82], [43.87, 53.02]];
nuc_standoff = 12;
nuc_pos = [110, 0];  // board centre. ST-LINK end faces +Y.
rbt_size = [85, 56]; // [EST] X-STM32MP-RBT01 - measure yours
rbt_inset = 3.5;     // [EST] hole inset from the board corners
rbt_standoff = 10;
rbt_pos = [-111, -68];

/* [Layout] */
bat_pos = [-111, 62];     // 4 x 18650 cradle
pwr_pos = [-105, -9.5];   // BMS + fuses + 5 V buck plate
pwr_size = [70, 40];
drv_pos = [112, -90];     // cutter driver plate
drv_size = [50, 30];
tof_z_off = 28;           // ToF window centre above the floor
side_tof_x = 60;
cam_x = 140;              // camera pod on the lid
cam_mast_h = 35;
cam_tilt = 20;            // camera looks down by this angle
estop_pos = [-140, 0];
vent_pos = [[-50, 0], [45, 0]];

/* [Power and performance budget] */
cell_mAh = 2500;
cells_s  = 4;
cell_V   = 3.6;
P_cut    = 100;    // [EST] W while cutting with 1 mm line
P_drive  = 15;     // [EST] W
P_logic  = 5;      // [EST] W: N6 board, Wi-Fi, 4 ToF, camera
crr      = 0.20;
slope    = 15;
accel    = 0.3;
mec_eff  = 0.70;
v_mow    = 0.30;

/* [Hidden] */
$fn = 48;
m3 = 3.4;
m4 = 4.5;

// ---------- cutter motor presets ----------
is775   = CUT_MOTOR == "775_DC";
cm_name = is775 ? "Cutter motor RS-775 DC 12-18 V [EST]" : "Stock GIGAWATTS 250 W motor, 230 V AC [EST]";
cm_d       = is775 ? 42   : 58;
cm_len     = is775 ? 67   : 80;
cm_boss    = is775 ? 17.5 : 22;
cm_hole_sp = is775 ? 29   : 0;
cm_g       = is775 ? 330  : 480;

// ---------- heights ----------
axle_z    = wheel_d / 2;
pod_top   = dm_d / 2 + 3;            // motor axis to floor underside
pod_bot   = dm_d / 2 + 3;
deck_z    = axle_z + pod_top;        // floor underside
floor_top = deck_z + floor_t;
tub_top   = deck_z + tub_h;          // tub rim
track     = body_W + 2 * wheel_gap + wheel_w;

spool_bot  = cut_height - spool_line_z;
spool_top  = spool_bot + spool_h;
tower_t    = 6;
tower_bot  = spool_top + coupler_len;
motor_face = tower_bot + tower_t;
motor_top  = motor_face + cm_len;
post_h     = tower_bot - floor_top;
tower_bolt_r = 60;
cut_hole_d = spool_d + 12;
dam_ro     = cut_hole_d / 2 + 2.4;

guard_r_in   = cut_d / 2 + guard_clear;
guard_r_out  = guard_r_in + guard_wall;
skirt_bot    = cut_height - guard_drop;
skirt_flange = 12;
skirt_hole_r = guard_r_in - skirt_flange / 2;

lid_top    = motor_top + lid_headroom + lid_t;
lid_in_top = lid_top - lid_t;
lid_bot    = tub_top - lid_overlap;
lid_off    = lid_clear + lid_wall;
lip_w      = 2.6;
lip_h      = 2.6;
tof_z      = floor_top + tof_z_off;

// ---------- drive ----------
wheel_pos  = [[ wheelbase / 2,  track / 2], [ wheelbase / 2, -track / 2],
              [-wheelbase / 2,  track / 2], [-wheelbase / 2, -track / 2]];  // FL FR RL RR
wheel_hand = [1, -1, -1, 1];
pod_w      = 44;
pod_len    = 60;
pod_rear   = 3;
pod_yc     = body_W / 2 - pod_len / 2;   // pod centre, |y|
pod_screw  = [16.5, 22];
pod_wire_dy = 21;                        // wire hole, local y (outward)
split_gap  = 1.0;
dm_front_y = body_W / 2 - 1;             // gearbox face, |y|

fender_gap = 7;
fender_ri  = wheel_d / 2 + fender_gap;
fender_ro  = fender_ri + 2.4;
fender_wy  = wheel_gap + wheel_w + 5;
fender_dx  = 22;
fender_screw_z = 30;                     // above the axle

// ---------- ToF ----------
tof_win   = [20, 14];
tof_pcb   = [25, 20];                    // [EST] VL53L9 board size (y, z)
tof_holes = [[-10, 7], [10, 7], [-10, -7], [10, -7]];  // [EST] M2, board frame (y, z)
tof_spots = [[ body_L / 2 - tub_wall, 0, 0], [-(body_L / 2 - tub_wall), 0, 180],
             [side_tof_x,  body_W / 2 - tub_wall, 90], [side_tof_x, -(body_W / 2 - tub_wall), 270]];
tof_boss_xy = [[-17, -10], [-17, 10]];   // holder frame

// ---------- camera pod ----------
cam_in   = CAMERA == "USB_UVC" ? [30, 40, 40] : [18.6, 30, 33];   // inside x, y, z (top open)
head_o   = [cam_in[0] + 5.4, cam_in[1] + 6, cam_in[2] + 3];
head_c   = [head_o[0] / 2 - 4, 0, 4 + cam_mast_h + head_o[2] / 2];
cam_foot_holes = [for (sx = [-1, 1], sy = [-1, 1]) [sx * 15, sy * 13]];

// ---------- battery / boxes ----------
bat_lx = 80;  bat_ly = 77;  bat_h = 21;   // 4 x 18650 holder
cradle = [bat_lx + 5, bat_ly + 5];
pwr_h = 15;
drv_h = 15;

// ---------- helpers ----------
function rot2(p, a) = [p[0] * cos(a) - p[1] * sin(a), p[0] * sin(a) + p[1] * cos(a)];
function nuc_xy(h) = [nuc_pos[0] - nuc_w / 2 + h[0], nuc_pos[1] - nuc_l / 2 + h[1]];
function corners(c, sz, inset) = [for (sx = [-1, 1], sy = [-1, 1]) [c[0] + sx * (sz[0] / 2 - inset), c[1] + sy * (sz[1] / 2 - inset)]];
function sum_mass(t, i = 0) = i < len(t) ? t[i][1] * t[i][2] + sum_mass(t, i + 1) : 0;
function sum_moment(t, i = 0) = i < len(t) ? t[i][1] * t[i][2] * t[i][3] + sum_moment(t, i + 1) : [0, 0, 0];
function r1(x) = round(x * 10) / 10;
function fits(a, b) = (a <= bed_x && b <= bed_y) || (a <= bed_y && b <= bed_x);

// ---------- floor bosses: [x, y, height, insert from below?, diameter] ----------
skirt_holes = [for (k = [0 : 7]) let (a = 22.5 + 45 * k) [skirt_hole_r * cos(a), skirt_hole_r * sin(a)]];
tower_lugs  = [for (a = [45 : 90 : 315]) [tower_bolt_r * cos(a), tower_bolt_r * sin(a)]];
pod_holes   = [for (w = wheel_pos, sx = [-1, 1], sy = [-1, 1]) [w[0] + sx * pod_screw[0], sign(w[1]) * (pod_yc + sy * pod_screw[1])]];
wire_holes  = [for (w = wheel_pos) [w[0], sign(w[1]) * (pod_yc + pod_wire_dy)]];
tof_bosses  = [for (t = tof_spots, b = tof_boss_xy) [t[0], t[1]] + rot2(b, t[2])];
nuc_bosses  = [for (h = nuc_holes) nuc_xy(h)];
rbt_bosses  = corners(rbt_pos, rbt_size, rbt_inset);
bat_bosses  = corners(bat_pos, cradle, 6);
pwr_bosses  = corners(pwr_pos, pwr_size, 5);
drv_bosses  = corners(drv_pos, drv_size, 5);
plate_boss_h = 8;

floor_bosses = concat(
    [for (p = nuc_bosses)  [p[0], p[1], nuc_standoff, 0, 8]],
    [for (p = rbt_bosses)  [p[0], p[1], rbt_standoff, 0, 8]],
    [for (p = bat_bosses)  [p[0], p[1], plate_boss_h, 0, 8]],
    [for (p = pwr_bosses)  [p[0], p[1], plate_boss_h, 0, 8]],
    [for (p = drv_bosses)  [p[0], p[1], plate_boss_h, 0, 8]],
    [for (p = tof_bosses)  [p[0], p[1], 5, 0, 8]],
    [for (p = tower_lugs)  [p[0], p[1], 6, 0, 10]],
    [for (p = skirt_holes) [p[0], p[1], 6, 1, 8]],
    [for (p = pod_holes)   [p[0], p[1], 6, 1, 8]]);

// ---------- wall bosses: [x, y, z, outward direction] (point on the outer wall) ----------
lid_screws = concat(
    [for (x = [-140, -75, 75, 140], s = [-1, 1]) [x, s * body_W / 2, tub_top - 5, s > 0 ? 90 : 270]],
    [for (y = [-90, 90], s = [-1, 1]) [s * body_L / 2, y, tub_top - 5, s > 0 ? 0 : 180]]);
fender_screws = [for (w = wheel_pos, dx = [-fender_dx, fender_dx])
                     [w[0] + dx, sign(w[1]) * body_W / 2, axle_z + fender_screw_z, w[1] > 0 ? 90 : 270]];
wall_bosses = INCLUDE_FENDERS ? concat(lid_screws, fender_screws) : lid_screws;

tof_windows = [[ body_L / 2, 0, tof_z, 0], [-body_L / 2, 0, tof_z, 180],
               [side_tof_x, body_W / 2, tof_z, 90], [side_tof_x, -body_W / 2, tof_z, 270]];
port_charge = [-body_L / 2,  45, floor_top + 29, 180];   // DC jack or XT30 panel, 12 mm [EST]
port_switch = [-body_L / 2, -27, floor_top + 29, 180];   // main switch, 16 mm [EST]
port_usb    = [ body_L / 2,  60, floor_top + 29, 0];     // panel-mount USB-C extension to ST-LINK CN10 [EST]

// ---------- lid bosses: [x, y, diameter, height] hanging under the roof ----------
vent_screws = [for (v = vent_pos, a = [90, 210, 330]) v + [26 * cos(a), 26 * sin(a)]];
cam_screws  = [for (h = cam_foot_holes) [cam_x + h[0], h[1]]];
lid_inserts = concat(vent_screws, cam_screws);
lid_bosses  = concat([[estop_pos[0], estop_pos[1], 34, 3]],
                     [for (p = lid_inserts) [p[0], p[1], 8, 6]]);

// ---------- joint rib bolts: [y, z] ----------
tub_rib_bolts = concat([for (y = [-105, -75, 75, 105]) [y, floor_top + rib_w / 2]],
                       [for (s = [-1, 1], z = [floor_top + 18, floor_top + 40]) [s * (body_W / 2 - tub_wall - rib_w / 2), z]]);
lid_rib_bolts = concat([for (y = [-95, -65, -30, 30, 65, 95]) [y, lid_in_top - rib_w / 2]],
                       [for (s = [-1, 1], z = [tub_top + 15, tub_top + 35]) [s * (body_W / 2 + lid_clear - rib_w / 2), z]]);

// ---------- printed-part masses (g) ----------
rho        = print_rho / 1000;
perim      = 2 * (body_L + body_W) - (8 - 2 * PI) * body_r;
floor_area = body_L * body_W - (4 - PI) * body_r * body_r - PI * pow(cut_hole_d / 2, 2);
tub_mass   = rho * (floor_area * floor_t * 0.75 + perim * (tub_h - floor_t) * tub_wall) + 55;   // + ribs, bosses, dam
lid_mass   = rho * ((body_L + 2 * lid_off) * (body_W + 2 * lid_off) * lid_t
                    + (perim + 2 * PI * lid_off) * (lid_top - lid_bot) * lid_wall) + 35;
pod_mass   = rho * (pod_w * pod_len * (pod_top + pod_bot) - PI * pow(dm_d / 2, 2) * (pod_len - pod_rear)) * 0.55;
skirt_mass = rho * (PI * (pow(guard_r_out, 2) - pow(guard_r_in, 2)) * (deck_z - skirt_bot)
                    + PI * (pow(guard_r_in, 2) - pow(guard_r_in - skirt_flange, 2)) * 3);
fender_mass = rho * (PI * (pow(fender_ro, 2) - pow(fender_ri, 2)) * 140 / 360 * fender_wy + 2 * fender_ro * 26 * 2.4);

// ---------- weight budget: [name, qty, grams each, [x, y, z] of the group CG] ----------
cam_cg  = [cam_x + head_c[0], 0, lid_top + head_c[2]];
bat_cg  = [bat_pos[0], bat_pos[1], floor_top + plate_boss_h + 3 + bat_h / 2];
PARTS = [
    // electronics
    ["NUCLEO-N657X0-Q [EST]",                  1, 55, [nuc_pos[0], nuc_pos[1], floor_top + nuc_standoff + 5]],
    ["X-NUCLEO-67W61M1 Wi-Fi shield [EST]",    1, 15, [nuc_pos[0], nuc_pos[1], floor_top + nuc_standoff + 14]],
    ["X-STM32MP-RBT01 motor board [EST]",      1, 45, [rbt_pos[0], rbt_pos[1], floor_top + rbt_standoff + 5]],
    ["VL53L9 ToF board + cable [EST]",         4,  8, [side_tof_x / 2, 0, tof_z]],
    [CAMERA == "USB_UVC" ? "USB camera [EST]" : "MIPI CSI camera module + ribbon [EST]",
                                               1, CAMERA == "USB_UVC" ? 45 : 8, cam_cg],
    ["USB-C panel extension for ST-LINK [EST]", 1, 12, [150, 50, floor_top + 25]],
    // power
    ["18650 Li-ion cell",                      4, 47, bat_cg],
    ["4 x 18650 holder",                       1, 35, bat_cg],
    ["4S BMS + 5 V USB-C buck + fuses [EST]",  1, 35, [pwr_pos[0], pwr_pos[1], floor_top + plate_boss_h + 3 + pwr_h / 2]],
    ["Cutter driver [EST]",                    1, 40, [drv_pos[0], drv_pos[1], floor_top + plate_boss_h + 3 + drv_h / 2]],
    ["E-stop [EST]",                           1, 30, [estop_pos[0], estop_pos[1], lid_top - 10]],
    ["Main switch + charge jack [EST]",        1, 15, [-body_L / 2 + 10, 10, floor_top + 29]],
    ["Wiring and connectors [EST]",            1, 60, [0, 0, floor_top + 15]],
    // drive
    ["Mecanum wheel 75 x 35 [EST]",            4, wheel_g, [0, 0, axle_z]],
    ["Drive gearmotor JGA25-370 class [EST]",  4, dm_g,    [0, 0, axle_z]],
    ["Wheel hub coupler [EST]",                4, 8,       [0, 0, axle_z]],
    // cutter
    [cm_name,                                  1, cm_g,    [0, 0, motor_face + cm_len / 2]],
    ["Trimmer spool + 1 mm line [EST]",        1, spool_g, [0, 0, spool_bot + spool_h / 2]],
    ["Shaft-to-spool adapter [EST]",           1, 15,      [0, 0, spool_top + coupler_len / 2]],
    // printed parts
    ["Tub, 2 halves",                          1, tub_mass,  [0, 0, deck_z + tub_h * 0.3]],
    ["Lid, 2 halves",                          1, lid_mass,  [0, 0, lid_top - (lid_top - lid_bot) * 0.3]],
    ["Motor pod (saddle + cap)",               4, pod_mass,  [0, 0, axle_z]],
    ["Cutter tower",                           1, 65,        [0, 0, floor_top + post_h]],
    ["Guard skirt, 2 halves",                  1, skirt_mass, [0, 0, (skirt_bot + deck_z) / 2]],
    ["Wheel fender",   INCLUDE_FENDERS ? 4 : 0, fender_mass, [0, 0, axle_z + 30]],
    ["ToF holder",                             4, 6,  [side_tof_x / 2, 0, tof_z - 8]],
    ["Camera pod + cover",                     1, CAMERA == "USB_UVC" ? 45 : 30, [cam_x, 0, lid_top + head_c[2] * 0.6]],
    ["Vent base + cap",                        2, 16, [(vent_pos[0][0] + vent_pos[1][0]) / 2, 0, lid_top + 8]],
    ["Battery cradle",                         1, 35, [bat_pos[0], bat_pos[1], floor_top + plate_boss_h + 5]],
    ["Mount plates (power, cutter driver)",    1, 20, [0, -50, floor_top + plate_boss_h + 1.5]],
    ["Screws, heat-set inserts, nuts [EST]",   1, 80, [0, 0, floor_top + 10]]
];

// =====================================================================
//  Assembly
// =====================================================================
c_tub = "SlateGray";
c_lid = "OliveDrab";
c_prt = "Orange";

module assembly() {
    lift = VIEW == "lid_lifted" ? 150 : 0;

    // body
    color(c_tub) for (s = [-1, 1]) tub_half(s);
    if (VIEW != "no_lid") translate([0, 0, lift]) {
        color(c_lid) for (s = [-1, 1]) lid_half(s);
        color(c_prt) for (v = vent_pos) translate([v[0], v[1], lid_top]) {
            vent_base();
            translate([0, 0, vent_cap_z]) mirror([0, 0, 1]) vent_cap();
        }
        translate([cam_x, 0, lid_top]) { color(c_prt) camera_pod(); color("Gold") camera_pod_cover_placed(); camera_board(); }
        translate([estop_pos[0], estop_pos[1], lid_top]) estop();
    }

    // wheels, pods, motors, fenders
    for (i = [0 : 3]) let (w = wheel_pos[i], s = sign(w[1])) {
        translate([w[0], w[1], axle_z]) mecanum(wheel_hand[i]);
        translate([w[0], s * pod_yc, axle_z]) rotate(s > 0 ? 0 : 180) color(c_prt) { motor_pod_part(0); motor_pod_part(1); }
        translate([w[0], s * (dm_front_y - dm_len / 2), axle_z]) rotate([-s * 90, 0, 0]) gearmotor();
        if (INCLUDE_FENDERS) translate([w[0], s * body_W / 2, axle_z]) rotate(s > 0 ? 0 : 180) color(c_prt) wheel_fender();
    }

    // cutter
    color(c_prt) translate([0, 0, floor_top]) cutter_tower();
    color("DarkOrange") guard_skirt(0);
    cutter_motor_and_spool();

    // boards
    translate([nuc_pos[0] - nuc_w / 2, nuc_pos[1] - nuc_l / 2, floor_top + nuc_standoff]) nucleo_n6_stack();
    translate([rbt_pos[0], rbt_pos[1], floor_top + rbt_standoff]) rbt01();

    // power
    translate([bat_pos[0], bat_pos[1], floor_top + plate_boss_h]) {
        color(c_prt) battery_cradle();
        translate([0, 0, 3]) cell_pack(bat_lx, bat_ly, 18.6, 65, "RoyalBlue");
    }
    translate([pwr_pos[0], pwr_pos[1], floor_top + plate_boss_h]) {
        color(c_prt) mount_plate(pwr_size);
        color("MediumPurple") translate([0, 0, 3]) module_box(pwr_size - [8, 8], pwr_h);
    }
    translate([drv_pos[0], drv_pos[1], floor_top + plate_boss_h]) {
        color(c_prt) mount_plate(drv_size);
        color("RoyalBlue") translate([0, 0, 3]) module_box(drv_size - [6, 6], drv_h);
    }
    wall_ports();

    // ToF
    for (t = tof_spots) translate([t[0], t[1], floor_top]) rotate(t[2]) { color(c_prt) tof_holder(); tof_board(); }
}

// =====================================================================
//  Report (console)
// =====================================================================
M_total = sum_mass(PARTS);
CG      = sum_moment(PARTS) / M_total;
f_front = (wheelbase / 2 + CG[0]) / wheelbase;
f_left  = (track / 2 + CG[1]) / track;
F_need  = M_total / 1000 * (9.81 * (crr * cos(slope) + sin(slope)) + accel);
T_wheel = F_need / mec_eff / 4 * wheel_d / 2000;
rpm_need = v_mow / (PI * wheel_d / 1000) * 60;
pack_V  = cells_s * cell_V;
pack_Wh = pack_V * cell_mAh / 1000;
P_total = P_cut + P_drive + P_logic;
tub_xy  = [body_L / 2, body_W];
lid_xy  = [body_L / 2 + lid_off, body_W + 2 * lid_off];

// pairs of floor bosses closer than their radii + 1 mm
function boss_clash(b, i = 0, j = 1) =
    i >= len(b) - 1 ? [] :
    j >= len(b) ? boss_clash(b, i + 1, i + 2) :
    concat(norm([b[i][0] - b[j][0], b[i][1] - b[j][1]]) < (b[i][4] + b[j][4]) / 2 + 1 ? [[r1(b[i][0]), r1(b[i][1])]] : [],
           boss_clash(b, i, j + 1));
clash = boss_clash(floor_bosses);
// floor bosses that touch the inside of a wall (CGAL then makes a non-manifold edge)
wall_touch = [for (b = floor_bosses)
                  if (abs(b[1]) + b[4] / 2 > body_W / 2 - tub_wall - 1 || abs(b[0]) + b[4] / 2 > body_L / 2 - tub_wall - 1)
                      [r1(b[0]), r1(b[1])]];

module report() {
    echo("=================== GRASSKIT ROVER v2 - WEIGHT BUDGET ===================");
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
    echo(str("Footprint         : ", body_L + 2 * lid_off, " x ", INCLUDE_FENDERS ? body_W + 2 * fender_wy : track + wheel_w,
             " mm, lid top ", round(lid_top), " mm, camera top ~", round(lid_top + head_c[2] + head_o[2] / 2 + 5), " mm"));
    echo(str("Cut height ", cut_height, " mm, swath ", cut_d, " mm, clearance under pods ", axle_z - pod_bot, " mm"));
    echo("--------------------------- PRINT BED ---------------------------");
    echo(str("Tub half ", tub_xy[0], " x ", tub_xy[1], " x ", tub_h, " mm : ",
             fits(tub_xy[0], tub_xy[1]) ? "fits" : "DOES NOT FIT", " a ", bed_x, " x ", bed_y, " bed"));
    echo(str("Lid half ", r1(lid_xy[0]), " x ", r1(lid_xy[1]), " x ", r1(lid_top - lid_bot), " mm : ",
             fits(lid_xy[0], lid_xy[1]) ? "fits" : "DOES NOT FIT"));
    echo(str("Guard skirt half ", 2 * guard_r_out, " x ", guard_r_out, " mm : ",
             fits(2 * guard_r_out, guard_r_out) ? "fits" : "DOES NOT FIT"));
    if (len(clash) > 0) echo(str("WARNING: floor bosses overlap at ", clash));
    if (len(wall_touch) > 0) echo(str("WARNING: floor bosses touch a wall at ", wall_touch));
    if (post_h < 3) echo("WARNING: cut_height too low - the spool hits the floor.");
    if (norm([wheelbase / 2 - pod_w / 2, pod_yc - pod_len / 2]) < guard_r_out + 3) echo("WARNING: motor pods touch the guard ring.");
    if (skirt_bot < 8) echo("WARNING: guard skirt drags on the ground.");
}

// =====================================================================
//  Render
// =====================================================================
report();

if (MODE == "assembly") assembly();
else if (MODE == "tub_front_half")   translate([0, 0, -deck_z]) tub_half(1);                     // floor on the bed
else if (MODE == "tub_rear_half")    translate([0, 0, -deck_z]) tub_half(-1);
else if (MODE == "lid_front_half")   translate([0, 0, lid_top]) mirror([0, 0, 1]) lid_half(1);   // roof on the bed
else if (MODE == "lid_rear_half")    translate([0, 0, lid_top]) mirror([0, 0, 1]) lid_half(-1);
else if (MODE == "motor_pod") {                                                                   // print 4 sets
    translate([0, 0, pod_top]) rotate([180, 0, 0]) motor_pod_part(0);
    translate([pod_w + 10, 0, pod_bot]) motor_pod_part(1);
}
else if (MODE == "cutter_tower")     cutter_tower();                                              // tree supports under the roof
else if (MODE == "guard_skirt_half") translate([0, 0, deck_z]) mirror([0, 0, 1]) guard_skirt(1); // print 2
else if (MODE == "wheel_fender")     rotate([90, 0, 0]) wheel_fender();                        // print 4, web on the bed
else if (MODE == "tof_holder")       translate([0, 0, -5]) tof_holder();                          // print 4
else if (MODE == "camera_pod")       translate([0, 0, head_o[1] / 2]) rotate([90, 0, 0]) camera_pod();   // on its side
else if (MODE == "camera_pod_cover") camera_pod_cover();
else if (MODE == "vent_base")        vent_base();                                                 // print 2
else if (MODE == "vent_cap")         vent_cap();                                                  // print 2, roof on the bed
else if (MODE == "battery_cradle")   battery_cradle();
else if (MODE == "power_plate")      mount_plate(pwr_size);
else if (MODE == "driver_plate")     mount_plate(drv_size);

include <rover_printed_parts.scad>
include <rover_bought_parts.scad>
