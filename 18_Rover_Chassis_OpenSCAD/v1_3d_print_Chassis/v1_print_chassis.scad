// =====================================================================
//  GrassKit bench rover - v1 printed chassis for the parts on hand
//  (2026-11-05). OpenSCAD 2021.01 or newer. Units: mm.
//
//  Carries what we have today:
//    NUCLEO-L073RZ + X-NUCLEO-67W61M1 Wi-Fi shield   (rear)
//    X-STM32MP-RBT01 motor board                     (front)
//    4 x yellow TT gear motors + the 4 mecanum wheels from the red chassis
//    4 x 18650 holder and 2 x 2 AA holder            (strapped under the deck)
//
//  Frame: origin on the ground under the middle of the rover.
//         +X = forward (the FRONT arrow), +Y = left, +Z = up.
//  Motor numbers match the firmware and the RBT01A build sheet:
//         M1 front-left, M2 front-right, M3 rear-left, M4 rear-right.
//
//  MODE = "assembly"  everything, to check the fit
//         "deck"      the one printed part, upside down as it prints
//         "spacers"   4 x 12 mm tubes (Nucleo) + 4 x 18 mm tubes (RBT01)
//
//  [ST]  value from an ST document in documentation/
//  [EST] estimate from photos: measure your part, change the number,
//        re-export. Nothing else needs editing.
// =====================================================================

/* [Render] */
MODE = "assembly"; // [assembly, deck, spacers]
SHOW_WHEELS = true;
// Recommended: mount the RBT01 underside-up, so its 40-pin socket and the
// soldered motor joints face up and the jumper wires plug in from above.
RBT01_UNDERSIDE_UP = true;

/* [Wheels] */
wheel_d = 80;    // [EST] mecanum wheel diameter
wheel_w = 40;    // [EST] mecanum wheel width
hub_gap = 14;    // [EST] gearbox outer face to wheel inner face (the black coupler)

/* [TT gear motor] */
tt_h       = 22.5;   // gearbox height
tt_t       = 18.8;   // gearbox thickness, along the shaft
tt_front   = 11.25;  // shaft centre to the gearbox front end
tt_gear    = 36;     // [EST] gearbox length, front end to the motor can
tt_len     = 70;     // overall length without the shaft
tt_hole_dz = 8.75;   // [EST] the 2 mounting holes sit 17.5 mm apart, above and below the shaft line
tt_shaft_d = 5.4;
tt_shaft_out = 9.5;  // shaft stub past each gearbox face
fit        = 0.4;    // clearance each side of the gearbox

/* [Chassis] */
wheelbase = 150;   // front axle to rear axle
deck_L    = 180;
deck_W    = 130;
deck_t    = 4.5;
deck_r    = 6;
wall_o    = 2.5;   // outer pocket wall (wheel side)
wall_i    = 2.5;   // inner pocket wall
wall_end  = 3;
wall_drop = 4;     // walls run this far below the motor, for strength under the lower slot
pocket_in = 30;    // pocket walls reach this far from the shaft toward the middle
slot_x    = [15, 25];  // bolt slots: distance range from the shaft (holes measured at about 20 [EST])
peg_x     = [9, 14];   // relief for the small locating peg, distance range from the shaft [EST]
notch_w   = 9;     // shaft notch, open at the bottom so the motor pushes in from below
m3        = 3.4;

/* [Boards] */
nuc_size  = [70, 82.5];   // [ST] UM1724 fig. 5
// [ST] UM1724 fig. 5 dimension chain. Board frame: x across the 70 mm side,
// y up from the bottom edge (Arduino end), ST-LINK USB end at y = 82.5.
nuc_holes = [[10.87, 54.57], [59.13, 53.30], [43.89, 2.00]];
nuc_front = -5.5;         // deck X of the Nucleo's Arduino end (its ST-LINK end points backward)
rbt_size  = [65, 56.5];   // [EST] Raspberry Pi HAT outline: UM3482 fig. 2 + photos
rbt_inset = 3.5;          // [EST] HAT corner holes, 58 x 49 apart: check with calipers
rbt_rear  = 4;            // deck X of the RBT01 edge with the 40-pin socket (faces the Nucleo)
board_hole = 4.0;         // M3 with about 0.3 mm of play each way
cbore_d   = 7.5;          // screw-head counterbore under the deck, keeps the heads flush
cbore_h   = 3;
nuc_spacer = 12;          // clears the morpho pins under the Nucleo
rbt_spacer = 18;          // clears the 330 uF capacitor and terminal headers (underside-up)
spacer_od  = 6;

/* [Batteries] */
b18    = [81, 77, 21];    // [EST] 4 x 18650 holder: X (across the cells), Y (along the cells), height
b18_x0 = -44;             // its rear end (its front lip is also the AA holder's rear lip)
aa     = [32, 63, 31];    // [EST] 2 x 2 AA holder: X, Y (along the cells), height
aa_x0  = 42;              // its rear end
pack_fit = 1.5;
lip_h  = 6;               // locating lips under the deck
lip_t  = 2;
tie_slot = [6, 2.6];      // zip-tie slot: tie width x tie thickness

// ---------------------------------------------------------------------
// derived
// ---------------------------------------------------------------------
$fn = 32;
axle_z   = wheel_d / 2;
deck_bot = axle_z + tt_h / 2 + 0.2;   // motor tops sit just under the deck
deck_top = deck_bot + deck_t;
shaft_x  = wheelbase / 2;
y_out    = deck_W / 2;                // outer face of the outer wall = deck edge
y_go     = y_out - wall_o - fit;      // gearbox outer face
y_gi     = y_go - tt_t;               // gearbox inner face
y_wi     = y_out - wall_o - tt_t - 2 * fit;  // inner wall, motor side
y_wii    = y_wi - wall_i;             // inner wall, battery side
motor_y  = (y_go + y_gi) / 2;
wheel_y  = y_go + hub_gap + wheel_w / 2;
z_wall_bot = axle_z - tt_h / 2 - wall_drop;
corners  = [[1, 1], [1, -1], [-1, 1], [-1, -1]];   // M1 FL, M2 FR, M3 RL, M4 RR

function nuc_xy(h) = [nuc_front - h[1], h[0] - nuc_size[0] / 2];
function rbt_xy(h) = [rbt_rear + rbt_size[1] - h[1], h[0] - rbt_size[0] / 2];
rbt_hole_uv = [[rbt_inset, rbt_inset], [rbt_size[0] - rbt_inset, rbt_inset],
               [rbt_inset, rbt_size[1] - rbt_inset], [rbt_size[0] - rbt_inset, rbt_size[1] - rbt_inset]];
nuc_deck_holes = [for (h = nuc_holes) nuc_xy(h)];
rbt_deck_holes = [for (h = rbt_hole_uv) rbt_xy(h)];

b18_ties = [for (x = [-30, 22], s = [-1, 1]) [x, s * (b18[1] / 2 + pack_fit), 0]];             // ties run across Y
aa_ties  = [for (x = [aa_x0 + 4, aa_x0 + 24], s = [-1, 1]) [x, s * (aa[1] / 2 + pack_fit), 0]];
wire_slots = [for (s = [-1, 1]) [0, s * 50]];   // motor and battery leads come up here

// ---------------------------------------------------------------------
// report
// ---------------------------------------------------------------------
clearance = min(z_wall_bot, deck_bot - b18[2], deck_bot - aa[2]);
echo(str("Rover ", wheelbase + wheel_d, " long x ", round(2 * (wheel_y + wheel_w / 2)), " wide; track ",
         round(2 * wheel_y), ", wheelbase ", wheelbase));
echo(str("Deck ", deck_L, " x ", deck_W, " x ", deck_t, ", top at ", deck_top, " mm; ground clearance ",
         clearance, " mm (lowest: ", clearance == deck_bot - aa[2] ? "AA pack" : "pocket walls / 18650 pack", ")"));
echo(str("Nucleo holes on the deck: ", nuc_deck_holes));
echo(str("RBT01 holes on the deck: ", rbt_deck_holes));
// what hangs under the deck beside the packs: pocket walls, motor cans, bolt nuts and tails, shaft stubs
nut_x  = shaft_x - slot_x[1] - 3.2;          // nearest a motor nut can come to the middle (x)
nut_y  = y_wii - 2.4 - 3;                     // nut + 3 mm of bolt tail past the inner wall (y)
stub_y = y_gi - tt_shaft_out;                 // inner end of each shaft stub (y)
if (b18[1] / 2 + pack_fit > y_gi - 1)
    echo("WARNING: the 18650 holder is wider than the gap between the motor cans");
if (b18_x0 - pack_fit < -(shaft_x - pocket_in) && b18[1] / 2 + pack_fit > y_wii)
    echo("WARNING: the 18650 holder hits the rear motor pockets");
if (b18_x0 - pack_fit < -nut_x && b18[1] / 2 + pack_fit > nut_y)
    echo("WARNING: the 18650 holder can touch the rear motor bolts");
if (b18_x0 + b18[0] + pack_fit > aa_x0 - pack_fit + 0.01)
    echo("WARNING: the 18650 and AA holders overlap");
if (aa_x0 + aa[0] + pack_fit > nut_x && aa[1] / 2 + pack_fit > nut_y)
    echo("WARNING: the AA holder can touch the front motor bolts");
if (aa_x0 + aa[0] + pack_fit > shaft_x - tt_shaft_d / 2 - 1 && aa[1] / 2 + pack_fit > stub_y - 0.5)
    echo("WARNING: the AA holder reaches the front shaft stubs");

// ---------------------------------------------------------------------
// the printed deck
// ---------------------------------------------------------------------
module deck_outline() offset(r = deck_r) offset(delta = -deck_r) square([deck_L, deck_W], center = true);

// motor pocket for the front-left motor: shaft at x = 0, body toward -x
module pocket_solid() {
    x0 = -pocket_in;
    x1 = tt_front + fit + wall_end;
    h  = deck_bot - z_wall_bot + 0.5;
    translate([x0, y_out - wall_o, z_wall_bot]) cube([x1 - x0, wall_o, h]);
    translate([x0, y_wii, z_wall_bot])          cube([x1 - x0, wall_i, h]);
    translate([tt_front + fit, y_wii, z_wall_bot]) cube([wall_end, y_out - y_wii, h]);
}

module y_rod(d, x, z, len) translate([x, y_wii - 1, z]) rotate([-90, 0, 0]) cylinder(d = d, h = len);

module pocket_cuts() {
    len = y_out - y_wii + 2;
    // shaft notch, open at the bottom
    translate([-notch_w / 2, y_wii - 1, z_wall_bot - 1]) cube([notch_w, len, axle_z - z_wall_bot + 1]);
    y_rod(notch_w, 0, axle_z, len);
    // two bolt slots, above and below the shaft line
    for (s = [-1, 1]) hull() for (x = slot_x) y_rod(m3, -x, axle_z + s * tt_hole_dz, len, $fn = 16);
    // locating peg relief
    hull() for (x = peg_x) y_rod(5, -x, axle_z, len, $fn = 20);
}

module at_corner(c) translate([c[0] * shaft_x, 0, 0]) scale([c[0], c[1], 1]) children();

module lip(x0, x1, y0, y1) translate([x0, y0, deck_bot - lip_h]) cube([x1 - x0, y1 - y0, lip_h + 0.5]);

module lips() {
    // 18650 holder: ends and short side lips
    yb = b18[1] / 2 + pack_fit;
    lip(b18_x0 - pack_fit - lip_t, b18_x0 - pack_fit, -20, 20);
    lip(b18_x0 + b18[0] + pack_fit, b18_x0 + b18[0] + pack_fit + lip_t, -20, 20);
    for (s = [-1, 1]) lip(-12, 12, s > 0 ? yb : -yb - lip_t, s > 0 ? yb + lip_t : -yb);
    // AA holder: ends only (zip ties hold the sides)
    lip(aa_x0 - pack_fit - lip_t, aa_x0 - pack_fit, -15, 15);
    lip(aa_x0 + aa[0] + pack_fit, aa_x0 + aa[0] + pack_fit + lip_t, -15, 15);
}

module stadium(p, len, w) hull() for (s = [-1, 1]) translate([p[0] + s * (len - w) / 2, p[1]]) circle(d = w);

module engraving() {
    depth = 0.8;
    translate([0, 0, deck_top - depth]) linear_extrude(depth + 1) {
        translate([deck_L / 2 - 7, 0]) rotate(-90)
            text("FRONT", size = 5, halign = "center", valign = "center", font = "Liberation Sans:style=Bold");
        for (s = [-1, 1]) translate([deck_L / 2 - 7, s * 18])
            polygon([[-3.5, -3], [-3.5, 3], [3, 0]]);
        labels = ["M1", "M2", "M3", "M4"];
        for (i = [0:3]) translate([corners[i][0] * (deck_L / 2 - 16), corners[i][1] * 54]) rotate(-90)
            text(labels[i], size = 6, halign = "center", valign = "center", font = "Liberation Sans:style=Bold");
    }
}

module deck() {
    difference() {
        union() {
            translate([0, 0, deck_bot]) linear_extrude(deck_t) deck_outline();
            intersection() {
                for (c = corners) at_corner(c) pocket_solid();
                translate([0, 0, z_wall_bot - 1]) linear_extrude(deck_bot - z_wall_bot + 2) deck_outline();
            }
            lips();
        }
        for (c = corners) at_corner(c) pocket_cuts();
        // board screws: through hole + counterbore underneath so the heads sit flush
        for (p = concat(nuc_deck_holes, rbt_deck_holes)) translate([p[0], p[1], 0]) {
            translate([0, 0, deck_bot - 1]) cylinder(d = board_hole, h = deck_t + 2);
            translate([0, 0, deck_bot - 0.01]) cylinder(d = cbore_d, h = cbore_h);
        }
        translate([0, 0, deck_bot - lip_h - 1]) linear_extrude(deck_t + lip_h + 2) {
            for (p = wire_slots) stadium(p, 30, 10);
            for (t = b18_ties) translate([t[0], t[1]]) square([tie_slot[0], tie_slot[1]], center = true);
            for (t = aa_ties)  translate([t[0], t[1]]) square([tie_slot[0], tie_slot[1]], center = true);
        }
        engraving();
    }
}

module spacer(h) difference() {
    cylinder(d = spacer_od, h = h);
    translate([0, 0, -1]) cylinder(d = m3, h = h + 2, $fn = 16);
}

// ---------------------------------------------------------------------
// bought parts, for the assembly view only
// ---------------------------------------------------------------------
module tt_motor() {   // shaft on the y axis, body toward -x, centred on the gearbox
    color("Gold") translate([-(tt_gear - tt_front), -tt_t / 2, -tt_h / 2]) cube([tt_gear, tt_t, tt_h]);
    color("Silver") translate([-(tt_len - tt_front), 0, 0]) rotate([0, 90, 0])
        cylinder(d = 17, h = tt_len - tt_gear);
    color("Ivory") rotate([90, 0, 0]) cylinder(d = tt_shaft_d, h = tt_t + 2 * tt_shaft_out, center = true, $fn = 16);
}

module mecanum_wheel() {   // axis on y, centred; mirroring at the corners gives the X roller pattern
    rd = wheel_d * 0.17;
    rl = wheel_w * 0.95 - rd;
    color("DimGray") rotate([90, 0, 0]) cylinder(d = wheel_d * 0.55, h = wheel_w - 8, center = true, $fn = 48);
    color("#202020") for (s = [-1, 1]) translate([0, s * (wheel_w / 2 - 2), 0])
        rotate([90, 0, 0]) cylinder(d = wheel_d * 0.74, h = 2.5, center = true, $fn = 48);
    color("#4a4a4a") for (i = [0:9]) rotate([0, i * 36, 0]) translate([0, 0, wheel_d / 2 - rd / 2])
        rotate([0, 0, 45]) hull() for (s = [-1, 1]) translate([0, s * rl / 2, 0]) sphere(d = rd, $fn = 16);
}

module nucleo_stack() {   // board frame, z = 0 at the PCB underside
    color("WhiteSmoke") difference() {
        cube([nuc_size[0], nuc_size[1], 1.6]);
        for (h = nuc_holes) translate([h[0], h[1], -1]) cylinder(d = 3.2, h = 4, $fn = 16);
    }
    color("Black") for (x = [1.9, 63]) translate([x, 3.8, 1.6]) cube([5, 48.4, 8.5]);    // morpho CN7 / CN10
    color("Black") for (x = [1.9, 63]) translate([x, 3.8, -8]) cube([5, 48.4, 8]);       // morpho pins underneath
    color("Silver") translate([44, nuc_size[1] - 7, 1.6]) cube([8, 9, 4]);               // ST-LINK USB
    translate([8.3, 0, 12]) {                                                            // X-NUCLEO-67W61M1 [EST]
        color("MidnightBlue") cube([53.3, 58, 1.6]);
        color("Silver") translate([18, 32, 1.6]) cube([16, 22, 2.5]);
    }
}

module rbt01() {   // u along the 40-pin edge, v away from it; z = 0 at the PCB underside, parts on top
    color("WhiteSmoke") difference() {
        union() {
            cube([rbt_size[0], rbt_size[1], 1.6]);
            translate([22.8, -17, 0]) cube([21, 17.5, 1.6]);                    // ToF tab [EST]
        }
        for (h = rbt_hole_uv) translate([h[0], h[1], -1]) cylinder(d = 3.2, h = 4, $fn = 16);
    }
    color("LimeGreen") for (v = [3.5, 26]) translate([57, v, 1.6]) cube([9, 20.3, 10]);   // CN4 / CN5 headers [EST]
    color("LimeGreen") translate([3, -2, 1.6]) cube([10.2, 10, 10]);                    // CN3 power header [EST]
    color("Silver") translate([11.4, 25.6, 1.6]) cylinder(d = 12.5, h = 13.5);           // 330 uF capacitor [EST]
    color("Black") translate([rbt_size[0] / 2 - 25.4, rbt_size[1] - 6, -8.5]) cube([50.8, 5, 8.5]);  // CN2 socket, underside
}

module pack_18650() {
    color("#202020") difference() {
        translate([0, -b18[1] / 2, 0]) cube(b18);
        translate([1.5, -b18[1] / 2 + 5, 3]) cube([b18[0] - 3, b18[1] - 10, b18[2]]);
    }
    color("RoyalBlue") for (i = [0:3]) translate([b18[0] / 8 * (2 * i + 1), 0, b18[2] - 9.3])
        rotate([90, 0, 0]) cylinder(d = 18.4, h = 65, center = true);
}

module pack_aa() color("#202020") translate([0, -aa[1] / 2, 0]) cube(aa);

module assembly() {
    color("DarkOrange") deck();
    for (i = [0:3]) at_corner(corners[i]) {
        translate([0, motor_y, axle_z]) tt_motor();
        if (SHOW_WHEELS) {
            color("Black") translate([0, y_go, axle_z]) rotate([-90, 0, 0]) cylinder(d = 12, h = hub_gap);
            translate([0, wheel_y, axle_z]) mecanum_wheel();
        }
    }
    // Nucleo + Wi-Fi shield, ST-LINK USB toward the back
    for (p = nuc_deck_holes) color("Gray") translate([p[0], p[1], deck_top]) spacer(nuc_spacer);
    translate([nuc_front, -nuc_size[0] / 2, deck_top + nuc_spacer]) rotate([0, 0, 90]) nucleo_stack();
    // RBT01, 40-pin socket edge toward the Nucleo
    for (p = rbt_deck_holes) color("Gray") translate([p[0], p[1], deck_top]) spacer(rbt_spacer);
    if (RBT01_UNDERSIDE_UP)
        translate([0, 0, deck_top + rbt_spacer + 1.6]) rotate([180, 0, 0])
            translate([rbt_rear + rbt_size[1], -rbt_size[0] / 2, 0]) rotate([0, 0, 90]) rbt01();
    else
        translate([rbt_rear + rbt_size[1], -rbt_size[0] / 2, deck_top + rbt_spacer]) rotate([0, 0, 90]) rbt01();
    // batteries, strapped up against the deck underside
    translate([b18_x0, 0, deck_bot - b18[2]]) pack_18650();
    translate([aa_x0, 0, deck_bot - aa[2]]) pack_aa();
}

// ---------------------------------------------------------------------
if (MODE == "assembly") assembly();
else if (MODE == "deck") translate([0, 0, deck_top]) rotate([180, 0, 0]) deck();   // top face on the bed
else if (MODE == "spacers") {
    for (i = [0:3]) translate([i * 10, 0, 0]) spacer(nuc_spacer);
    for (i = [0:3]) translate([i * 10, 12, 0]) spacer(rbt_spacer);
}
