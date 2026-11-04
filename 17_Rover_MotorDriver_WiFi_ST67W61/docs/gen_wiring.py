"""Generates wiring.html (SVG wiring sheet) for the GrassKit Rover bench setup:
NUCLEO-L073RZ + X-NUCLEO-67W61M1 (stacked) -> X-STM32MP-RBT01 -> 4 DC motors.
Pin data mirrors firmware/Core/Inc/hardware_config.h. Render with render_wiring.ps1."""
from pathlib import Path

C = {"3v3": "#f97316", "gnd": "#111827", "en": "#7c3aed", "dir": "#16a34a",
     "pwm": "#eab308", "vm": "#e11d48", "mot": "#64748b"}

# (num, color, y, nucleo_main, nucleo_note, rbt_main, rbt_note, via_shield)
ROWS = [
    (1, "3v3", 200, "ST67 CN6 · 4th hole from top", "3V3 · same net as CN7-16 (inner, 8th from top)",
     "CN2 pin 1 · inner, 1st from L", "3V3 logic supply · square pad", True),
    (2, "gnd", 244, "CN7 outer · 10th from top", "GND · CN7-19 (or CN10-20 / ST67 CN6 GND)",
     "CN2 pin 6 · outer, 3rd from L", "GND · common ground", False),
    (3, "en", 314, "CN10 outer · 3rd from bottom", "PC4 · CN10-34 → M1 EN/FAULT",
     "CN2 pin 13 · inner, 7th from L", "M1 EN/FAULT · U2", False),
    (4, "en", 358, "CN10 outer · 3rd from top", "PC5 · CN10-6 → M2 EN/FAULT",
     "CN2 pin 15 · inner, 8th from L", "M2 EN/FAULT · U2", False),
    (5, "en", 402, "CN10 inner · 1st from top", "PC9 · CN10-1 (above ST67 CN5) → M3 EN",
     "CN2 pin 16 · outer, 8th from L", "M3 EN/FAULT · U3", False),
    (6, "en", 446, "CN10 outer · 1st from top", "PC8 · CN10-2 → M4 EN/FAULT",
     "CN2 pin 18 · outer, 9th from L", "M4 EN/FAULT · U3", False),
    (7, "dir", 516, "ST67 CN8 · 1st hole from bottom", "PC0 · A5 · CN7-38 → M1 DIR",
     "CN2 pin 23 · inner, 12th from L", "M1 DIR", True),
    (8, "dir", 560, "ST67 CN8 · 2nd hole from bottom", "PC1 · A4 · CN7-36 → M2 DIR",
     "CN2 pin 24 · outer, 12th from L", "M2 DIR", True),
    (9, "dir", 604, "CN7 outer · 2nd from bottom", "PC2 · CN7-35 → M3 DIR",
     "CN2 pin 19 · inner, 10th from L", "M3 DIR", False),
    (10, "dir", 648, "CN7 outer · 1st from bottom", "PC3 · CN7-37 → M4 DIR",
     "CN2 pin 21 · inner, 11th from L", "M4 DIR", False),
    (11, "pwm", 718, "CN10 outer · 2nd from top", "PC6 · TIM3_CH1 · CN10-4 → M1 PWM",
     "CN2 pin 32 · outer, 5th from R", "M1 PWM (16th from L)", False),
    (12, "pwm", 762, "ST67 CN9 · 5th hole from bottom", "PB5 · D4 · TIM3_CH2 · CN10-29 → M2 PWM",
     "CN2 pin 33 · inner, 4th from R", "M2 PWM (17th from L)", True),
    (13, "pwm", 806, "ST67 CN8 · 3rd hole from bottom", "PB0 · A3 · TIM3_CH3 · CN7-34 → M3 PWM",
     "CN2 pin 31 · inner, 5th from R", "M3 PWM (16th from L)", True),
    (14, "pwm", 850, "CN10 outer · 8th from bottom", "PB1 · TIM3_CH4 · CN10-24 → M4 PWM",
     "CN2 pin 36 · outer, 3rd from R", "M4 PWM (18th from L)", False),
]
GROUPS = [(176, "POWER · logic 3V3 + common ground (connect first)"),
          (290, "EN / FAULT · STM32 drives high, STSPIN948 pulls low on fault"),
          (492, "DIR · direction, one GPIO per wheel"),
          (694, "PWM · TIM3 20 kHz, phase-interleaved L / R")]

NX, NW = 40, 440          # Nucleo board
RX, RW = 1080, 440        # RBT01 board
TOP, BOT = 130, 1000
W, H = 1800, 1180

s = []
a = s.append
a(f'<svg viewBox="0 0 {W} {H}" xmlns="http://www.w3.org/2000/svg" role="img">')
a(f'<rect width="{W}" height="{H}" fill="#dfe6f1"/>')
a('<g stroke="#cfd9e8" stroke-width="1">' +
  "".join(f'<path d="M0 {y}H{W}"/>' for y in range(115, H, 115)) +
  "".join(f'<path d="M{x} 0V{H}"/>' for x in range(200, W, 200)) + '</g>')

# ---------- Nucleo + ST67W61 ----------
a(f'<text class="sv-ink" x="{NX}" y="80" font-size="32">NUCLEO-L073RZ + X-NUCLEO-67W61M1</text>')
a(f'<text class="sv-dark" x="{NX}" y="104" font-size="13">ST67W61 Wi-Fi 6 shield on top · USB at TOP · "outer" = edge column of Morpho</text>')
a(f'<rect x="{NX}" y="{TOP}" width="{NW}" height="{BOT-TOP}" rx="18" fill="#1d3a86" stroke="#0b1630" stroke-width="3"/>')
a(f'<g fill="#d4a537"><circle cx="{NX+26}" cy="{TOP+26}" r="10"/><circle cx="{NX+NW-26}" cy="{TOP+26}" r="10"/>'
  f'<circle cx="{NX+26}" cy="{BOT-26}" r="10"/><circle cx="{NX+NW-26}" cy="{BOT-26}" r="10"/></g>')
# USB connector at top
a(f'<rect x="{NX+90}" y="{TOP-14}" width="56" height="26" rx="4" fill="#cbd5e1" stroke="#0b1630" stroke-width="2"/>')
a(f'<text class="sv-note" x="{NX+118}" y="{TOP+30}" font-size="11" text-anchor="middle">ST-LINK USB</text>')
# Shield block
a(f'<rect x="{NX+22}" y="{TOP+60}" width="150" height="250" rx="10" fill="#0f5132" stroke="#7ee2a8" stroke-width="2" stroke-dasharray="6 4"/>')
a(f'<text class="sv-mono" x="{NX+97}" y="{TOP+86}" font-size="12" text-anchor="middle">X-NUCLEO-67W61M1</text>')
a(f'<text class="sv-note" x="{NX+97}" y="{TOP+104}" font-size="11" text-anchor="middle">stacked on Arduino</text>')
a(f'<text class="sv-note" x="{NX+97}" y="{TOP+119}" font-size="11" text-anchor="middle">CN5 / CN6 / CN8 / CN9</text>')
a(f'<rect x="{NX+57}" y="{TOP+140}" width="80" height="60" rx="5" fill="#0b1630" stroke="#7ee2a8" stroke-width="1.5"/>')
a(f'<text class="sv-mono" x="{NX+97}" y="{TOP+175}" font-size="13" text-anchor="middle">ST67W61</text>')
a(f'<path d="M{NX+60} {TOP+250} l12 -16 l12 16 l12 -16 l12 16 l12 -16 l12 16" class="thin" stroke="#7fd3dd"/>')
a(f'<text class="sv-note" x="{NX+97}" y="{TOP+272}" font-size="11" text-anchor="middle">antenna (bottom)</text>')
a(f'<text class="sv-note" x="{NX+97}" y="{TOP+292}" font-size="10.5" text-anchor="middle">SPI: D10-D13 · D3 · D5 · D6</text>')
# MCU
a(f'<rect x="{NX+40}" y="{TOP+370}" width="114" height="90" rx="6" fill="#0b1630" stroke="#5b76b8" stroke-width="2"/>')
a(f'<text class="sv-mono" x="{NX+97}" y="{TOP+410}" font-size="14" text-anchor="middle">STM32L073RZ</text>')
a(f'<text class="sv-note" x="{NX+97}" y="{TOP+430}" font-size="11" text-anchor="middle">Cortex-M0+ · 32 MHz</text>')
a(f'<text class="sv-sub" x="{NX+28}" y="{TOP+500}" font-size="13">TIM3 · 4 × PWM</text>')
a(f'<text class="sv-note" x="{NX+28}" y="{TOP+518}" font-size="12">20 kHz · CH2/CH4 inverted</text>')
a(f'<text class="sv-note" x="{NX+28}" y="{TOP+536}" font-size="12">35 ms staggered start</text>')
a(f'<text class="sv-note" x="{NX+28}" y="{TOP+554}" font-size="12">USART2 → WebApp (USB)</text>')
# Warning
wy = 880
a(f'<rect x="{NX+22}" y="{wy}" width="{NW-44}" height="84" rx="8" fill="#0b1630" stroke="#f5b301" stroke-width="1.5"/>')
a(f'<text x="{NX+38}" y="{wy+24}" font-size="13" fill="#f5b301" font-family="IBM Plex Sans, Segoe UI, sans-serif" font-weight="600">'
  'Leave PB4 / D5 EMPTY: it is WIFI_CHIP_EN on the shield</text>')
a(f'<text class="sv-note" x="{NX+38}" y="{wy+44}" font-size="12">CN10 inner 6th from bottom = ST67 CN9 6th hole from bottom.</text>')
a(f'<text class="sv-note" x="{NX+38}" y="{wy+62}" font-size="12">Also keep off D3, D6, D10-D13 (shield SPI / RDY / BOOT).</text>')

# ---------- RBT01 ----------
a(f'<text class="sv-ink" x="{RX+RW}" y="80" font-size="32" text-anchor="end">X-STM32MP-RBT01</text>')
a(f'<text class="sv-dark" x="{RX+RW}" y="104" font-size="13" text-anchor="end">2 × STSPIN948 · CN2 along TOP, capacitor LEFT · inner row = odd pins, outer = even</text>')
a(f'<rect x="{RX}" y="{TOP}" width="{RW}" height="{BOT-TOP}" rx="18" fill="#7f1d1d" stroke="#0b1630" stroke-width="3"/>')
a(f'<g fill="#d4a537"><circle cx="{RX+26}" cy="{TOP+26}" r="10"/><circle cx="{RX+RW-26}" cy="{TOP+26}" r="10"/>'
  f'<circle cx="{RX+26}" cy="{BOT-26}" r="10"/><circle cx="{RX+RW-26}" cy="{BOT-26}" r="10"/></g>')
a(f'<text class="sv-sub" x="{RX+48}" y="{TOP+30}" font-size="12" fill="#fecaca">40-pin header CN2 (left edge here)</text>')

# Motor terminals
terms = [("CN5 · U3 · rear axle", [(220, "OUTB4"), (260, "OUTA4"), (380, "OUTB3"), (420, "OUTA3")],
          [(240, "M4", "Rear-Right"), (400, "M3", "Rear-Left")]),
         ("CN4 · U2 · front axle", [(580, "OUTB2"), (620, "OUTA2"), (740, "OUTB1"), (780, "OUTA1")],
          [(600, "M2", "Front-Right"), (760, "M1", "Front-Left")])]
tx = RX + RW
for title, pins, motors in terms:
    a(f'<rect x="{tx-80}" y="{pins[0][0]-22}" width="66" height="{pins[-1][0]-pins[0][0]+44}" rx="6" fill="#15803d" stroke="#0b1630" stroke-width="2"/>')
    mid = (pins[0][0] + pins[-1][0]) / 2
    a(f'<text class="sv-mono" font-size="12" text-anchor="middle" transform="translate({tx-96} {mid}) rotate(-90)">{title}</text>')
    a(f'<text class="sv-note" font-size="11" fill="#fecaca" text-anchor="middle" transform="translate({tx-112} {mid}) rotate(-90)">screws 1-2 bottom · 3-4 top</text>')
    for y, lbl in pins:
        a(f'<circle cx="{tx-47}" cy="{y}" r="9" fill="#cbd5e1" stroke="#0b1630" stroke-width="1.5"/>'
          f'<path d="M{tx-53} {y-6}l12 12" stroke="#0b1630" stroke-width="2"/>')
        a(f'<text class="sv-darkmono" x="{tx+20}" y="{y-7}" font-size="10.5">{lbl}</text>')
        a(f'<rect x="{tx-12}" y="{y-8}" width="24" height="16" rx="3" fill="{C["mot"]}" stroke="#fff" stroke-width="2"/>')
    for cy, m, name in motors:
        mx = 1660
        for dy in (-20, 20):
            a(f'<path class="wire" stroke="{C["mot"]}" d="M{tx+12} {cy+dy}H{mx-40}"/>')
        a(f'<circle cx="{mx}" cy="{cy}" r="40" fill="#1f2937" stroke="#0b1630" stroke-width="3"/>')
        a(f'<circle cx="{mx}" cy="{cy}" r="26" fill="#374151" stroke="#9ca3af" stroke-width="2"/>')
        a(f'<text class="sv-mono" x="{mx}" y="{cy+6}" font-size="17" text-anchor="middle">{m}</text>')
        a(f'<text class="sv-darkmono" x="{mx}" y="{cy+58}" font-size="13" text-anchor="middle">{name}</text>')
a(f'<text class="sv-dark" x="1660" y="880" font-size="12" text-anchor="middle">Wheel spins backwards?</text>')
a(f'<text class="sv-dark" x="1660" y="896" font-size="12" text-anchor="middle">Use "Flip M1..M4" in the</text>')
a(f'<text class="sv-dark" x="1660" y="912" font-size="12" text-anchor="middle">WebApp (INVERT:n), no rewiring.</text>')

# Capacitor + CN3
a(f'<circle cx="{RX+330}" cy="{BOT-110}" r="34" fill="#cbd5e1" stroke="#0b1630" stroke-width="2.5"/>')
a(f'<circle cx="{RX+330}" cy="{BOT-110}" r="22" fill="none" stroke="#64748b" stroke-width="2"/>')
a(f'<text class="sv-darkmono" x="{RX+330}" y="{BOT-106}" font-size="11" text-anchor="middle">330µF</text>')
a(f'<text class="sv-note" x="{RX+330}" y="{BOT-62}" font-size="11" fill="#fecaca" text-anchor="middle">bulk cap C2</text>')
a(f'<rect x="{RX+50}" y="{BOT-70}" width="120" height="50" rx="6" fill="#15803d" stroke="#0b1630" stroke-width="2"/>')
for x, lbl in ((RX+80, "VM"), (RX+140, "GND")):
    a(f'<circle cx="{x}" cy="{BOT-48}" r="10" fill="#cbd5e1" stroke="#0b1630" stroke-width="1.5"/>'
      f'<path d="M{x-6} {BOT-54}l12 12" stroke="#0b1630" stroke-width="2"/>')
    a(f'<text class="sv-mono" x="{x}" y="{BOT-78}" font-size="11" text-anchor="middle">{lbl}</text>')
a(f'<text class="sv-mono" x="{RX+190}" y="{BOT-50}" font-size="12">CN3 · motor power</text>')
a(f'<text class="sv-note" x="{RX+190}" y="{BOT-34}" font-size="11" fill="#fecaca">left = VM(+) · right = GND</text>')
a(f'<rect x="{RX+68}" y="{BOT-12}" width="24" height="16" rx="3" fill="{C["vm"]}" stroke="#fff" stroke-width="2"/>')
a(f'<rect x="{RX+128}" y="{BOT-12}" width="24" height="16" rx="3" fill="{C["gnd"]}" stroke="#fff" stroke-width="2"/>')

# ---------- Rows / wires ----------
for gy, txt in GROUPS:
    a(f'<text class="sv-dark" x="780" y="{gy}" font-size="12.5" text-anchor="middle" font-weight="600">{txt}</text>')
for n, col, y, nm, nn, rm, rn, via in ROWS:
    c = C[col]
    a(f'<text class="sv-mono" x="{NX+NW-24}" y="{y-2}" font-size="13" text-anchor="end">{nm}</text>')
    a(f'<text class="sv-note" x="{NX+NW-24}" y="{y+13}" font-size="11" text-anchor="end">{nn}</text>')
    a(f'<text class="sv-mono" x="{RX+24}" y="{y-2}" font-size="13">{rm}</text>')
    a(f'<text class="sv-note" x="{RX+24}" y="{y+13}" font-size="11" fill="#fecaca">{rn}</text>')
    dash = ' wire-dash' if via else ''
    a(f'<path class="wire{dash}" stroke="{c}" d="M{NX+NW+12} {y}H{RX-12}"/>')
    a(f'<rect x="{NX+NW-12}" y="{y-8}" width="24" height="16" rx="3" fill="{c}" stroke="#fff" stroke-width="2"/>')
    a(f'<rect x="{RX-12}" y="{y-8}" width="24" height="16" rx="3" fill="{c}" stroke="#fff" stroke-width="2"/>')
    a(f'<g class="marker"><circle cx="600" cy="{y}" r="14"/><text x="600" y="{y}">{n}</text></g>')

# ---------- Bench PSU ----------
px, py = 620, 1040
a(f'<rect x="{px}" y="{py}" width="280" height="110" rx="12" fill="#0b1630" stroke="#334155" stroke-width="3"/>')
a(f'<text class="sv-mono" x="{px+20}" y="{py+30}" font-size="15">BENCH DC SUPPLY</text>')
a(f'<rect x="{px+20}" y="{py+44}" width="120" height="40" rx="4" fill="#052e16" stroke="#22c55e" stroke-width="1.5"/>')
a(f'<text x="{px+80}" y="{py+71}" font-size="20" fill="#4ade80" text-anchor="middle" font-family="IBM Plex Mono, Consolas, monospace">9.0 V</text>')
a(f'<text class="sv-note" x="{px+20}" y="{py+102}" font-size="11">set 8-12 V · limit ≈ 0.54 A</text>')
a(f'<circle cx="{px+250}" cy="{py+35}" r="11" fill="{C["vm"]}" stroke="#fff" stroke-width="2"/><text class="sv-mono" x="{px+222}" y="{py+40}" font-size="14">+</text>')
a(f'<circle cx="{px+250}" cy="{py+75}" r="11" fill="{C["gnd"]}" stroke="#fff" stroke-width="2"/><text class="sv-mono" x="{px+222}" y="{py+80}" font-size="14">−</text>')
a(f'<path class="wire fat" stroke="{C["vm"]}" d="M{px+261} {py+35}H{RX+80}V{BOT+4}"/>')
a(f'<path class="wire fat" stroke="{C["gnd"]}" d="M{px+261} {py+75}H{RX+140}V{BOT+4}"/>')
a(f'<g class="marker"><circle cx="980" cy="{py+35}" r="14"/><text x="980" y="{py+35}">15</text></g>')
a(f'<g class="marker"><circle cx="1020" cy="{py+75}" r="14"/><text x="1020" y="{py+75}">16</text></g>')
a(f'<text class="sv-alert" x="320" y="{py+40}" font-size="12.5" text-anchor="middle">Not 6 V (UVLO 5.2 V) · not 20 V (inrush trips 0.54 A)</text>')
a(f'<text class="sv-dark" x="320" y="{py+60}" font-size="12" text-anchor="middle">Motor GND and logic GND meet on the RBT01 (wire 2).</text>')
a(f'<text class="sv-dark" x="320" y="{py+78}" font-size="12" text-anchor="middle">Prop the chassis up: wheels free in the air for bench tests.</text>')
a('</svg>')
svg = "\n".join(s)

legend = [("#f97316", "+3V3 logic", ""), ("#111827", "GND", ""), ("#7c3aed", "EN / FAULT", ""),
          ("#16a34a", "DIR", ""), ("#eab308", "PWM", ""), ("#e11d48", "VM (bench +)", ""),
          ("#64748b", "motor leads", ""), ("#4b5d80", "dashed = plug into ST67W61 top socket", "dash")]
leg = "".join(f'<span><i class="{d}" style="border-color:{c}"></i>{t}</span>' for c, t, d in legend)

html = f"""<meta charset="utf-8">
<title>GrassKit Rover Wiring</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Barlow+Condensed:wght@600;700&family=IBM+Plex+Mono:wght@500;600&family=IBM+Plex+Sans:wght@400;500;600&display=swap">
<style>
:root {{ --ground:#dfe6f1; --ink:#0b1630; --ink-soft:#4b5d80; --navy:#0b1630; --amber:#f5b301;
  --display:"Barlow Condensed","Arial Narrow",sans-serif; --body:"IBM Plex Sans","Segoe UI",sans-serif;
  --mono:"IBM Plex Mono",Consolas,monospace; }}
body {{ margin:0; background:var(--ground); color:var(--ink); font-family:var(--body); width:1800px; }}
.band {{ background:var(--navy); color:#fff; border-bottom:4px solid #2563eb; }}
.band-inner, .wrap {{ padding:0 20px; box-sizing:border-box; }}
.band-inner {{ padding-block:18px 16px; }}
.band h1 {{ font-family:var(--display); font-weight:700; font-size:40px; line-height:1.05; margin:0; text-transform:uppercase; }}
.band p {{ margin:8px 0 12px; color:#a9bbe0; font-size:15px; max-width:120ch; line-height:1.5; }}
.badge {{ display:inline-block; background:var(--amber); color:var(--navy); font-family:var(--display); font-weight:700; letter-spacing:.06em; font-size:16px; padding:6px 14px; border-radius:999px; }}
.wrap {{ padding-block:16px 22px; display:grid; gap:16px; }}
.legend {{ display:flex; flex-wrap:wrap; gap:8px 22px; font-size:14px; color:var(--ink-soft); }}
.legend span {{ display:inline-flex; align-items:center; gap:7px; }}
.legend i {{ width:26px; border-top:4px solid; display:inline-block; }}
.legend i.dash {{ border-top-style:dashed; }}
.sheet {{ margin:0; border-radius:14px; outline:1px solid #c3cfe2; overflow:hidden; }}
.sheet svg {{ display:block; width:100%; height:auto; }}
footer {{ background:var(--navy); color:#a9bbe0; padding:14px 20px 18px; font-size:13.5px; line-height:1.55; }}
footer p {{ margin:0 0 6px; }} footer strong {{ color:#fff; }} footer code {{ font-family:var(--mono); color:#fff; font-size:12.5px; }}
.sv-ink {{ font-family:var(--display); font-weight:700; fill:var(--ink); }}
.sv-sub {{ font-family:var(--body); fill:#bfd0f5; }}
.sv-mono {{ font-family:var(--mono); font-weight:600; fill:#fff; }}
.sv-note {{ font-family:var(--body); fill:#a9bce6; }}
.sv-dark {{ font-family:var(--body); fill:var(--ink-soft); }}
.sv-darkmono {{ font-family:var(--mono); font-weight:600; fill:var(--ink); }}
.sv-alert {{ font-family:var(--body); font-weight:600; fill:#9a3412; }}
.wire {{ fill:none; stroke-width:4.5; stroke-linecap:round; stroke-linejoin:round; }}
.wire.fat {{ stroke-width:6; }}
.wire-dash {{ stroke-dasharray:11 7; }}
.thin {{ fill:none; stroke-width:2.5; stroke-linecap:round; }}
.marker circle {{ fill:#dc2626; stroke:#fff; stroke-width:2.5; }}
.marker text {{ font-family:var(--body); font-weight:600; font-size:13px; fill:#fff; text-anchor:middle; dominant-baseline:central; }}
</style>
<header class="band"><div class="band-inner">
<h1>GrassKit Rover — NUCLEO-L073RZ + X-NUCLEO-67W61M1 → X-STM32MP-RBT01 → 4 × DC wheel motors</h1>
<p>14 jumper wires carry logic between the stacked Nucleo / ST67W61 Wi-Fi 6 pair and the RBT01's 40-pin CN2 header; two heavy wires bring bench power into CN3; four motors land on CN4 (front) and CN5 (rear).
Each label gives the physical spot to count to: on the Nucleo hold the USB at the top, on the RBT01 hold CN2 along the top with the silver capacitor on the left.
Signals on the shield's Arduino pins can be taken from the ST67W61 top sockets (dashed) or the Morpho pin underneath.</p>
<span class="badge">ST67W61-SAFE PINOUT · M1 PWM ON PC6 · D5 LEFT FREE</span>
</div></header>
<main class="wrap">
<div class="legend">{leg}</div>
<figure class="sheet">{svg}</figure>
</main>
<footer>
<p><strong>Signal chain:</strong> STM32L073RZ TIM3 CH1-CH4 (PC6 / PB5 / PB0 / PB1) + DIR (PC0-PC3) + EN/FAULT (PC4 / PC5 / PC9 / PC8) → RBT01 CN2 → STSPIN948 U2 (M1, M2 → CN4) and U3 (M3, M4 → CN5). Bench supply → CN3 VM / GND. Nucleo on its own ST-LINK USB (WebApp via USART2, 115200 baud).</p>
<p><strong>Sources:</strong> NUCLEO-L073RZ user manual UM1724 (Fig. 20, Table 30) and MB1136 schematic · X-STM32MP-RBT01 user manual UM3482 (Fig. 2, 4, 13, Table 11) · X-NUCLEO-67W61M1 Arduino pin use (D3, D5, D6, D10-D13) · firmware <code>17_Rover_MotorDriver_WiFi_ST67W61/firmware/Core/Inc/hardware_config.h</code>.</p>
</footer>
"""
out = Path(__file__).with_name("wiring.html")
out.write_text(html, encoding="utf-8")
print("wrote", out)
