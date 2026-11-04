"""Preview the rover control page on a PC, without the rover.

python web/mock_server.py [port]   then open http://localhost:8765/ (or your port)
Serves web/index.html gzipped (as the rover does) and answers /c?x=<command>
with the same "OK:... W:..." replies, using the firmware's wheel mixing.
"""
import gzip
import http.server
import pathlib
import sys
import urllib.parse

HERE = pathlib.Path(__file__).resolve().parent
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8765

state = {"st": "IDLE", "md": "M", "sf": 0, "lk": 1, "fl": [1, 1, 1, 1], "tr": [100, 100, 100, 100], "w": [0, 0, 0, 0]}


def clamp(v, lo, hi):
    return max(lo, min(hi, v))


def drive(th, tn, sf):
    if state["md"] == "T":
        sf = 0
    elif state["sf"]:
        sf = -sf
    m = [th + tn + sf, th - tn - sf, th + tn - sf, th - tn + sf]
    top = max(abs(x) for x in m)
    if top > 100:
        m = [x * 100 / top for x in m]
    set_wheels([clamp(x * t / 100, -100, 100) for x, t in zip(m, state["tr"])])


def set_wheels(m):
    state["w"] = [round(x) for x in m]
    state["st"] = "RUNNING" if any(abs(x) > 1 for x in m) else state["st"]


def run(cmd):
    c = cmd.upper()
    nums = lambda s: [float(x) for x in s.split(",") if x.strip()]
    if c.startswith("DRIVE:"):
        v = nums(c[6:]) + [0, 0, 0]
        drive(*v[:3])
        return "OK:DRIVE"
    if c.startswith("WHEELS:"):
        set_wheels((nums(c[7:]) + [0, 0, 0, 0])[:4])
        return "OK:WHEELS"
    if c == "STOP":
        state["w"] = [0, 0, 0, 0]
        state["st"] = "IDLE"
        return "OK:STOP"
    if c == "ESTOP":
        state["w"] = [0, 0, 0, 0]
        state["st"] = "ESTOP"
        return "ALARM:EMERGENCY_STOP_TRIGGERED"
    if c == "RESET":
        state["st"] = "IDLE"
        return "OK:RESET"
    if c.startswith("MODE:"):
        state["md"] = "M" if ("MEC" in c or "OMNI" in c) else "T"
        return "OK:MODE:" + ("MECANUM" if state["md"] == "M" else "TANK")
    if c.startswith("FLIP:"):
        i, v = nums(c[5:])[:2]
        idx = int(i) - 1
        state["fl"][idx] = int(v != 0)
        if state["lk"]:
            state["fl"][idx ^ 2] = int(v != 0)
            return "OK:FLIP " + ("Front left + Rear left" if idx in (0, 2) else "Front right + Rear right")
        return "OK:FLIP"
    if c.startswith("LINK:"):
        state["lk"] = int(c[5:] != "0")
        if state["lk"]:
            state["fl"][0], state["fl"][1] = state["fl"][2], state["fl"][3]
            return "OK:LINK:ON (fronts follow rears)"
        return "OK:LINK:OFF"
    if c.startswith("TRIM:"):
        i, v = nums(c[5:])[:2]
        state["tr"][int(i) - 1] = int(clamp(v, 50, 150))
        return "OK:TRIM"
    if c.startswith("SFLIP:"):
        state["sf"] = int(c[6:] != "0")
        return "OK:SFLIP"
    if c == "SAVE":
        return "OK:SAVED"
    if c == "DEFAULTS":
        state.update(md="M", sf=0, lk=1, fl=[1, 1, 1, 1], tr=[100, 100, 100, 100])
        return "OK:DEFAULTS (not saved yet)"
    if c == "STATUS":
        return ""
    return "ERR:UNKNOWN_COMMAND"


def web_status():
    d = [abs(x) for x in state["w"]]
    ph = [int((x >= 0) != bool(f)) for x, f in zip(state["w"], state["fl"])]
    j = lambda a: ",".join(str(x) for x in a)
    return (f"W:st={state['st']};f=0;md={state['md']};sf={state['sf']};lk={state['lk']};w={j(state['w'])};d={j(d)};"
            f"ph={j(ph)};fl={j(state['fl'])};tr={j(state['tr'])}")


class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        url = urllib.parse.urlparse(self.path)
        if url.path in ("/", "/index.html"):
            body = gzip.compress((HERE / "index.html").read_bytes())
            self.reply(body, "text/html; charset=utf-8", gz=True)
        elif url.path == "/c":
            cmd = urllib.parse.parse_qs(url.query).get("x", [""])[0]
            print("cmd:", cmd, flush=True)
            self.reply((run(cmd) + " " + web_status()).strip().encode(), "text/plain")
        else:
            self.send_error(404)

    def reply(self, body, ctype, gz=False):
        self.send_response(200)
        self.send_header("Content-Type", ctype)
        if gz:
            self.send_header("Content-Encoding", "gzip")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, *args):
        pass


if __name__ == "__main__":
    print(f"mock rover on http://localhost:{PORT}/", flush=True)
    http.server.ThreadingHTTPServer(("127.0.0.1", PORT), Handler).serve_forever()
