"""Build the GrassKit Rover project status report.  Usage: build_status_pdf.py <out.pdf>"""
import sys

from reportlab.lib.colors import HexColor, white
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.pdfmetrics import stringWidth
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (BaseDocTemplate, CondPageBreak, Flowable, Frame, KeepTogether,
                                NextPageTemplate, PageTemplate, Paragraph, Spacer, Table, TableStyle)

OUT = sys.argv[1]
REPORT_DATE = "12 October 2026"

# ---------------------------------------------------------------- fonts
FONTS = "C:/Windows/Fonts/"
pdfmetrics.registerFont(TTFont("GK", FONTS + "segoeui.ttf"))
pdfmetrics.registerFont(TTFont("GK-Bold", FONTS + "segoeuib.ttf"))
pdfmetrics.registerFont(TTFont("GK-Semi", FONTS + "seguisb.ttf"))
pdfmetrics.registerFont(TTFont("GK-Italic", FONTS + "segoeuii.ttf"))
pdfmetrics.registerFont(TTFont("GK-Light", FONTS + "segoeuil.ttf"))
pdfmetrics.registerFont(TTFont("GK-Mono", FONTS + "consola.ttf"))
pdfmetrics.registerFontFamily("GK", normal="GK", bold="GK-Semi", italic="GK-Italic", boldItalic="GK-Semi")

# ---------------------------------------------------------------- palette
INK = HexColor("#1B1F23")
MUTED = HexColor("#5C6670")
RULE = HexColor("#D8DEDA")
GREEN_DARK = HexColor("#1F5132")
GREEN = HexColor("#3C8D5A")
GREEN_TINT = HexColor("#EAF4EC")
GREEN_PALE = HexColor("#BFE0C8")
AMBER = HexColor("#B36B00")
AMBER_TINT = HexColor("#FCF1DA")
RED = HexColor("#A12A1E")
RED_TINT = HexColor("#FAE1DD")
GREY = HexColor("#4A5560")
GREY_TINT = HexColor("#EDF0F2")
PANEL = HexColor("#F6F8F7")

STATUS = {
    "Done": (HexColor("#1E6B3A"), HexColor("#DDF0E2")),
    "In progress": (AMBER, AMBER_TINT),
    "Open issue": (RED, RED_TINT),
    "Not started": (GREY, GREY_TINT),
}

# ---------------------------------------------------------------- page geometry
PAGE_W, PAGE_H = A4
MARGIN_X = 18 * mm
BAND_H = 46 * mm
FRAME_W = PAGE_W - 2 * MARGIN_X

# ---------------------------------------------------------------- styles
body = ParagraphStyle("body", fontName="GK", fontSize=9.6, leading=14.2, textColor=INK, spaceAfter=6,
                      allowWidows=0, allowOrphans=0)
lead = ParagraphStyle("lead", parent=body, fontSize=10.4, leading=15.4, spaceAfter=7)
h1 = ParagraphStyle("h1", fontName="GK-Semi", fontSize=17, leading=21, textColor=GREEN_DARK,
                    spaceBefore=17, spaceAfter=7, keepWithNext=1)
h2 = ParagraphStyle("h2", fontName="GK-Semi", fontSize=11.6, leading=15, textColor=INK,
                    spaceBefore=11, spaceAfter=4, keepWithNext=1)
bullet = ParagraphStyle("bullet", parent=body, leftIndent=13, bulletIndent=2, spaceAfter=3.4,
                        bulletFontName="GK", bulletFontSize=9.6)
caption = ParagraphStyle("caption", fontName="GK-Italic", fontSize=8.2, leading=11, textColor=MUTED,
                         spaceBefore=3, spaceAfter=9)
cell = ParagraphStyle("cell", fontName="GK", fontSize=8.7, leading=12, textColor=INK)
cell_head = ParagraphStyle("cell_head", parent=cell, fontName="GK-Semi", textColor=GREEN_DARK)
cell_mono = ParagraphStyle("cell_mono", parent=cell, fontName="GK-Mono", fontSize=8.3)
callout = ParagraphStyle("callout", parent=body, fontSize=9.6, leading=14, spaceAfter=4.5)
callout_head = ParagraphStyle("callout_head", fontName="GK-Semi", fontSize=8.2, leading=11,
                              textColor=GREEN, spaceAfter=4)


def P(text, style=body):
    return Paragraph(text, style)


def bullets(items, style=bullet):
    return [Paragraph(t, style, bulletText="\u2022") for t in items]


def mono(text):
    return f'<font name="GK-Mono" size="8.6">{text}</font>'


class Heading(Paragraph):
    """Paragraph that also adds a PDF outline entry."""

    def __init__(self, text, style, outline, level):
        super().__init__(text, style)
        self.outline = outline
        self.level = level


_sec = [0, 0]


def H1(title):
    _sec[0] += 1
    _sec[1] = 0
    n = _sec[0]
    return Heading(f'<font color="#3C8D5A">{n}</font>&nbsp;&nbsp;{title}', h1, f"{n}  {title}", 0)


def H2(title):
    _sec[1] += 1
    n = f"{_sec[0]}.{_sec[1]}"
    return Heading(f'<font color="#5C6670">{n}</font>&nbsp;&nbsp;{title}', h2, f"{n}  {title}", 1)


class Chip(Flowable):
    def __init__(self, text):
        super().__init__()
        self.text = text
        self.fg, self.bg = STATUS[text]
        self.w = stringWidth(text, "GK-Semi", 7.5) + 13
        self.h = 13.5

    def wrap(self, aw, ah):
        return self.w, self.h

    def draw(self):
        c = self.canv
        c.setFillColor(self.bg)
        c.roundRect(0, 0, self.w, self.h, 6.7, stroke=0, fill=1)
        c.setFillColor(self.fg)
        c.setFont("GK-Semi", 7.5)
        c.drawString(6.5, 3.9, self.text)


def table(rows, widths, mono_cols=(), head=True):
    data = []
    for r, row in enumerate(rows):
        out = []
        for i, v in enumerate(row):
            if isinstance(v, Flowable):
                out.append(v)
            elif head and r == 0:
                out.append(Paragraph(v, cell_head))
            else:
                out.append(Paragraph(v, cell_mono if i in mono_cols else cell))
        data.append(out)
    t = Table(data, colWidths=widths, repeatRows=1 if head else 0)
    style = [
        ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ("LEFTPADDING", (0, 0), (-1, -1), 6),
        ("RIGHTPADDING", (0, 0), (-1, -1), 6),
        ("TOPPADDING", (0, 0), (-1, -1), 4.6),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 5),
        ("LINEBELOW", (0, 0), (-1, -1), 0.45, RULE),
    ]
    if head:
        style += [("BACKGROUND", (0, 0), (-1, 0), GREEN_TINT), ("LINEBELOW", (0, 0), (-1, 0), 0.9, GREEN)]
    t.setStyle(TableStyle(style))
    t.spaceAfter = 9
    t.spaceBefore = 3
    return t


def callout_box(title, paragraphs):
    inner = [Paragraph(title.upper(), callout_head)] + paragraphs
    t = Table([[inner]], colWidths=[FRAME_W])
    t.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, -1), GREEN_TINT),
        ("LINEBEFORE", (0, 0), (0, -1), 3, GREEN),
        ("LEFTPADDING", (0, 0), (-1, -1), 13),
        ("RIGHTPADDING", (0, 0), (-1, -1), 13),
        ("TOPPADDING", (0, 0), (-1, -1), 10),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 7),
    ]))
    t.spaceAfter = 6
    return t


# ---------------------------------------------------------------- drawing helpers
def text_block(c, cx, cy, lines):
    """lines: [(text, font, size, colour)], centred on (cx, cy)."""
    gap = 2.6
    total = sum(s for _, _, s, _ in lines) + gap * (len(lines) - 1)
    y = cy + total / 2.0
    for text, font, size, colour in lines:
        y -= size
        c.setFont(font, size)
        c.setFillColor(colour)
        c.drawCentredString(cx, y + size * 0.22, text)
        y -= gap


def box(c, x, y, w, h, lines, fill=white, stroke=GREY, dash=None, lw=0.9):
    c.saveState()
    c.setFillColor(fill)
    c.setStrokeColor(stroke)
    c.setLineWidth(lw)
    if dash:
        c.setDash(*dash)
    c.roundRect(x, y, w, h, 5, stroke=1, fill=1)
    c.restoreState()
    text_block(c, x + w / 2.0, y + h / 2.0, lines)


def arrow(c, x1, y1, x2, y2, colour=GREY, dash=None, lw=1.1, head=5.2, both=False):
    import math
    c.saveState()
    c.setStrokeColor(colour)
    c.setFillColor(colour)
    c.setLineWidth(lw)
    ang = math.atan2(y2 - y1, x2 - x1)
    ux, uy = math.cos(ang), math.sin(ang)
    sx, sy = (x1 + ux * head * 0.8, y1 + uy * head * 0.8) if both else (x1, y1)
    if dash:
        c.setDash(*dash)
    c.line(sx, sy, x2 - ux * head * 0.8, y2 - uy * head * 0.8)
    c.setDash()

    def tip(px, py, a):
        p = c.beginPath()
        p.moveTo(px, py)
        p.lineTo(px - head * math.cos(a - 0.42), py - head * math.sin(a - 0.42))
        p.lineTo(px - head * math.cos(a + 0.42), py - head * math.sin(a + 0.42))
        p.close()
        c.drawPath(p, stroke=0, fill=1)

    tip(x2, y2, ang)
    if both:
        tip(x1, y1, ang + math.pi)
    c.restoreState()


def label(c, x, y, text, colour=MUTED, size=7.1, font="GK", align="c"):
    c.setFont(font, size)
    c.setFillColor(colour)
    if align == "c":
        c.drawCentredString(x, y, text)
    elif align == "l":
        c.drawString(x, y, text)
    else:
        c.drawRightString(x, y, text)


T = lambda s, col=INK, size=8.5: (s, "GK-Semi", size, col)      # box title line
S = lambda s, col=MUTED, size=7.2: (s, "GK", size, col)          # box sub line


class Figure(Flowable):
    fig_h = 100

    def wrap(self, aw, ah):
        self.width = aw
        self.height = self.fig_h
        return aw, self.fig_h


class Roadmap(Figure):
    fig_h = 70

    def draw(self):
        c = self.canv
        stages = [
            ("Drive base", "", "Mostly done", GREEN_TINT, GREEN, HexColor("#1E6B3A")),
            ("Bluetooth", "control", "Next", AMBER_TINT, AMBER, AMBER),
            ("Sensing", "and SLAM", "Later", white, RULE, MUTED),
            ("Lawn mapping", "and coverage", "Later", white, RULE, MUTED),
            ("Cutting", "head", "Later", white, RULE, MUTED),
            ("Two rovers", "together", "Later", white, RULE, MUTED),
        ]
        gap = 13
        w = (self.width - gap * (len(stages) - 1)) / len(stages)
        h = 62
        y = 4
        for i, (a, b, state, fill, stroke, col) in enumerate(stages):
            x = i * (w + gap)
            c.setFillColor(fill)
            c.setStrokeColor(stroke)
            c.setLineWidth(1.0)
            c.roundRect(x, y, w, h, 5, stroke=1, fill=1)
            label(c, x + 8, y + h - 13, f"STAGE {i + 1}", col, 6.6, "GK-Semi", "l")
            label(c, x + 8, y + h - 27, a, INK, 8.6, "GK-Semi", "l")
            if b:
                label(c, x + 8, y + h - 38, b, INK, 8.6, "GK-Semi", "l")
            label(c, x + 8, y + 8, state, col, 7.4, "GK", "l")
            if i < len(stages) - 1:
                arrow(c, x + w + 2.5, y + h / 2, x + w + gap - 2.5, y + h / 2, MUTED, lw=0.9, head=4.2)


class SystemDiagram(Figure):
    fig_h = 186

    def draw(self):
        c = self.canv
        W = self.width
        ok = HexColor("#1E6B3A")
        # panel
        c.setFillColor(PANEL)
        c.roundRect(0, 0, W, self.height, 6, stroke=0, fill=1)
        ox, oy = 12, 12            # inner offset
        inner_w = W - 2 * ox

        # columns
        a_w, b_w, c_w, m_w = 92, 114, 96, 30
        b_x = ox + a_w + 62
        m_x = ox + inner_w - m_w
        c_x = m_x - 14 - c_w
        top_y, top_h = oy + 84, 78
        low_y, low_h = oy + 2, 44

        box(c, ox, top_y + 18, a_w, 44, [T("PC browser"), S("control page")])
        box(c, ox, low_y, a_w, low_h, [T("Phone"), S("Web Bluetooth page")])
        box(c, b_x, top_y, b_w, top_h, [T("NUCLEO-L073RZ", GREEN_DARK, 9), S("our firmware:"), S("commands \u00b7 safety"),
                                        S("motor control")], fill=GREEN_TINT, stroke=GREEN, lw=1.1)
        box(c, b_x, low_y, b_w, low_h, [T("X-NUCLEO-IDB05A2"), S("Bluetooth LE shield")])
        box(c, c_x, top_y, c_w, top_h, [T("X-STM32MP-RBT01"), S("motor board"), S("2 \u00d7 STSPIN948 driver")])
        box(c, c_x, low_y, c_w, low_h, [T("Motor supply"), S("bench supply today")])

        # PC -> Nucleo (working)
        y1 = top_y + 40
        arrow(c, ox + a_w + 3, y1, b_x - 3, y1, ok)
        label(c, (ox + a_w + b_x) / 2, y1 + 5, "USB serial", ok, 7.3, "GK-Semi")
        label(c, (ox + a_w + b_x) / 2, y1 - 11, "working", ok)
        # Phone -> BLE (next)
        y2 = low_y + low_h / 2
        arrow(c, ox + a_w + 3, y2, b_x - 3, y2, AMBER, dash=(3, 2.4))
        label(c, (ox + a_w + b_x) / 2, y2 + 5, "Bluetooth LE", AMBER, 7.3, "GK-Semi")
        label(c, (ox + a_w + b_x) / 2, y2 - 11, "next step", AMBER)
        # BLE -> Nucleo
        bx = b_x + b_w / 2
        arrow(c, bx, low_y + low_h + 3, bx, top_y - 3, GREY, both=True)
        label(c, bx + 7, (low_y + low_h + top_y) / 2 - 2.5, "SPI", MUTED, 7.2, "GK", "l")
        # Nucleo -> RBT01
        arrow(c, b_x + b_w + 3, y1, c_x - 3, y1, GREY)
        mid = (b_x + b_w + c_x) / 2
        label(c, mid, y1 + 14, "per motor:", MUTED, 6.9)
        label(c, mid, y1 + 5, "PWM, direction", MUTED, 6.9)
        label(c, mid, y1 - 11, "enable / fault", MUTED, 6.9)
        # supply -> RBT01
        sx = c_x + c_w / 2
        arrow(c, sx, low_y + low_h + 3, sx, top_y - 3, GREY)
        label(c, sx + 7, (low_y + low_h + top_y) / 2 - 2.5, "motor power", MUTED, 7.2, "GK", "l")
        # motors
        names = ["FL", "FR", "RL", "RR"]
        mh, mg = 15.5, 6
        for i, n in enumerate(names):
            my = top_y + top_h - mh - i * (mh + mg)
            c.setStrokeColor(GREY)
            c.setLineWidth(0.9)
            c.line(c_x + c_w, my + mh / 2, m_x, my + mh / 2)
            box(c, m_x, my, m_w, mh, [T(n, INK, 7.6)])
        label(c, m_x + m_w / 2, top_y - 11, "wheel", MUTED, 6.9)
        label(c, m_x + m_w / 2, top_y - 20, "motors", MUTED, 6.9)


class TargetDiagram(Figure):
    fig_h = 168

    def draw(self):
        c = self.canv
        W = self.width
        c.setFillColor(PANEL)
        c.roundRect(0, 0, W, self.height, 6, stroke=0, fill=1)
        ox, oy = 12, 10
        inner = W - 2 * ox
        col_w = 124
        gap = (inner - 3 * col_w) / 2.0
        x1, x2, x3 = ox, ox + col_w + gap, ox + 2 * (col_w + gap)
        top = self.height - 24
        for x, name in ((x1, "SENSE"), (x2, "DECIDE"), (x3, "ACT")):
            label(c, x, top + 8, name, GREEN, 7.0, "GK-Semi", "l")
        body_h = top - oy
        dash = (3, 2.2)

        # sense: three boxes
        sh = (body_h - 2 * 8) / 3.0
        sense = [("Lidar or rotating ToF", "360\u00b0 range scan"), ("Depth camera", "depth map \u00b7 images"),
                 ("Wheel encoders \u00b7 IMU", "distance and heading")]
        centres = []
        for i, (a, b) in enumerate(sense):
            y = top - sh - i * (sh + 8)
            box(c, x1, y, col_w, sh, [T(a, INK, 8.2), S(b)], stroke=GREY, dash=dash)
            centres.append(y + sh / 2)
        bus = x1 + col_w + gap * 0.42
        c.setStrokeColor(GREY)
        c.setLineWidth(0.9)
        for cy in centres:
            c.line(x1 + col_w, cy, bus, cy)
        c.line(bus, centres[0], bus, centres[-1])

        # decide: Linux computer + radio
        radio_h = 30
        linux_h = body_h - radio_h - 16
        linux_y = top - linux_h
        box(c, x2, linux_y, col_w, linux_h, [T("Linux computer", GREEN_DARK, 9), S("SLAM \u00b7 lawn map"),
                                             S("coverage planning"), S("grass and obstacle recognition")],
            fill=GREEN_TINT, stroke=GREEN, dash=dash, lw=1.1)
        box(c, x2, oy, col_w, radio_h, [T("Radio link", INK, 8.2), S("phone \u00b7 second rover")], stroke=GREY, dash=dash)
        arrow(c, bus, linux_y + linux_h / 2, x2 - 3, linux_y + linux_h / 2, GREY)
        arrow(c, x2 + col_w / 2, oy + radio_h + 2.5, x2 + col_w / 2, linux_y - 2.5, GREY, both=True, head=4.4)

        # act: MCU + wheels + cutter
        mcu_h = 50
        mcu_y = top - mcu_h
        box(c, x3, mcu_y, col_w, mcu_h, [T("Motor controller", INK, 8.6), S("today's Nucleo firmware:"),
                                         S("wheel speed \u00b7 safety stops")], stroke=GREEN, lw=1.1)
        arrow(c, x2 + col_w + 3, mcu_y + mcu_h / 2, x3 - 3, mcu_y + mcu_h / 2, GREY)
        label(c, x2 + col_w + gap / 2, mcu_y + mcu_h / 2 + 5, "drive", MUTED, 6.8)
        label(c, x2 + col_w + gap / 2, mcu_y + mcu_h / 2 - 11, "commands", MUTED, 6.8)
        half = (col_w - 8) / 2.0
        low_h = body_h - mcu_h - 18
        box(c, x3, oy, half, low_h, [T("Wheel", INK, 8.2), T("motors", INK, 8.2), S("on the RBT01")], stroke=GREEN, lw=1.1)
        box(c, x3 + half + 8, oy, half, low_h, [T("Cutter", INK, 8.2), T("motor", INK, 8.2), S("own driver")],
            stroke=GREY, dash=dash)
        arrow(c, x3 + half / 2, mcu_y - 2.5, x3 + half / 2, oy + low_h + 2.5, GREY, head=4.4)
        arrow(c, x3 + half + 8 + half / 2, mcu_y - 2.5, x3 + half + 8 + half / 2, oy + low_h + 2.5, GREY, head=4.4)


# ---------------------------------------------------------------- page furniture
def first_page(c, doc):
    c.saveState()
    c.setFillColor(GREEN_DARK)
    c.rect(0, PAGE_H - BAND_H, PAGE_W, BAND_H, stroke=0, fill=1)
    c.setFillColor(GREEN)
    c.rect(0, PAGE_H - BAND_H - 3, PAGE_W, 3, stroke=0, fill=1)
    c.setFillColor(GREEN_PALE)
    c.setFont("GK-Semi", 8.4)
    c.drawString(MARGIN_X, PAGE_H - 15 * mm, "PROJECT STATUS REPORT")
    c.drawRightString(PAGE_W - MARGIN_X, PAGE_H - 15 * mm, REPORT_DATE.upper())
    c.setFillColor(white)
    c.setFont("GK-Semi", 30)
    c.drawString(MARGIN_X, PAGE_H - 29 * mm, "GrassKit Rover")
    c.setFont("GK-Light", 13.5)
    c.setFillColor(HexColor("#E3F1E6"))
    c.drawString(MARGIN_X, PAGE_H - 37.5 * mm, "Two rovers that mow any lawn on their own: where the build stands and what comes next")
    footer(c, doc)
    c.restoreState()


def later_page(c, doc):
    c.saveState()
    c.setFont("GK-Semi", 8)
    c.setFillColor(GREEN_DARK)
    c.drawString(MARGIN_X, PAGE_H - 12.5 * mm, "GrassKit Rover")
    c.setFont("GK", 8)
    c.setFillColor(MUTED)
    c.drawString(MARGIN_X + stringWidth("GrassKit Rover", "GK-Semi", 8) + 6, PAGE_H - 12.5 * mm, "Project status report")
    c.drawRightString(PAGE_W - MARGIN_X, PAGE_H - 12.5 * mm, REPORT_DATE)
    c.setStrokeColor(RULE)
    c.setLineWidth(0.6)
    c.line(MARGIN_X, PAGE_H - 15 * mm, PAGE_W - MARGIN_X, PAGE_H - 15 * mm)
    footer(c, doc)
    c.restoreState()


def footer(c, doc):
    c.setFont("GK", 8)
    c.setFillColor(MUTED)
    c.drawRightString(PAGE_W - MARGIN_X, 10.5 * mm, f"Page {doc.page}")


class Doc(BaseDocTemplate):
    def afterFlowable(self, flowable):
        if isinstance(flowable, Heading):
            key = f"sec-{self.seq.nextf('outline')}"
            self.canv.bookmarkPage(key)
            self.canv.addOutlineEntry(flowable.outline, key, flowable.level, closed=False)


doc = Doc(OUT, pagesize=A4, leftMargin=MARGIN_X, rightMargin=MARGIN_X,
          title="GrassKit Rover: project status report", author="GrassKit Rover project",
          subject=f"Status on {REPORT_DATE}: what has been built and what comes next")
doc.addPageTemplates([
    PageTemplate(id="first", onPage=first_page,
                 frames=[Frame(MARGIN_X, 17 * mm, FRAME_W, PAGE_H - BAND_H - 17 * mm - 9 * mm,
                               leftPadding=0, rightPadding=0, topPadding=0, bottomPadding=0)]),
    PageTemplate(id="later", onPage=later_page,
                 frames=[Frame(MARGIN_X, 17 * mm, FRAME_W, PAGE_H - 17 * mm - 21 * mm,
                               leftPadding=0, rightPadding=0, topPadding=0, bottomPadding=0)]),
])

W = FRAME_W
story = [NextPageTemplate("later")]

# ================================================================ summary
story.append(callout_box("At a glance", [
    P("<b>Where we are.</b> One rover drives. A four-wheel mecanum base runs our own firmware on an STM32 Nucleo "
      "board and has been driven on the bench from a browser over a USB cable.", callout),
    P("<b>What is not right yet.</b> Several drive commands move the rover the wrong way, and Bluetooth does not "
      "carry commands yet.", callout),
    P("<b>What comes next.</b> Fix the wheel directions, then finish Bluetooth so the rover can be driven from a "
      "phone with no cable. Sensing, SLAM, the cutting head and the second rover all come after that.", callout),
]))

# ================================================================ 1 goal
story.append(H1("The goal"))
story.append(P(
    "Two small rovers that can be put down on any lawn and cut it on their own: no boundary wire, no map prepared "
    "in advance and nobody steering. The rovers have to find the shape of the lawn, agree how to share it, cover "
    "every part of it, keep clear of whatever is in the way, and cut as they go.", lead))
story.append(P(
    "The work is staged so that each stage ends with something that runs. Stage 1 is nearly complete; stage 2 is "
    "the immediate next step."))
story.append(Roadmap())
story.append(P("Figure 1. The six stages. Stage 1 still has open issues, listed in section 4.", caption))

# ================================================================ 2 status
story.append(H1("Where the project stands"))
story.append(P(
    "One rover exists and drives under cable control. Figure 2 shows it as built; the table that follows lists "
    "every area of the project and how far it has got."))
story.append(SystemDiagram())
story.append(P("Figure 2. The rover today. The USB path is what all testing has used; the Bluetooth path is the "
               "next step.", caption))
status_rows = [
    ["Area", "Status", "Notes"],
    ["Hardware chosen and wired", Chip("Done"),
     "Nucleo-L073RZ controller, X-STM32MP-RBT01 motor board, Bluetooth shield, four-motor mecanum chassis"],
    ["Pin map checked against ST manuals", Chip("Done"),
     "Rebuilt from ST's board manual and driver datasheet after the first table proved wrong"],
    ["Motor firmware", Chip("Done"), "PWM, direction, enable, ramping and mecanum mixing; run on the bench"],
    ["Safety layer", Chip("Done"),
     "Safe start-up, command timeout, emergency stop, driver-fault detection. No hardware watchdog yet"],
    ["Command protocol and USB control page", Chip("Done"), "Used for all bench testing so far"],
    ["Vendor documents and repository", Chip("Done"), "43 PDFs indexed in the documentation folder; code on GitHub"],
    ["Wheel direction mapping", Chip("Open issue"),
     "Rear-left wheel runs backwards; sideways and turn commands move the wrong way"],
    ["Bluetooth control", Chip("In progress"),
     "The shield answers over SPI. It is not confirmed advertising and has no data service"],
    ["Battery power", Chip("Not started"), "Bench supply so far; batteries still to be chosen"],
    ["Odometry and sensing", Chip("Not started"), "No wheel encoders; the motor board's sensors are not read yet"],
    ["SLAM, depth maps, lawn mapping", Chip("Not started"), "Needs a larger computer than the Nucleo"],
    ["Cutting head", Chip("Not started"), "Blade or trimmer line still to be decided"],
    ["Machine learning", Chip("Not started"), "Grass, lawn-edge and obstacle recognition"],
    ["Second rover", Chip("Not started"), "A copy of the first, plus coordination between the two"],
]
story.append(table(status_rows, [W * 0.335, W * 0.155, W * 0.51]))

# ================================================================ 3 done
story.append(H1("What has been done"))

story.append(H2("The rover as built"))
story.append(P(
    "The rover is assembled from ST evaluation boards on a four-wheel chassis, connected as in Figure 2. Nothing "
    "is custom-made yet, which keeps every part documented and replaceable."))
story.append(table([
    ["Part", "What it is", "What it does here"],
    ["NUCLEO-L073RZ", "STM32L073RZ microcontroller board: Cortex-M0+ at 32 MHz, 192 KB flash, 20 KB RAM",
     "Runs all of the firmware"],
    ["X-STM32MP-RBT01", "ST robotics board with two STSPIN948 dual full-bridge motor drivers. It also carries an "
     "inertial sensor, a magnetometer, a pressure sensor and a time-of-flight ranging sensor",
     "Drives the four wheel motors. Its sensors are not used yet"],
    ["X-NUCLEO-IDB05A2", "Bluetooth Low Energy 4.2 shield built on the BlueNRG-M0 module, connected over SPI",
     "Wireless link to a phone. Not working yet"],
    ["Chassis", "Four DC gear motors with mecanum wheels", "Lets the rover move forwards, sideways and rotate"],
    ["Power", "USB for the Nucleo, a bench supply for the motors", "Batteries are still to be chosen"],
], [W * 0.2, W * 0.47, W * 0.33]))
story.append(H2("Getting the wiring right"))
story.append(P(
    "The RBT01 was designed to plug onto an STM32MP discovery kit, not a Nucleo, so every signal is jumper-wired "
    "from the Nucleo to the board's 40-pin header. The project's first pin table had been written without ST's "
    "manual and did not match the board. It was rebuilt from the board's user manual (UM3482, Table 11 and the "
    "schematic sheets) and the STSPIN948 datasheet, and the firmware, the CubeMX configuration and every wiring "
    "document were changed to match."))
story.append(P("Three findings from that work shape the firmware:"))
story += bullets([
    "<b>Each motor needs three lines:</b> a PWM line for speed, a phase line for direction, and a combined "
    "enable and fault line.",
    "<b>The drivers are on by default.</b> The board pulls every enable line high, so a motor can be driven as "
    "soon as its supply is present. The firmware therefore holds all four lines low at power-up and whenever the "
    "rover is idle.",
    "<b>The enable line is also the fault output.</b> The driver pulls it low on under-voltage, over-current or "
    "over-temperature, so the firmware drives it open-drain and reads it back.",
])
story.append(Spacer(1, 3))
story.append(table([
    ["Motor", "Speed (PWM)", "Direction", "Enable / fault", "Motor terminals"],
    ["1  Front-left", "PB4 \u2192 pin 32", "PC0 \u2192 pin 23", "PC4 \u2192 pin 13", "CN4: 2A + 1A"],
    ["2  Front-right", "PB5 \u2192 pin 33", "PC1 \u2192 pin 24", "PC5 \u2192 pin 15", "CN4: 1B + 2B"],
    ["3  Rear-left", "PB0 \u2192 pin 31", "PC2 \u2192 pin 19", "PC6 \u2192 pin 16", "CN5: 2A + 1A"],
    ["4  Rear-right", "PB1 \u2192 pin 36", "PC3 \u2192 pin 21", "PC8 \u2192 pin 18", "CN5: 1B + 2B"],
], [W * 0.2, W * 0.2, W * 0.2, W * 0.2, W * 0.2], mono_cols=(1, 2, 3, 4)))
story.append(P("Table: Nucleo pin to RBT01 header pin for each motor. The board's mode jumpers stay at their "
               "factory positions.", caption))

story.append(H2("Firmware"))
story.append(P(
    "The firmware is about 2,400 lines of application C on ST's HAL library, built with the GCC toolchain that "
    "ships with STM32CubeIDE and flashed through the Nucleo's on-board ST-LINK. It occupies about 79 KB of the "
    "192 KB of flash and under 2 KB of the 20 KB of RAM for static data. Everything runs from one loop that "
    "executes once per millisecond."))
story.append(table([
    ["Module", "Responsibility"],
    ["app.c", "The one-millisecond loop: read commands, run the safety checks, update the motor ramps, send "
     "telemetry, and blink the status LED at a rate that shows the state"],
    ["motor_control.c", "PWM, direction and enable for four motors; mecanum mixing; ramping; driver-fault detection"],
    ["safety.c", "Command timeout, emergency-stop button, latched emergency stop"],
    ["command_parser.c", "The text command protocol"],
    ["bluetooth.c", "Serial receive from USB and a spare UART into one buffer; bring-up of the Bluetooth module"],
    ["engine_model.c", "A virtual engine (RPM, gears, turbo) left from the project's start as a race-car demo. "
     "It feeds telemetry only"],
    ["debug.c", "Boot banner and the periodic telemetry line"],
], [W * 0.24, W * 0.76], mono_cols=(0,)))
story.append(P(
    "The project began as a race-car demonstrator with synthesised engine sound. The audio engine has been removed "
    "from the firmware and documents. The project folder name, the virtual engine model and three commands "
    "(GEAR, TURBO, REV) are what remains of that."))

story.append(H2("Motor control"))
story += bullets([
    "<b>PWM.</b> One timer produces four 20 kHz PWM signals, above the audible range.",
    "<b>Speed and direction.</b> Each wheel takes a signed speed from \u2212100 % to +100 %. Below 4 % the output "
    "is held at zero.",
    "<b>Ramping.</b> A wheel's speed moves towards its target by at most 2 % per millisecond, so a standing start "
    "to full speed takes 50 ms. This avoids current spikes.",
    "<b>Mixing.</b> Forward, sideways and rotation demands are combined into four wheel speeds with the standard "
    "mecanum equations, and all four are scaled down together if any would exceed 100 %. A second mode drives the "
    "left and right sides at different speeds, without sideways motion.",
    "<b>Stopping.</b> STOP ramps down and then switches the drivers off. BRAKE zeroes the PWM with the drivers on, "
    "which brakes the motors. Emergency stop switches the drivers off at once.",
    "<b>Single-wheel test.</b> One command drives one wheel alone, which is how wiring and direction are checked.",
])

story.append(H2("Safety"))
story += bullets([
    "<b>Safe start-up.</b> The motor outputs are set up before anything else at boot, and the drivers stay off "
    "until a drive command arrives.",
    "<b>Off when idle.</b> Once the wheels have ramped to zero the drivers are switched off again.",
    "<b>Command timeout.</b> If the rover is moving and no valid command arrives for 1.5 seconds, it stops. "
    "Unrecognised commands do not count.",
    "<b>Emergency stop.</b> The blue button on the Nucleo or the ESTOP command switches the drivers off and zeroes "
    "the PWM. The stop is latched until RESET is sent.",
    "<b>Driver faults.</b> After a 10 ms settling time, three low readings in a row on a motor's fault line switch "
    "all drivers off and report which motor tripped.",
])
story.append(P(
    "One gap remains: there is no independent hardware watchdog, so a firmware hang would leave the outputs in "
    "their last state."))

story.append(H2("Commands and telemetry"))
story.append(P(
    "Commands are plain text, one per line, at 115200 baud, and are not case-sensitive. Each gets a reply that "
    "starts with OK, ERR, WARN or ALARM. The same parser serves the USB serial port and the spare UART, and is "
    "where Bluetooth will deliver its data."))
story.append(table([
    ["Command", "Effect"],
    ["START, STOP, BRAKE", "Enable the drivers; ramp to a stop; brake"],
    ["ESTOP, RESET", "Latched emergency stop; clear it (this also clears a driver fault)"],
    ["DRIVE:f,r,s", "Forward, rotation and sideways demand in one line, each from \u2212100 to 100"],
    ["THROTTLE:n, STEER:n, STRAFE:n", "Set one of the three demands at a time"],
    ["MODE:MECANUM, MODE:RACECAR", "Choose the mixing"],
    ["MOTOR:n,speed", "Drive one wheel alone (n is 1 to 4)"],
    ["STATUS", "One-line report: state and the four wheel speeds"],
    ["BLE_TEST, BLE_ADV", "Bluetooth diagnostics: run the bring-up sequence and report each step; restart advertising"],
    ["GEAR, TURBO, REV", "Race-car leftovers. They change the virtual engine only, not the wheels"],
], [W * 0.36, W * 0.64], mono_cols=(0,)))
story.append(P("While the rover is running or a controller is connected, a telemetry line with the four wheel "
               "speeds is sent every 200 ms."))

story.append(H2("Control tools"))
story.append(table([
    ["Tool", "What it is"],
    ["web_serial_server.py", "A small Python server that opens the Nucleo's USB serial port and serves a control "
     "page in the browser: a drive pad, a test button for each wheel and live telemetry. It keeps sending commands "
     "so the command timeout does not trip. All bench testing so far used this page"],
    ["racecar_controller.py", "A desktop Python window with sliders, a sideways pad and a log"],
    ["web_bluetooth_controller.html", "A phone page that uses Web Bluetooth and expects a Nordic UART Service on "
     "the rover. It is waiting for the firmware side"],
    ["scan_ble.py", "Scans for the rover's Bluetooth advertisement"],
], [W * 0.33, W * 0.67], mono_cols=(0,)))

story.append(H2("What has run on the bench"))
story += bullets([
    "The firmware builds, flashes and boots, and reports its state over USB.",
    "All four wheels have been driven from the browser control page.",
    "<b>A dead front-right wheel was traced to the firmware, not the wiring.</b> On this microcontroller the pin "
    "used for motor 2's PWM is connected to a different timer unless a remap setting is made. The firmware never "
    "made it, so motor 2 received no PWM. With motor 2 commanded to 50 %, the pin read low in 12 of 12 samples "
    "before the fix and high in 6 of 12 after it.",
    "The highest voltage and current recorded at the wheels were 6.5 V and 0.45 A.",
    "Not every command moves the rover the right way yet. That is the first item in section 4.",
])

story.append(H2("Documents and repository"))
story.append(P(
    "Forty-three vendor PDFs are stored in the documentation folder in nine numbered sub-folders: board manuals, "
    "schematics, datasheets, tool guides, ST's material on using the RBT01 from Linux, and one SLAM reference. An "
    "index gives each document's number and revision and says what to use it for. The code and documents are in a "
    "Git repository on GitHub (nottysukku/GrassKit_Rover)."))

# ================================================================ 4 open issues
story.append(CondPageBreak(70 * mm))
story.append(H1("Open issues in what exists"))
story.append(P("These need closing before the next stage builds on top of them."))

story.append(H2("Wheel directions"))
story.append(P("Driving from the control page gave these results:"))
story.append(table([
    ["Command", "What the rover did"],
    ["Forward", "Rear-left wheel runs backwards"],
    ["Sideways left", "Moves right"],
    ["Sideways right", "Moves forwards"],
    ["Turn left", "Moves forwards"],
    ["Turn right", "Moves sideways to the right"],
], [W * 0.3, W * 0.7]))
story.append(P(
    "Working the mixing equations through with the rear-left motor reversed gives motion consistent with four of "
    "the five results (forward, sideways-right, turn-left and turn-right), so that one motor is the likely cause "
    "of most of this. It can be corrected "
    "by swapping its two leads or by inverting it in firmware. Sideways-left does not fit the same explanation and "
    "should be tested again after the correction; if it is still wrong, the roller pattern of the wheels is the "
    "next thing to check."))

story.append(H2("Bluetooth does not carry commands"))
story.append(P(
    "The shield is fitted and its SPI link responds: reset works, the module raises its interrupt line when it "
    "boots, and it accepts command headers. The firmware resets the module, initialises it and asks it to "
    "advertise under the name RACECAR-L073. Two things are missing. Advertising has not been confirmed from a "
    "phone or a scanner, and the firmware defines no data service, so a phone that did connect would have nowhere "
    "to send commands."))

story.append(H2("Power"))
story.append(P(
    "The motors have only run from a bench supply. The plan is a 9 V battery for the Nucleo and a separate battery "
    "pack for the motor board, sized from the 6.5 V and 0.45 A measurement with margin. The motor pack has not "
    "been chosen."))

story.append(H2("Smaller items"))
story += bullets([
    "The four current-limit trimmers on the motor board should be set to suit the motors and the setting recorded.",
    "Add a hardware watchdog so that a firmware hang switches the drivers off.",
    "Remove the race-car leftovers: the virtual engine model, the GEAR, TURBO and REV commands, and the project "
    "folder name.",
])

# ================================================================ 5 next
story.append(CondPageBreak(80 * mm))
story.append(H1("What comes next"))
story.append(P(
    "These are proposals, in the order they are likely to be needed. Each is kept short on purpose; none has been "
    "started."))

story.append(H2("Bluetooth control (the next step)"))
story += bullets([
    "Get the module advertising, and see it in the scanner script and on a phone.",
    "Add a UART-style data service. The phone page already expects the Nordic UART Service: one characteristic "
    "for commands in, one for telemetry out.",
    "Handle the module's events properly (connected, disconnected, data written) and pass written data to the "
    "existing command parser. Stop the rover when the phone disconnects.",
    "Result: the rover is driven from a phone with no USB cable.",
])

story.append(H2("Battery power and a first drive on grass"))
story += bullets([
    "Choose the motor battery, set the current limits and run the rover untethered.",
    "Try real grass early. Mecanum wheels are made for hard, flat floors, so grip and sideways motion on a lawn "
    "need to be measured. If they are poor, treaded wheels with left-right steering are the fallback, and the "
    "firmware already has that mode.",
])

story.append(H2("Odometry: knowing how far the rover has moved"))
story += bullets([
    "The motors run open-loop today: the firmware sets a duty cycle and never learns how fast a wheel turned. Add "
    "wheel encoders, or motors with encoders built in, and closed-loop speed control.",
    "Read the inertial sensor and magnetometer already on the RBT01 for heading.",
    "Wheel distance plus heading is an input that every SLAM method needs.",
])

story.append(H2("A larger computer on the rover"))
story += bullets([
    "The Nucleo's microcontroller has 20 KB of RAM. It cannot run SLAM, depth processing or neural networks.",
    "The RBT01 was designed to sit on an STM32MP157F-DK2 or STM32MP257F-DK running Linux, and ST's X-LINUX-RBT1 "
    "package drives its motors and sensors from there. Its header also matches the Raspberry Pi layout.",
    "Decision to make: move motor control onto the Linux board, or keep the Nucleo as the real-time motor "
    "controller and have the Linux board send it the same text commands. Figure 3 shows the second option.",
])
story.append(Spacer(1, 4))
story.append(KeepTogether([
    TargetDiagram(),
    P("Figure 3. A proposed layout for each rover. Solid outlines exist today; dashed outlines are still to be "
      "added.", caption),
]))

story.append(H2("SLAM and depth maps"))
story += bullets([
    "<b>Range sensor.</b> SLAM needs a scan of distances all around the rover. A ready-made spinning 2D lidar "
    "gives that directly. The alternative is to build the rotator: a small motor turning a time-of-flight sensor, "
    "with an angle sensor and a slip ring for the wiring. Buying is simpler and more accurate; building is cheaper "
    "and reuses ST parts.",
    "<b>Depth.</b> A stereo or time-of-flight depth camera gives a depth map of what is ahead. The 8 \u00d7 8 zone "
    "time-of-flight sensor on the RBT01 can cover the area just in front of the rover.",
    "<b>Software.</b> ROS 2 with slam_toolbox for mapping and Nav2 for navigation is the usual starting point.",
    "<b>Lawn caveat.</b> Open grass gives a lidar little to lock onto, and sunlight shortens time-of-flight range. "
    "Expect to combine the scan with odometry and the inertial sensor, and possibly to add satellite positioning "
    "(RTK) or fixed markers.",
])

story.append(H2("Mapping an unknown lawn and covering it"))
story += bullets([
    "<b>Find the boundary.</b> Either drive the edge once by remote control and record it, or detect the edge of "
    "the grass with the camera.",
    "<b>Store the map</b> as a grid of cells marked lawn, obstacle or keep-out (flower beds, ponds).",
    "<b>Plan coverage.</b> Split the lawn into simple regions and sweep each in back-and-forth stripes with a small "
    "overlap, recording what has been cut.",
    "<b>Recover.</b> Find the rover's position again after a restart, and cope with slopes and wheel slip.",
])

story.append(H2("Cutting head"))
story += bullets([
    "<b>Blade or line.</b> Small pivoting blades on a spinning disc cut cleanly and need little power, but must "
    "be guarded. Trimmer line tolerates stones and edges and is gentler on contact, but uses more power, wears "
    "and is louder.",
    "<b>Its own motor and driver.</b> All four outputs of the RBT01 are taken by the wheels, so the cutter needs a "
    "separate driver. Motor current can be used to detect a stalled or blocked cutter.",
    "<b>Placement.</b> Under the chassis on the centre line, between the axles, with adjustable height. The "
    "cutting width sets the stripe spacing for the coverage planner.",
    "<b>Safety.</b> A guard, a lift and tilt sensor that stops the cutter, and an emergency stop that removes "
    "cutter power as well as wheel power.",
])

story.append(H2("Machine learning"))
story += bullets([
    "<b>Grass or not grass</b> in the camera image, to follow the lawn edge and stay on the lawn.",
    "<b>Cut or uncut grass,</b> to check coverage.",
    "<b>Obstacles:</b> people, pets, toys, hoses. Stop for some, steer round others.",
    "<b>What it needs:</b> labelled images collected by the rover's own camera on several lawns, and models small "
    "enough to run on the on-board computer.",
])

story.append(H2("Two rovers"))
story += bullets([
    "A shared map in one coordinate frame, a way to divide the lawn between the rovers, and a rule for keeping "
    "clear of each other.",
    "A radio link between the rovers. Bluetooth stays as the link to the phone.",
    "Build one complete rover first. The second is a copy plus the coordination layer.",
])

story.append(H2("Ready for the field"))
story += bullets([
    "Weather sealing, battery life, and a charging dock the rover can return to.",
    "The safety standard for robotic lawnmowers, IEC 60335-2-107, as the reference for guards, sensors and "
    "stopping behaviour.",
])

# ================================================================ 6 order
story.append(CondPageBreak(75 * mm))
story.append(H1("Suggested order of work"))
story.append(table([
    ["Step", "Work", "Done when"],
    ["1", "Fix wheel directions", "Every move on the drive pad goes the commanded way"],
    ["2", "Bluetooth control", "The rover is driven from a phone with no cable"],
    ["3", "Battery power", "The rover runs untethered, and has been tried on grass"],
    ["4", "Encoders and heading", "The rover reports distance and heading and can drive a measured square"],
    ["5", "Linux computer on board", "ROS 2 runs on the rover and its drive commands reach the wheels"],
    ["6", "Range sensor and SLAM", "The rover maps a room, then a lawn"],
    ["7", "Boundary and coverage planning", "The rover covers a whole lawn with the cutter off"],
    ["8", "Cutting head", "The rover cuts while it covers"],
    ["9", "Recognition models", "The rover stops for obstacles and follows the grass edge"],
    ["10", "Second rover", "Two rovers share one lawn"],
], [W * 0.09, W * 0.33, W * 0.58]))

doc.build(story)
print("wrote", OUT)
