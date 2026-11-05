"""Build sheet for the replacement X-STM32MP-RBT01 (marked XMP$RBT01A) on the GrassKit bench rover.

Writes three pages next to this script:
  rbt01a_build_sheet.html  full page: sheet A (motor + power soldering), sheet B (logic jumpers),
                           meter checks and the serial commands that set each wheel's direction
  rbt01a_sheet_a.html      sheet A alone at 1800 px, for PNG export
  rbt01a_sheet_b.html      sheet B alone at 1800 px, for PNG export

Pin facts: UM3482 Table 2 / Table 11 and schematic sheets 4-5 (CN3 = power in, CN4 = U2 front,
CN5 = U3 rear, test points TP1/3/4/5 and TP10-13), UM1724 Table 30 (morpho pins),
firmware/Core/Inc/hardware_config.h. Wiring rule: black motor lead -> 2A/2B, red -> 1A/1B.
"""
from html import escape
from pathlib import Path

HERE = Path(__file__).parent

C = {"3v3": "#f97316", "gnd": "#111827", "en": "#7c3aed", "dir": "#16a34a",
     "pwm": "#eab308", "vm": "#e11d48", "red": "#dc2626", "blk": "#111827"}
BOARD_RBT = "#3f2b72"
BOARD_NUC = "#1d3a86"


def T(x, y, s, cls, size, anchor="start", extra=""):
    a = f' text-anchor="{anchor}"' if anchor != "start" else ""
    return f'<text class="{cls}" x="{x}" y="{y}" font-size="{size}"{a}{extra}>{escape(s)}</text>'


def marker(x, y, n):
    return f'<g class="marker"><circle cx="{x}" cy="{y}" r="15"/><text x="{x}" y="{y}">{n}</text></g>'


def background(w, h):
    return (f'<rect width="{w}" height="{h}" fill="#dfe6f1"/>'
            '<g stroke="#cfd9e8" stroke-width="1">'
            + "".join(f'<path d="M0 {y}H{w}"/>' for y in range(115, h, 115))
            + "".join(f'<path d="M{x} 0V{h}"/>' for x in range(200, w, 200)) + '</g>')


def holes(points):
    return '<g fill="#d4a537" stroke="#0b1630" stroke-width="1.5">' + "".join(
        f'<circle cx="{x}" cy="{y}" r="10"/>' for x, y in points) + '</g>'


# --------------------------------------------------------------------------------------------
# Sheet A: motors and power, soldered on the RBT01 underside
# --------------------------------------------------------------------------------------------
def sheet_a():
    W, H = 1800, 1250
    s = [f'<svg viewBox="0 0 {W} {H}" xmlns="http://www.w3.org/2000/svg" role="img" '
         'aria-label="Sheet A: which motor wire is soldered to which RBT01 pin">', background(W, H)]
    a = s.append

    # ---- left: the rover seen from above ----
    a(T(40, 72, "THE ROVER · SEEN FROM ABOVE", "sv-ink", 30))
    a(T(40, 98, "Front = the end the arrows stamped on the top plate", "sv-dark", 14))
    a(T(40, 116, "point to. The RBT01 sits on that front half.", "sv-dark", 14))
    a('<path d="M280 134 l36 42 h-21 v40 h-30 v-40 h-21 z" fill="#0b1630"/>')
    a(T(280, 244, "FRONT", "sv-darkmono", 15, "middle"))
    a('<rect x="170" y="258" width="220" height="640" rx="10" fill="#c2302b" stroke="#7f1d1d" stroke-width="3"/>')
    wheels = [(88, 278), (402, 278), (88, 712), (402, 712)]
    for wx, wy in wheels:
        a(f'<rect x="{wx}" y="{wy}" width="70" height="170" rx="18" fill="#1f2937" stroke="#0b1630" stroke-width="2"/>')
        a('<g stroke="#4b5563" stroke-width="3">' + "".join(
            f'<path d="M{wx + 10} {wy + 18 + i * 19}h50"/>' for i in range(7)) + '</g>')
    a(T(123, 270, "mecanum", "sv-dark", 11, "middle"))
    # motors: yellow gearbox, silver can toward the middle of the chassis
    motors = [("M1", "FL", 184, 288, "down"), ("M2", "FR", 310, 288, "down"),
              ("M3", "RL", 184, 748, "up"), ("M4", "RR", 310, 748, "up")]
    for m, pos, mx, my, can in motors:
        a(f'<rect x="{mx}" y="{my}" width="66" height="122" rx="7" fill="#facc15" stroke="#a16207" stroke-width="2.5"/>')
        cy = my + 122 if can == "down" else my - 40
        a(f'<rect x="{mx + 9}" y="{cy}" width="48" height="40" rx="6" fill="#cbd5e1" stroke="#475569" stroke-width="2"/>')
        a(T(mx + 33, my + 58, m, "sv-darkmono", 22, "middle"))
        a(T(mx + 33, my + 80, pos, "sv-darkmono", 13, "middle"))
    for y in (348, 808):  # axles
        a(f'<path d="M158 {y}H184M376 {y}H402" stroke="#0b1630" stroke-width="5"/>')
    # RBT01 on the chassis, motor leads running to it
    a(f'<rect x="200" y="470" width="160" height="104" rx="8" fill="{BOARD_RBT}" stroke="#0b1630" stroke-width="2.5"/>')
    a(T(280, 510, "RBT01", "sv-mono", 16, "middle"))
    a(T(280, 530, "motor pins on", "sv-note", 11, "middle"))
    a(T(280, 545, "its underside", "sv-note", 11, "middle"))
    for x0, y0, x1, y1 in ((217, 450, 222, 470), (343, 450, 338, 470), (217, 708, 222, 574), (343, 708, 338, 574)):
        a(f'<path d="M{x0 - 3} {y0}L{x1 - 3} {y1}" stroke="{C["blk"]}" stroke-width="3"/>'
          f'<path d="M{x0 + 3} {y0}L{x1 + 3} {y1}" stroke="{C["red"]}" stroke-width="3"/>')
    a(T(280, 934, "M1 front-left  ·  M2 front-right", "sv-darkmono", 14, "middle"))
    a(T(280, 956, "M3 rear-left  ·  M4 rear-right", "sv-darkmono", 14, "middle"))
    a(T(280, 978, "(these are the firmware's motor numbers)", "sv-dark", 12.5, "middle"))
    # meter check box
    a('<rect x="40" y="1012" width="480" height="206" rx="10" fill="#0b1630" stroke="#f5b301" stroke-width="2"/>')
    a(T(60, 1042, "AFTER SOLDERING · METER, NO POWER", "sv-amber", 15))
    checks = ["Across one motor's two pins: a few ohms",
              "Middle pair 1B ↔ 1A: not a short",
              "Any motor pin ↔ GND: open",
              "CN3 + ↔ GND: not a short (reading climbs)",
              "CN3 GND ↔ CN2 pin 6: beeps",
              "No solder blob between neighbouring pins"]
    for i, c in enumerate(checks):
        a(T(60, 1072 + i * 24, "•  " + c, "sv-sub", 14))

    # ---- right: the RBT01 underside ----
    a(T(1760, 72, "X-STM32MP-RBT01 · UNDERSIDE", "sv-ink", 30, "end"))
    a(T(1760, 98, "Board turned over, as in your photo: green blocks along the top edge,", "sv-dark", 14, "end"))
    a(T(1760, 116, "black 40-pin socket on the right. Solder on the pin tails on this side.", "sv-dark", 14, "end"))

    # pins along the top edge, left to right, as printed on the underside
    pins = [(760, "2B", "CN4"), (840, "1B", "CN4"), (920, "1A", "CN4"), (1000, "2A", "CN4"),
            (1160, "2B", "CN5"), (1240, "1B", "CN5"), (1320, "1A", "CN5"), (1400, "2A", "CN5")]
    tps = ["TP5", "TP4", "TP3", "TP1", "TP13", "TP12", "TP11", "TP10"]
    # (motor, position, centre x, [(pin x, lead colour, marker number)])
    mot = [("M2", "FRONT-RIGHT", 800, [(760, "blk", 3), (840, "red", 4)]),
           ("M1", "FRONT-LEFT", 960, [(920, "red", 2), (1000, "blk", 1)]),
           ("M4", "REAR-RIGHT", 1200, [(1160, "blk", 7), (1240, "red", 8)]),
           ("M3", "REAR-LEFT", 1360, [(1320, "red", 6), (1400, "blk", 5)])]
    who = {760: "M2 black", 840: "M2 red", 920: "M1 red", 1000: "M1 black",
           1160: "M4 black", 1240: "M4 red", 1320: "M3 red", 1400: "M3 black"}

    # board body
    a(f'<rect x="620" y="566" width="1120" height="444" rx="18" fill="{BOARD_RBT}" stroke="#0b1630" stroke-width="3"/>')
    a(holes([(652, 598), (1712, 598), (652, 980), (1712, 986)]))
    # green blocks peeking past the top edge
    for x0 in (728, 1128):
        a(f'<rect x="{x0}" y="538" width="304" height="28" rx="4" fill="#16a34a" stroke="#0b1630" stroke-width="2"/>')

    for name, pos, cx, leads in mot:
        a(T(cx, 150, f"{name} · {pos}", "sv-darkmono", 14, "middle"))
        a(f'<rect x="{cx - 64}" y="164" width="128" height="68" rx="8" fill="#facc15" stroke="#a16207" stroke-width="2.5"/>')
        a(T(cx, 206, name, "sv-darkmono", 22, "middle"))
        a(f'<rect x="{cx - 46}" y="232" width="92" height="24" rx="5" fill="#cbd5e1" stroke="#475569" stroke-width="2"/>')
        for px, col, n in leads:
            a(f'<rect x="{px - 6}" y="256" width="12" height="10" fill="#94a3b8"/>')
            a(f'<path class="wire" stroke="{C[col]}" d="M{px} 266V592"/>')
            lbl = "black" if col == "blk" else "red"
            side, anchor = (-10, "end") if px < cx else (10, "start")
            a(T(px + side, 300, lbl, "sv-dark", 11.5, anchor))
            a(marker(px, 430, n))

    for (px, lbl, grp), tp in zip(pins, tps):
        a(f'<circle cx="{px}" cy="592" r="12" fill="#d4a537" stroke="#0b1630" stroke-width="2"/>'
          f'<circle cx="{px}" cy="592" r="4" fill="#5c4317"/>')
        a(T(px, 632, lbl, "sv-mono", 19, "middle"))
        a(T(px, 654, who[px], "sv-note", 12, "middle"))
        a(T(px, 672, f"top side: {tp}", "sv-faint", 10.5, "middle"))
    a(T(880, 708, "CN4 · FRONT DRIVER U2", "sv-mono", 14, "middle"))
    a(T(880, 726, "motors 1 and 2", "sv-note", 12, "middle"))
    a(T(1280, 708, "CN5 · REAR DRIVER U3", "sv-mono", 14, "middle"))
    a(T(1280, 726, "motors 3 and 4", "sv-note", 12, "middle"))

    # the rule
    a('<rect x="700" y="752" width="860" height="104" rx="10" fill="#0b1630" stroke="#f5b301" stroke-width="2"/>')
    a(T(722, 782, "THE RULE · SAME ON ALL FOUR MOTORS", "sv-amber", 15))
    a(T(722, 812, "BLACK lead → the 2 pin (2A or 2B)     RED lead → the 1 pin (1A or 1B)", "sv-mono", 16.5))
    a(T(722, 838, "Same polarity as the old rear wheels, which the firmware's default direction was tuned to.", "sv-note", 12.5))
    a(T(1130, 884, "In each block the middle pair (1B, 1A) belongs to two different motors: keep those joints apart.",
        "sv-sub", 13, "middle"))

    # CN2 socket on the right edge
    a('<rect x="1648" y="616" width="56" height="350" rx="5" fill="#0b0f1a" stroke="#64748b" stroke-width="2"/>')
    a('<g fill="#475569">' + "".join(
        f'<circle cx="{x}" cy="{628 + r * 17.5}" r="3.5"/>' for r in range(20) for x in (1664, 1688)) + '</g>')
    a(T(0, 0, "CN2 · 40-PIN SOCKET · PLUG-IN JUMPERS, SHEET B", "sv-mono", 12, "middle",
        ' transform="translate(1628 791) rotate(-90)"'))

    # CN3 power pins, bottom-left
    a('<rect x="668" y="1010" width="154" height="24" rx="4" fill="#16a34a" stroke="#0b1630" stroke-width="2"/>')
    for x, lbl in ((700, "+ VM"), (790, "GND")):
        a(f'<circle cx="{x}" cy="958" r="13" fill="#d4a537" stroke="#0b1630" stroke-width="2"/>'
          f'<circle cx="{x}" cy="958" r="4.5" fill="#5c4317"/>')
        a(T(x, 930, lbl, "sv-mono", 13, "middle"))
    a(T(830, 950, "CN3 · MOTOR POWER IN", "sv-mono", 14))
    a(T(830, 970, "Go by the + / GND print beside it.", "sv-note", 12))
    a(T(830, 987, "Its GND pin beeps to CN2 pin 6.", "sv-note", 12))

    # supply
    a('<path class="wire fat" stroke="#e11d48" d="M700 1075V958"/>')
    a('<path class="wire fat" stroke="#111827" d="M790 1075V958"/>')
    a(marker(700, 1054, 9))
    a(marker(790, 1054, 10))
    a('<rect x="620" y="1086" width="300" height="122" rx="12" fill="#0b1630" stroke="#334155" stroke-width="3"/>')
    a('<circle cx="700" cy="1086" r="11" fill="#e11d48" stroke="#fff" stroke-width="2"/>')
    a('<circle cx="790" cy="1086" r="11" fill="#111827" stroke="#fff" stroke-width="2"/>')
    a(T(640, 1120, "BENCH SUPPLY OR 2S PACK", "sv-mono", 13.5))
    a('<rect x="640" y="1134" width="118" height="42" rx="4" fill="#052e16" stroke="#22c55e" stroke-width="1.5"/>')
    a('<text x="699" y="1163" font-size="21" fill="#4ade80" text-anchor="middle" '
      'font-family="IBM Plex Mono, Consolas, monospace">9.0 V</text>')
    a(T(772, 1150, "7.4–9 V", "sv-note", 12))
    a(T(772, 1168, "1 A limit per wheel", "sv-note", 12))
    a(T(772, 1186, "2 A for all four", "sv-note", 12))
    a(T(950, 1112, "The yellow TT motors are 3–6 V parts and the firmware does not cap the PWM,", "sv-dark", 13.5))
    a(T(950, 1132, "so keep motor power at 7.4–9 V, not the 14 V pack. Below about 5.2 V the", "sv-dark", 13.5))
    a(T(950, 1152, "driver stays off (undervoltage lock-out).", "sv-dark", 13.5))
    a(T(950, 1182, "Connect power last, after the meter checks pass.", "sv-alert", 13.5))

    # top-right callout
    a('<rect x="1462" y="140" width="298" height="168" rx="10" fill="#0b1630" stroke="#f5b301" stroke-width="2"/>')
    a(T(1480, 168, "NO SCREW TERMINALS", "sv-amber", 15))
    for i, line in enumerate(["CN3, CN4 and CN5 are plug-in",
                              "headers whose screw plug is",
                              "not fitted. Solder each wire",
                              "to its pin tail on this side:",
                              "20 AWG, 18 AWG for CN3."]):
        a(T(1480, 196 + i * 21, line, "sv-sub", 14))
    a('</svg>')
    return "\n".join(s)


# --------------------------------------------------------------------------------------------
# Sheet B: the 14 plug-in jumper wires, Nucleo morpho -> RBT01 CN2
# --------------------------------------------------------------------------------------------
# (n, colour, y, nucleo pin, nucleo spot, signal, CN2 pin)
ROWS = [
    (1, "3v3", 200, "CN7-16", "inner column, 8th from top", "+3V3", 1),
    (2, "gnd", 244, "CN7-19", "outer column, 10th from top", "GND", 6),
    (3, "en", 314, "CN10-34", "outer column, 3rd from bottom", "PC4 · M1 enable / fault", 13),
    (4, "en", 358, "CN10-6", "outer column, 3rd from top", "PC5 · M2 enable / fault", 15),
    (5, "en", 402, "CN10-1", "inner column, 1st from top", "PC9 · M3 enable / fault", 16),
    (6, "en", 446, "CN10-2", "outer column, 1st from top", "PC8 · M4 enable / fault", 18),
    (7, "dir", 516, "CN7-38", "inner column, 1st from bottom", "PC0 · M1 direction", 23),
    (8, "dir", 560, "CN7-36", "inner column, 2nd from bottom", "PC1 · M2 direction", 24),
    (9, "dir", 604, "CN7-35", "outer column, 2nd from bottom", "PC2 · M3 direction", 19),
    (10, "dir", 648, "CN7-37", "outer column, 1st from bottom", "PC3 · M4 direction", 21),
    (11, "pwm", 718, "CN10-4", "outer column, 2nd from top", "PC6 · TIM3 CH1 · M1 speed", 32),
    (12, "pwm", 762, "CN10-29", "inner column, 5th from bottom", "PB5 · TIM3 CH2 · M2 speed", 33),
    (13, "pwm", 806, "CN7-34", "inner column, 3rd from bottom", "PB0 · TIM3 CH3 · M3 speed", 31),
    (14, "pwm", 850, "CN10-24", "outer column, 8th from bottom", "PB1 · TIM3 CH4 · M4 speed", 36),
]
CN2_ROLE = {1: "3V3 in", 6: "GND", 13: "M1 enable", 15: "M2 enable", 16: "M3 enable", 18: "M4 enable",
            23: "M1 direction", 24: "M2 direction", 19: "M3 direction", 21: "M4 direction",
            32: "M1 speed", 33: "M2 speed", 31: "M3 speed", 36: "M4 speed"}
GROUPS = [(176, "POWER · logic 3V3 and common ground (connect first)"),
          (290, "ENABLE / FAULT · one per motor"),
          (492, "DIRECTION · one per motor (wires 7 and 8 are the front wheels)"),
          (694, "SPEED · TIM3 PWM, 20 kHz")]


def ordinal(n):
    return f"{n}{'th' if 10 <= n % 100 <= 20 else {1: 'st', 2: 'nd', 3: 'rd'}.get(n % 10, 'th')}"


def sheet_b():
    W, H = 1800, 1130
    NX, NW = 40, 560
    RX, RW = 1080, 380
    TOP, BOT = 130, 960
    MID = (NX + NW + 12 + RX - 12) // 2  # marker column, halfway along each wire
    s = [f'<svg viewBox="0 0 {W} {H}" xmlns="http://www.w3.org/2000/svg" role="img" '
         'aria-label="Sheet B: jumper wires from the Nucleo to the RBT01 40-pin socket">', background(W, H)]
    a = s.append

    # Nucleo: parts drawn in a left column (x 62-262), pin labels right-aligned on the right half
    a(T(NX, 62, "NUCLEO-L073RZ + WI-FI SHIELD", "sv-ink", 30))
    a(T(NX, 86, "Hold it USB at the top. Morpho CN7 = left pin field, CN10 = right;", "sv-dark", 14))
    a(T(NX, 104, "“outer” = the column nearest the board edge. All wires use morpho pins.", "sv-dark", 14))
    a(f'<rect x="{NX}" y="{TOP}" width="{NW}" height="{BOT - TOP}" rx="18" fill="{BOARD_NUC}" stroke="#0b1630" stroke-width="3"/>')
    a(holes([(NX + 26, TOP + 26), (NX + NW - 26, TOP + 26), (NX + 26, BOT - 26), (NX + NW - 26, BOT - 26)]))
    cx = NX + 122
    a(f'<rect x="{cx - 28}" y="{TOP - 14}" width="56" height="26" rx="4" fill="#cbd5e1" stroke="#0b1630" stroke-width="2"/>')
    a(T(cx, TOP + 30, "ST-LINK USB · COM7", "sv-note", 11, "middle"))
    a(f'<rect x="{NX + 22}" y="{TOP + 50}" width="200" height="220" rx="10" fill="#0f5132" stroke="#7ee2a8" stroke-width="2" stroke-dasharray="6 4"/>')
    a(T(cx, TOP + 78, "X-NUCLEO-67W61M1", "sv-mono", 12, "middle"))
    a(T(cx, TOP + 96, "stacked on the", "sv-note", 11, "middle"))
    a(T(cx, TOP + 111, "Arduino sockets", "sv-note", 11, "middle"))
    a(f'<rect x="{cx - 40}" y="{TOP + 130}" width="80" height="60" rx="5" fill="#0b1630" stroke="#7ee2a8" stroke-width="1.5"/>')
    a(T(cx, TOP + 165, "ST67W61", "sv-mono", 13, "middle"))
    a(T(cx, TOP + 230, "no wires from", "sv-note", 11, "middle"))
    a(T(cx, TOP + 245, "its sockets", "sv-note", 11, "middle"))
    a(f'<rect x="{cx - 70}" y="{TOP + 310}" width="140" height="80" rx="6" fill="#0b1630" stroke="#5b76b8" stroke-width="2"/>')
    a(T(cx, TOP + 346, "STM32L073RZ", "sv-mono", 14, "middle"))
    a(T(cx, TOP + 366, "Cortex-M0+ · 32 MHz", "sv-note", 11, "middle"))
    wy = TOP + 430
    a(f'<rect x="{NX + 22}" y="{wy}" width="200" height="{BOT - 46 - wy}" rx="8" fill="#0b1630" stroke="#f5b301" stroke-width="1.5"/>')
    warn = [("sv-amber", "LEAVE PB4 / D5 EMPTY"), ("sv-note", "It is the Wi-Fi shield's"),
            ("sv-note", "CHIP_EN line."), ("", ""), ("sv-amber", "FRONT DIRECTION WIRES"),
            ("sv-note", "Wires 7 and 8 come"), ("sv-note", "straight from CN7-38"),
            ("sv-note", "and CN7-36. On the old"), ("sv-note", "board they went through"),
            ("sv-note", "the shield's A5 / A4"), ("sv-note", "sockets, never reached"),
            ("sv-note", "the driver, and the front"), ("sv-note", "wheels only turned"),
            ("sv-note", "forward.")]
    for i, (cls, txt) in enumerate(warn):
        if txt:
            a(T(NX + 36, wy + 28 + i * 19, txt, cls, 13 if cls == "sv-amber" else 12.5))

    # RBT01
    a(T(RX + RW, 62, "RBT01 · CN2", "sv-ink", 30, "end"))
    a(T(RX + RW, 86, "Female socket on the underside:", "sv-dark", 14, "end"))
    a(T(RX + RW, 104, "the wire's male end plugs in.", "sv-dark", 14, "end"))
    a(f'<rect x="{RX}" y="{TOP}" width="{RW}" height="{BOT - TOP}" rx="18" fill="{BOARD_RBT}" stroke="#0b1630" stroke-width="3"/>')
    a(holes([(RX + 26, TOP + 26), (RX + RW - 26, TOP + 26), (RX + 26, BOT - 26), (RX + RW - 26, BOT - 26)]))
    a(T(RX + 30, BOT - 66, "Row and spot are counted from", "sv-note", 12))
    a(T(RX + 30, BOT - 49, "pin 1, the square pad, so they", "sv-note", 12))
    a(T(RX + 30, BOT - 32, "hold from either side of the board.", "sv-note", 12))

    for gy, txt in GROUPS:
        a(T(MID, gy, txt, "sv-dark", 12.5, "middle", ' font-weight="600"'))
    for n, col, y, npin, nspot, sig, cpin in ROWS:
        c = C[col]
        a(T(NX + NW - 24, y - 2, f"{npin} · {nspot}", "sv-mono", 12.5, "end"))
        a(T(NX + NW - 24, y + 13, sig, "sv-note", 11, "end"))
        row = "edge row" if cpin % 2 else "inner row"
        spot = (cpin + 1) // 2
        a(T(RX + 24, y - 2, f"CN2 pin {cpin} · {row}, spot {spot}", "sv-mono", 12.5))
        a(T(RX + 24, y + 13, CN2_ROLE[cpin], "sv-note", 11))
        a(f'<path class="wire" stroke="{c}" d="M{NX + NW + 12} {y}H{RX - 12}"/>')
        a(f'<rect x="{NX + NW - 12}" y="{y - 8}" width="24" height="16" rx="3" fill="{c}" stroke="#fff" stroke-width="2"/>')
        a(f'<rect x="{RX - 12}" y="{y - 8}" width="24" height="16" rx="3" fill="{c}" stroke="#fff" stroke-width="2"/>')
        a(marker(MID, y, n))

    # CN2 map
    MX = 1520
    a(T(1660, 150, "CN2 MAP", "sv-ink", 24, "middle"))
    a(T(1600, 186, "inner", "sv-dark", 11.5, "middle"))
    a(T(1600, 199, "even", "sv-dark", 11.5, "middle"))
    a(T(1680, 186, "edge", "sv-dark", 11.5, "middle"))
    a(T(1680, 199, "odd", "sv-dark", 11.5, "middle"))
    a('<path d="M1724 206V902" stroke="#0b1630" stroke-width="3"/>')
    a(T(0, 0, "board edge", "sv-dark", 11.5, "middle", ' transform="translate(1740 554) rotate(90)"'))
    col_of = {r[6]: C[r[1]] for r in ROWS}
    special = {2: ("5V", "#fecaca"), 4: ("5V", "#fecaca"), 17: ("3V3", "#fed7aa")}
    gnds = {9, 14, 20, 25, 30, 34, 39}
    for spot in range(1, 21):
        y = 214 + (spot - 1) * 34
        a(T(MX + 30, y + 17, str(spot), "sv-dark", 11, "end"))
        for p, x in ((2 * spot, 1600), (2 * spot - 1, 1680)):
            if p in col_of:
                fill, tc, txt = col_of[p], ("#0b1630" if col_of[p] == C["pwm"] else "#ffffff"), str(p)
            elif p in gnds:
                fill, tc, txt = "#64748b", "#ffffff", f"{p} GND"
            elif p in special:
                fill, tc, txt = special[p][1], "#0b1630", f"{p} {special[p][0]}"
            else:
                fill, tc, txt = "#eef2f8", "#64748b", str(p)
            rx = 0 if p == 1 else 5
            sw = 3 if p == 1 else 1.5
            a(f'<rect x="{x - 34}" y="{y}" width="68" height="26" rx="{rx}" fill="{fill}" stroke="#0b1630" stroke-width="{sw}"/>')
            a(f'<text x="{x}" y="{y + 18}" font-size="12" text-anchor="middle" fill="{tc}" '
              f'font-family="IBM Plex Mono, Consolas, monospace" font-weight="600">{escape(txt)}</text>')
    a(T(MX - 4, 214 + 17, "spot", "sv-dark", 11, "end"))
    a(T(1640, 936, "Pin 1 = square pad (seen from the", "sv-dark", 12, "middle"))
    a(T(1640, 952, "component side). Beep test, no power:", "sv-dark", 12, "middle"))
    a(T(1640, 968, "pin 1 ↔ pin 17 beeps (both 3V3),", "sv-dark", 12, "middle"))
    a(T(1640, 984, "pin 6 ↔ CN3 GND beeps.", "sv-dark", 12, "middle"))

    # bottom notes
    a(T(40, 1012, "Plug-in jumper wires: female end on the Nucleo pin, male end into the RBT01 socket. Nothing is soldered on this sheet.", "sv-dark", 13.5))
    a(T(40, 1036, "Leave CN2 pins 2 and 4 (5 V) and every unlisted pin empty: they carry sensor and LED lines.", "sv-dark", 13.5))
    a(T(40, 1060, "Wiring check with power on: send MOTOR:1,30 then MOTOR:1,-30 and meter CN2 pin 23 to GND. It must switch between 0 V and 3.3 V (pins 24, 19, 21 for motors 2, 3, 4).", "sv-alert", 13.5))
    a('</svg>')
    return "\n".join(s)


# --------------------------------------------------------------------------------------------
# Page
# --------------------------------------------------------------------------------------------
CSS = """
:root {
  /* Layout: navy title band, then full-width wiring sheets on a blue-grey drafting ground, then three working cards. */
  --ground: #dfe6f1; --paper: #f4f7fb; --ink: #0b1630; --ink-soft: #4b5d80; --navy: #0b1630;
  --on-navy: #ffffff; --band-text: #a9bbe0; --amber: #f5b301; --rule: #c3cfe2; --accent: #2563eb;
  --code-fg: #e2e8f0; --ok: #15803d; --fix: #b45309; --bad: #b91c1c;
  --display: "Barlow Condensed", "Arial Narrow", sans-serif;
  --body: "IBM Plex Sans", "Segoe UI", sans-serif;
  --mono: "IBM Plex Mono", Consolas, monospace;
  color-scheme: light;
}
body { margin: 0; background: var(--ground); color: var(--ink); font-family: var(--body); font-size: 15px; line-height: 1.5; }
.png { width: 1800px; }
.gutter { padding-left: max(20px, env(safe-area-inset-left, 0px)); padding-right: max(20px, env(safe-area-inset-right, 0px)); }
.band { background: var(--navy); color: var(--on-navy); border-bottom: 4px solid var(--accent); }
.band .gutter { padding-block: 18px 16px; }
.band h1 { font-family: var(--display); font-weight: 700; font-size: clamp(28px, 4vw, 40px); line-height: 1.05; margin: 0; text-transform: uppercase; text-wrap: balance; }
.band p { margin: 8px 0 12px; color: var(--band-text); max-width: 120ch; }
.badge { display: inline-block; background: var(--amber); color: var(--navy); font-family: var(--display); font-weight: 700; letter-spacing: .06em; font-size: 17px; padding: 6px 14px; border-radius: 999px; }
main.gutter { padding-block: 16px 24px; display: grid; gap: 22px; }
.legend { display: flex; flex-wrap: wrap; gap: 8px 22px; font-size: 14px; color: var(--ink-soft); }
.legend span { display: inline-flex; align-items: center; gap: 7px; }
.legend i { width: 26px; border-top: 4px solid; display: inline-block; }
.block { display: grid; gap: 8px; min-width: 0; }
.block h2 { font-family: var(--display); font-weight: 700; font-size: 26px; letter-spacing: .02em; text-transform: uppercase; margin: 0; }
.block > p { margin: 0; color: var(--ink-soft); max-width: 110ch; }
.sheet { margin: 0; border-radius: 14px; outline: 1px solid var(--rule); overflow-x: auto; background: var(--ground); }
.sheet svg { display: block; width: 100%; min-width: 980px; height: auto; }
.png .sheet svg { min-width: 0; }
.cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(330px, 1fr)); gap: 16px; align-items: start; }
.card { background: var(--paper); border: 1px solid var(--rule); border-radius: 12px; padding: 16px 18px 18px; display: grid; gap: 10px; min-width: 0; }
.card h3 { font-family: var(--display); font-weight: 700; font-size: 21px; text-transform: uppercase; letter-spacing: .03em; margin: 0; }
.card p, .card li { margin: 0; }
.card ul, .card ol { margin: 0; padding-left: 20px; display: grid; gap: 4px; }
pre { margin: 0; background: var(--navy); color: var(--code-fg); font-family: var(--mono); font-size: 13.5px; line-height: 1.6; padding: 12px 14px; border-radius: 8px; overflow-x: auto; }
pre b { color: var(--amber); font-weight: 600; }
code { font-family: var(--mono); font-size: .92em; background: #e3e9f3; padding: 1px 5px; border-radius: 4px; }
table { border-collapse: collapse; width: 100%; font-size: 14px; }
th, td { text-align: left; vertical-align: top; padding: 6px 8px; border-top: 1px solid var(--rule); }
th { font-size: 12px; text-transform: uppercase; letter-spacing: .06em; color: var(--ink-soft); border-top: 0; }
.tbl { overflow-x: auto; }
.ok { color: var(--ok); font-weight: 600; } .fix { color: var(--fix); font-weight: 600; } .bad { color: var(--bad); font-weight: 600; }
footer { background: var(--navy); color: var(--band-text); font-size: 13.5px; line-height: 1.55; }
footer .gutter { padding-block: 14px 18px; }
footer p { margin: 0 0 6px; } footer strong { color: var(--on-navy); }
footer code { background: #1c2c52; color: var(--on-navy); }
.sv-ink { font-family: var(--display); font-weight: 700; fill: var(--ink); }
.sv-sub { font-family: var(--body); fill: #bfd0f5; }
.sv-amber { font-family: var(--body); font-weight: 600; fill: var(--amber); letter-spacing: .04em; }
.sv-mono { font-family: var(--mono); font-weight: 600; fill: #ffffff; }
.sv-note { font-family: var(--body); fill: #b9c8ea; }
.sv-faint { font-family: var(--body); fill: #9aa8c8; }
.sv-dark { font-family: var(--body); fill: var(--ink-soft); }
.sv-darkmono { font-family: var(--mono); font-weight: 600; fill: var(--ink); }
.sv-alert { font-family: var(--body); font-weight: 600; fill: #9a3412; }
.wire { fill: none; stroke-width: 4.5; stroke-linecap: round; stroke-linejoin: round; }
.wire.fat { stroke-width: 7; }
.marker circle { fill: #dc2626; stroke: #ffffff; stroke-width: 2.5; }
.marker text { font-family: var(--body); font-weight: 600; font-size: 13px; fill: #ffffff; text-anchor: middle; dominant-baseline: central; }
@media print { .sheet { overflow: visible; } .sheet svg { min-width: 0; } }
"""

HEAD = """<title>{title}</title>
<meta name="claude:viewport-fit-sides" content="cover">
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Barlow+Condensed:wght@600;700&family=IBM+Plex+Mono:wght@500;600&family=IBM+Plex+Sans:wght@400;500;600&display=swap">
<style>{css}</style>
"""

LEGEND = [(C["red"], "red motor lead"), (C["blk"], "black motor lead"), (C["vm"], "motor power +"),
          (C["gnd"], "GND"), (C["3v3"], "+3V3 logic"), (C["en"], "enable / fault"),
          (C["dir"], "direction"), (C["pwm"], "speed (PWM)")]
LEGEND_HTML = '<div class="legend">' + "".join(
    f'<span><i style="border-color:{c}"></i>{escape(t)}</span>' for c, t in LEGEND) + '</div>'

BAND = """<header class="band"><div class="gutter">
<h1>{h1}</h1>
<p>{lede}</p>
<span class="badge">RULE · BLACK → 2A / 2B · RED → 1A / 1B</span>
</div></header>"""

FOOTER = """<footer><div class="gutter">
<p><strong>Signal chain:</strong> STM32L073RZ TIM3 CH1–CH4 (PC6 / PB5 / PB0 / PB1) for speed, PC0–PC3 for direction, PC4 / PC5 / PC9 / PC8 for enable → RBT01 CN2 → STSPIN948 U2 (M1, M2 → CN4) and U3 (M3, M4 → CN5). Motor power → CN3. Forward with the default settings drives the 2 pin high, so current enters each motor's black lead.</p>
<p><strong>Sources:</strong> X-STM32MP-RBT01 user manual UM3482 Rev 1 (Table 11 CN2 pinout, Table 1 mode jumpers, schematic sheets 4–5 for CN3/CN4/CN5 and test points; its Table 2 wrongly lists the rear motors on CN3) · STSPIN948 datasheet DS14271 (Table 9 truth table; PH inputs have internal pull-downs) · NUCLEO-L073RZ user manual UM1724 Table 30 · X-NUCLEO-67W61M1 user manual UM3449 Tables 9 and 12 · firmware <code>17_Rover_MotorDriver_WiFi_ST67W61/firmware/Core/Inc/hardware_config.h</code> and <code>motor_control.c</code>.</p>
</div></footer>"""

CARDS = """<section class="block">
<h2>Before power, then firmware setup</h2>
<p>Do these in order with the wheels off the ground. Commands go over the Nucleo's USB serial port (COM7, 115 200 baud), from the web serial tool or any terminal. Each command keeps a wheel turning for about 1.5 s, then the firmware stops it by itself.</p>
<div class="cards">
<article class="card">
<h3>1 · Meter checks, no power</h3>
<ul>
<li>Across one motor's two pins: a few ohms (the winding). It may flicker as the shaft turns.</li>
<li>Middle pair of each block, 1B ↔ 1A: not a short.</li>
<li>Any motor pin ↔ GND: open.</li>
<li>CN3 + ↔ GND: not a short. The reading climbs as the 330 µF capacitor charges.</li>
<li>CN3 GND ↔ CN2 pin 6: beeps. CN2 pin 1 ↔ pin 17: beeps (both 3V3).</li>
<li>Mode jumpers: J6 1-2, J7 2-3, J8 1-2, J11 1-2, J12 2-3, J13 1-2.</li>
</ul>
<p>Then power up in this order: Nucleo USB first, then motor power at 7.4–9 V with a 1 A current limit.</p>
</article>
<article class="card">
<h3>2 · Set every wheel's direction</h3>
<p>Start from a known state. <code>FLIP:n,1</code> is the default direction that matches the soldering rule.</p>
<pre><b>LINK:0</b>        each wheel set on its own
<b>MODE:MEC</b>      mecanum mixing
<b>SFLIP:0</b>
<b>FLIP:1,1</b>
<b>FLIP:2,1</b>
<b>FLIP:3,1</b>
<b>FLIP:4,1</b></pre>
<p>Then test each motor, n = 1 to 4:</p>
<pre><b>MOTOR:n,30</b>    should roll forward
<b>MOTOR:n,-30</b>   should roll backward</pre>
<div class="tbl"><table>
<thead><tr><th>You see</th><th>Do</th></tr></thead>
<tbody>
<tr><td>Forward, then backward</td><td class="ok">Nothing, it's right</td></tr>
<tr><td>Backward, then forward</td><td class="fix"><code>INVERT:n</code></td></tr>
<tr><td>Same way both times</td><td class="bad">Its direction wire isn't arriving (card 3)</td></tr>
<tr><td>A different wheel turns</td><td class="bad">Wires on the wrong motor's pins: re-solder</td></tr>
<tr><td>Nothing turns</td><td class="bad">Check its speed and enable wires, then <code>STATUS</code></td></tr>
</tbody></table></div>
</article>
<article class="card">
<h3>3 · Whole rover, then save</h3>
<pre><b>FORWARD</b>       all four roll forward
<b>BACKWARD</b>      all four roll backward
<b>RIGHT</b>         spins clockwise, seen from above
<b>STRAFE:40</b>     slides to the right
<b>SAVE</b>          reply OK:SAVED, kept after power-off</pre>
<ul>
<li>Slides left on <code>STRAFE:40</code>: send <code>SFLIP:1</code>.</li>
<li>Twists instead of sliding: the mecanum wheels are on the wrong corners. Swap the two front wheels with each other and the two rear wheels with each other.</li>
<li><strong>Same way both times</strong> (the old front-wheel fault): meter that motor's direction pin on CN2 to GND while you send <code>MOTOR:n,30</code> then <code>MOTOR:n,-30</code>. Pins: M1 = 23, M2 = 24, M3 = 19, M4 = 21. It must switch between 0 V and 3.3 V. If it stays at 0 V, the driver's internal pull-down is winning because the wire isn't connected: re-seat it on the Nucleo (CN7-38, CN7-36, CN7-35, CN7-37).</li>
</ul>
</article>
</div>
</section>"""


def page(title, h1, lede, body_class, blocks):
    head = HEAD.format(title=escape(title), css=CSS)
    cls = f' class="{body_class}"' if body_class else ""
    # The Artifact skeleton supplies <body>; a body class is only needed for the PNG pages.
    opener = f'<div{cls} id="page">' if body_class else '<div id="page">'
    return (head + opener + BAND.format(h1=h1, lede=lede) +
            f'<main class="gutter">{LEGEND_HTML}{"".join(blocks)}</main>' + FOOTER + '</div>')


def block(h2, p, svg):
    return f'<section class="block"><h2>{h2}</h2><p>{p}</p><figure class="sheet">{svg}</figure></section>'


A_TXT = ("Solder each motor's two leads to the pin tails on the underside of the board. "
         "Numbers 1–8 are the eight motor joints, 9–10 the motor power.")
B_TXT = ("Fourteen plug-in jumper wires from the Nucleo's morpho pins to the RBT01's 40-pin socket. "
         "Same signals as before; the front direction wires now come straight from the Nucleo.")
H1 = "GrassKit rover — X-STM32MP-RBT01 (XMP$RBT01A) → 4 × TT gear motors"
LEDE = ("Build sheet for the replacement motor board. Sheet A shows where each motor's two wires are soldered "
        "on the underside of the RBT01 and where motor power goes. Sheet B shows the 14 plug-in jumper wires from "
        "the NUCLEO-L073RZ. Below them: meter checks and the serial commands that set each wheel's direction.")

if __name__ == "__main__":
    a_svg, b_svg = sheet_a(), sheet_b()
    full = page("RBT01A Rover Build Sheet", H1, LEDE, "", [
        block("Sheet A · Motors and power, soldered", A_TXT, a_svg),
        block("Sheet B · Logic wires, plugged in", B_TXT, b_svg),
        CARDS])
    (HERE / "rbt01a_build_sheet.html").write_text(full, encoding="utf-8")
    pa = page("RBT01A Sheet A", H1 + " · sheet A", "Sheet A of 2, for the person soldering. Sheet B shows the 14 "
              "plug-in logic wires.", "png", [block("Sheet A · Motors and power, soldered", A_TXT, a_svg)])
    pb = page("RBT01A Sheet B", H1 + " · sheet B", "Sheet B of 2. Sheet A shows the motor and power soldering.",
              "png", [block("Sheet B · Logic wires, plugged in", B_TXT, b_svg)])
    for name, html in (("rbt01a_sheet_a.html", pa), ("rbt01a_sheet_b.html", pb)):
        (HERE / name).write_text("<!doctype html><meta charset='utf-8'>" + html, encoding="utf-8")
    print("wrote rbt01a_build_sheet.html, rbt01a_sheet_a.html, rbt01a_sheet_b.html")
