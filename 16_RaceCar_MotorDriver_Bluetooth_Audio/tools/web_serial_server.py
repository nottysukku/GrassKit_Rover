#!/usr/bin/env python3
"""
================================================================================
STM32 RaceCar Web Serial Controller Server
================================================================================
Bridges browser UI (mobile phone & PC) to STM32 NUCLEO-L073RZ over USB Serial (COM7).
Features:
  - Responsive Mobile & Desktop Web Dashboard on http://localhost:8080
  - Continuous keep-alive heartbeat loop for smooth, uninterrupted driving
  - Mecanum 8-directional omni-strafe + differential turn D-pad
  - Keyboard controls (WASD + Arrows + Space)
  - Real-time telemetry: Virtual RPM, Throttle, Gear, Boost, M1-M4 Motor Speeds
"""

import sys
import time
import json
import threading
import socket
from http.server import HTTPServer, BaseHTTPRequestHandler
import serial
import serial.tools.list_ports

PORT = 8080
DEFAULT_SERIAL = 'COM7'
BAUD = 115200

state_lock = threading.Lock()
telemetry_data = {
    "state": "DISCONNECTED",
    "rpm": 900,
    "throttle": 0,
    "gear": 1,
    "boost": 0.0,
    "m1": 0,
    "m2": 0,
    "m3": 0,
    "m4": 0,
    "last_seen": 0,
    "connected": False
}
recent_logs = []
ser_device = None

def log_msg(msg):
    global recent_logs
    ts = time.strftime("%H:%M:%S")
    entry = f"[{ts}] {msg}"
    print(entry)
    with state_lock:
        recent_logs.append(entry)
        if len(recent_logs) > 40:
            recent_logs.pop(0)

def send_serial(cmd):
    global ser_device
    if not cmd.endswith('\n'):
        cmd += '\n'
    with state_lock:
        if ser_device and ser_device.is_open:
            try:
                ser_device.write(cmd.encode('utf-8'))
                return True
            except Exception as e:
                log_msg(f"Write error: {e}")
    return False

def serial_reader_thread():
    global ser_device, telemetry_data
    while True:
        try:
            if ser_device is None or not ser_device.is_open:
                # Try opening serial port
                try:
                    ser_device = serial.Serial(DEFAULT_SERIAL, BAUD, timeout=1)
                    log_msg(f"Connected to {DEFAULT_SERIAL} @ {BAUD} baud")
                    with state_lock:
                        telemetry_data["connected"] = True
                    # Query initial status
                    send_serial("STATUS\n")
                except Exception as e:
                    with state_lock:
                        telemetry_data["connected"] = False
                    time.sleep(2)
                    continue

            line = ser_device.readline()
            if not line:
                continue
            txt = line.decode('utf-8', errors='replace').strip()
            if not txt:
                continue

            with state_lock:
                telemetry_data["last_seen"] = time.time()
                telemetry_data["connected"] = True

                # Parse TELE: or STATUS:
                if "RPM=" in txt:
                    for part in txt.replace("STATUS:", "").replace("TELE:", "").split(","):
                        if "=" in part:
                            k, v = part.split("=", 1)
                            k = k.strip().upper()
                            v = v.replace("%", "").strip()
                            try:
                                if k == "STATE": telemetry_data["state"] = v
                                elif k == "RPM": telemetry_data["rpm"] = int(float(v))
                                elif k in ("THROTTLE", "THROT"): telemetry_data["throttle"] = int(float(v))
                                elif k == "GEAR": telemetry_data["gear"] = int(v) if v.isdigit() else v
                                elif k == "BOOST": telemetry_data["boost"] = float(v)
                                elif k == "M1": telemetry_data["m1"] = int(float(v))
                                elif k == "M2": telemetry_data["m2"] = int(float(v))
                                elif k == "M3": telemetry_data["m3"] = int(float(v))
                                elif k == "M4": telemetry_data["m4"] = int(float(v))
                            except Exception:
                                pass

                # Keep log of interesting responses
                if any(txt.startswith(p) for p in ("OK:", "ERR:", "ALARM:", "WARN:", "STATUS:")):
                    recent_logs.append(f"[{time.strftime('%H:%M:%S')}] << {txt}")
                    if len(recent_logs) > 40: recent_logs.pop(0)

        except Exception as e:
            log_msg(f"Serial exception: {e}")
            with state_lock:
                telemetry_data["connected"] = False
            time.sleep(2)

HTML_PAGE = """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>🏎️ STM32 RaceCar Web Console</title>
  <style>
    :root {
      --bg: #0b0e14;
      --card: #141923;
      --border: #232a3b;
      --accent: #ff4757;
      --green: #2ed573;
      --blue: #1e90ff;
      --gold: #ffa502;
      --purple: #9b59b6;
      --text: #f1f2f6;
      --dim: #747d8c;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; user-select: none; -webkit-user-select: none; }
    body {
      background: var(--bg);
      color: var(--text);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 10px;
      touch-action: manipulation;
    }
    .container {
      width: 100%;
      max-width: 520px;
      display: flex;
      flex-direction: column;
      gap: 10px;
    }
    .header {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 12px;
      padding: 10px 14px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .header h1 { font-size: 16px; font-weight: 800; }
    .badge {
      display: flex;
      align-items: center;
      gap: 6px;
      font-size: 12px;
      font-weight: 700;
      color: var(--accent);
    }
    .badge.connected { color: var(--green); }
    .dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: currentColor;
    }
    .gauge-card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 14px;
      padding: 14px;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 8px;
    }
    .rpm-display {
      font-family: monospace;
      font-size: 42px;
      font-weight: 900;
      color: var(--green);
      letter-spacing: -1px;
    }
    .rpm-label { font-size: 11px; color: var(--dim); font-weight: 600; text-transform: uppercase; }
    .cluster-grid {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      width: 100%;
      gap: 6px;
      background: #0d1118;
      padding: 8px;
      border-radius: 10px;
    }
    .stat-box { text-align: center; }
    .stat-val { font-size: 18px; font-weight: 800; font-family: monospace; color: var(--blue); }
    .stat-lbl { font-size: 10px; color: var(--dim); }
    
    .motors-row {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      width: 100%;
      gap: 6px;
      margin-top: 4px;
    }
    .m-bar-box {
      background: #0d1118;
      border-radius: 8px;
      padding: 6px;
      text-align: center;
    }
    .m-title { font-size: 10px; color: var(--dim); margin-bottom: 2px; }
    .m-val { font-size: 14px; font-weight: 800; font-family: monospace; color: var(--gold); }
    .m-progress {
      width: 100%;
      height: 4px;
      background: #202738;
      border-radius: 2px;
      margin-top: 4px;
      overflow: hidden;
    }
    .m-fill { height: 100%; width: 0%; background: var(--green); transition: width 0.1s; }

    .slider-card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 12px;
      padding: 10px 14px;
      display: flex;
      flex-direction: column;
      gap: 6px;
    }
    .slider-header { display: flex; justify-content: space-between; font-size: 12px; font-weight: 700; }
    input[type=range] {
      width: 100%;
      height: 8px;
      border-radius: 4px;
      accent-color: var(--blue);
      cursor: pointer;
    }

    .action-row {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 8px;
    }
    .btn-act {
      border: none;
      border-radius: 10px;
      padding: 12px 6px;
      font-weight: 800;
      font-size: 13px;
      color: #0b0e14;
      cursor: pointer;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 2px;
      transition: transform 0.08s, filter 0.08s;
    }
    .btn-act:active { transform: scale(0.96); filter: brightness(1.2); }
    .btn-start { background: var(--green); }
    .btn-stop { background: var(--dim); color: white; }
    .btn-brake { background: var(--gold); }
    .btn-rev { background: #ff9f43; }
    .btn-turbo { background: var(--purple); color: white; }
    .btn-estop {
      background: var(--accent);
      color: white;
      grid-column: span 3;
      padding: 14px;
      font-size: 15px;
      font-weight: 900;
    }

    /* Mecanum D-Pad */
    .dpad-card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 14px;
      padding: 12px;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 8px;
    }
    .dpad-grid {
      display: grid;
      grid-template-columns: repeat(3, 76px);
      grid-template-rows: repeat(3, 76px);
      gap: 8px;
    }
    .btn-pad {
      background: #1c2331;
      border: 1px solid #2a3449;
      color: white;
      font-weight: 800;
      font-size: 15px;
      border-radius: 12px;
      cursor: pointer;
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      touch-action: none;
      transition: background 0.1s, transform 0.08s;
    }
    .btn-pad:active, .btn-pad.active {
      background: var(--blue);
      transform: scale(0.94);
    }
    .btn-pad small { font-size: 9px; color: #a4b0be; margin-top: 2px; }
    .btn-center-stop {
      background: #3a151b;
      border-color: #5c1d27;
      color: var(--accent);
    }
    .btn-center-stop:active { background: var(--accent); color: white; }

    .log-box {
      background: #080a0f;
      border: 1px solid var(--border);
      border-radius: 10px;
      padding: 8px 10px;
      font-family: monospace;
      font-size: 11px;
      height: 90px;
      overflow-y: auto;
      color: #a4b0be;
    }
  </style>
</head>
<body>

<div class="container">
  <div class="header">
    <div>
      <h1>🏎️ RaceCar Web Controller</h1>
      <small style="color:var(--dim); font-size:11px;">STM32 NUCLEO-L073RZ & STSPIN948</small>
    </div>
    <div class="badge" id="badge"><div class="dot"></div><span id="conn-lbl">DISCONNECTED</span></div>
  </div>

  <div class="gauge-card">
    <div class="rpm-display" id="rpm">900</div>
    <div class="rpm-label">ENGINE VIRTUAL RPM</div>
    <div class="cluster-grid">
      <div class="stat-box"><div class="stat-val" id="disp-throt">0%</div><div class="stat-lbl">THROTTLE</div></div>
      <div class="stat-box"><div class="stat-val" id="disp-gear">1</div><div class="stat-lbl">GEAR</div></div>
      <div class="stat-box"><div class="stat-val" id="disp-boost">0.0</div><div class="stat-lbl">BOOST</div></div>
      <div class="stat-box"><div class="stat-val" id="disp-mode">OMNI</div><div class="stat-lbl">KINEMATICS</div></div>
    </div>
    <div class="motors-row">
      <div class="m-bar-box">
        <div class="m-title">M1 FL</div>
        <div class="m-val" id="m1-val">0</div>
        <div class="m-progress"><div class="m-fill" id="m1-bar"></div></div>
      </div>
      <div class="m-bar-box">
        <div class="m-title">M2 FR</div>
        <div class="m-val" id="m2-val">0</div>
        <div class="m-progress"><div class="m-fill" id="m2-bar"></div></div>
      </div>
      <div class="m-bar-box">
        <div class="m-title">M3 RL</div>
        <div class="m-val" id="m3-val">0</div>
        <div class="m-progress"><div class="m-fill" id="m3-bar"></div></div>
      </div>
      <div class="m-bar-box">
        <div class="m-title">M4 RR</div>
        <div class="m-val" id="m4-val">0</div>
        <div class="m-progress"><div class="m-fill" id="m4-bar"></div></div>
      </div>
    </div>
  </div>

  <div class="slider-card">
    <div class="slider-header">
      <span>SPEED LIMIT / BENCH THROTTLE</span>
      <span id="spd-val" style="color:var(--blue);">30%</span>
    </div>
    <input type="range" id="spd-slider" min="10" max="100" value="30" oninput="document.getElementById('spd-val').innerText = this.value + '%'">
  </div>

  <div class="action-row">
    <button class="btn-act btn-start" onclick="sendCmd('START')">▶️ START<br><small>ENABLE H-BRIDGE</small></button>
    <button class="btn-act btn-stop" onclick="sendCmd('STOP')">⏹ STOP<br><small>RAMP TO 0</small></button>
    <button class="btn-act btn-brake" onclick="sendCmd('BRAKE')">⚡ BRAKE<br><small>ZERO PWM</small></button>
    <button class="btn-act btn-rev" onclick="sendCmd('REV')">💨 REV ENGINE<br><small>BLIP THROTTLE</small></button>
    <button class="btn-act btn-turbo" id="turbo-btn" onclick="toggleTurbo()">🚀 TURBO: OFF</button>
    <button class="btn-act" style="background:#00d2d3; color:#0b0e14;" onclick="sendCmd('STATUS')">📊 REFRESH<br><small>QUERY STATE</small></button>
    <button class="btn-act btn-estop" onclick="sendCmd('ESTOP')">⛔ EMERGENCY STOP</button>
  </div>

  <!-- Mecanum Omnidirectional Driving Pad -->
  <div class="dpad-card">
    <div style="font-size:11px; font-weight:700; color:var(--dim); margin-bottom:2px;">MECANUM OMNIDIRECTIONAL PAD (HOLD TO DRIVE)</div>
    <div class="dpad-grid">
      <button class="btn-pad" onpointerdown="startDrive(1, 0, -1)" onpointerup="stopDrive()">↖<small>DIAG L</small></button>
      <button class="btn-pad" onpointerdown="startDrive(1, 0, 0)" onpointerup="stopDrive()">▲<small>FWD</small></button>
      <button class="btn-pad" onpointerdown="startDrive(1, 0, 1)" onpointerup="stopDrive()">↗<small>DIAG R</small></button>

      <button class="btn-pad" onpointerdown="startDrive(0, 0, -1)" onpointerup="stopDrive()">◀<small>STRAFE L</small></button>
      <button class="btn-pad btn-center-stop" onclick="stopDrive()">⏹<small>STOP</small></button>
      <button class="btn-pad" onpointerdown="startDrive(0, 0, 1)" onpointerup="stopDrive()">▶<small>STRAFE R</small></button>

      <button class="btn-pad" onpointerdown="startDrive(0, -1, 0)" onpointerup="stopDrive()">↺<small>TURN L</small></button>
      <button class="btn-pad" onpointerdown="startDrive(-1, 0, 0)" onpointerup="stopDrive()">▼<small>REV</small></button>
      <button class="btn-pad" onpointerdown="startDrive(0, 1, 0)" onpointerup="stopDrive()">↻<small>TURN R</small></button>
    </div>
  <!-- Single Motor Diagnostic Testing -->
  <div style="background:var(--card); border:1px solid var(--border); border-radius:14px; padding:12px; display:flex; flex-direction:column; gap:8px;">
    <div style="font-size:12px; font-weight:800; color:var(--gold); text-align:center;">🧪 INDIVIDUAL MOTOR BENCH TESTING (1-by-1)</div>
    <div style="display:grid; grid-template-columns: 1fr 1fr; gap:8px;">
      <button class="btn-act" style="background:#2980b9; color:white; padding:10px;" onclick="testSingleMotor(1)">
        🔵 TEST M1 (FL)<br><small>D5 / PB4 (CN2 Pin 32) · CN4 2A+1A</small>
      </button>
      <button class="btn-act" style="background:#27ae60; color:white; padding:10px;" onclick="testSingleMotor(2)">
        🟢 TEST M2 (FR)<br><small>D4 / PB5 (CN2 Pin 33) · CN4 1B+2B</small>
      </button>
      <button class="btn-act" style="background:#e67e22; color:white; padding:10px;" onclick="testSingleMotor(3)">
        🟠 TEST M3 (RL)<br><small>A3 / PB0 (CN2 Pin 31) · CN5 2A+1A</small>
      </button>
      <button class="btn-act" style="background:#8e44ad; color:white; padding:10px;" onclick="testSingleMotor(4)">
        🟣 TEST M4 (RR)<br><small>CN10.24 / PB1 (CN2 Pin 36) · CN5 1B+2B</small>
      </button>
    </div>
    <button class="btn-act btn-stop" style="padding:8px;" onclick="sendCmd('STOP'); sendCmd('MOTOR:1,0'); sendCmd('MOTOR:2,0'); sendCmd('MOTOR:3,0'); sendCmd('MOTOR:4,0');">
      ⏹ STOP ALL INDIVIDUAL MOTORS
    </button>
  </div>

  <div class="log-box" id="logs">Connecting to server...</div>
</div>

<script>
  let driveTimer = null;
  let turboState = false;

  function sendCmd(cmd) {
    fetch('/api/cmd', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ cmd: cmd })
    }).catch(e => console.error(e));
  }

  function testSingleMotor(id) {
    const speed = parseInt(document.getElementById('spd-slider').value) || 30;
    sendCmd('RESET');
    sendCmd('START');
    sendCmd(`MOTOR:${id},${speed}`);
    // Auto stop after 1.5 seconds for safe bench testing
    setTimeout(() => {
      sendCmd(`MOTOR:${id},0`);
      sendCmd('STOP');
    }, 1500);
  }

  function startDrive(thFactor, stFactor, strfFactor) {
    stopDrive();
    const speed = parseInt(document.getElementById('spd-slider').value) || 30;
    const th = thFactor * speed;
    const st = stFactor * speed;
    const strf = strfFactor * speed;

    function doSend() {
      if (th !== 0 && st === 0 && strf === 0) {
        sendCmd(`THROTTLE:${th}`);
      } else if (st !== 0 && th === 0 && strf === 0) {
        sendCmd(`STEER:${st}`);
      } else if (strf !== 0 && th === 0 && st === 0) {
        sendCmd(`STRAFE:${strf}`);
      } else {
        sendCmd(`DRIVE:${th},${st},${strf}`);
      }
    }

    sendCmd('START');
    doSend();

    // Heartbeat every 100ms to stay ahead of 1500ms watchdog
    driveTimer = setInterval(doSend, 100);
  }

  function stopDrive() {
    if (driveTimer) {
      clearInterval(driveTimer);
      driveTimer = null;
    }
    sendCmd('STOP');
  }

  function toggleTurbo() {
    turboState = !turboState;
    sendCmd(turboState ? 'TURBO:ON' : 'TURBO:OFF');
    document.getElementById('turbo-btn').innerText = turboState ? '🚀 TURBO: ON' : '🚀 TURBO: OFF';
  }

  // Keyboard navigation
  const keys = {};
  window.addEventListener('keydown', (e) => {
    if (keys[e.code]) return;
    keys[e.code] = true;
    if (e.code === 'KeyW' || e.code === 'ArrowUp') startDrive(1, 0, 0);
    else if (e.code === 'KeyS' || e.code === 'ArrowDown') startDrive(-1, 0, 0);
    else if (e.code === 'KeyA') startDrive(0, 0, -1);
    else if (e.code === 'KeyD') startDrive(0, 0, 1);
    else if (e.code === 'ArrowLeft' || e.code === 'KeyQ') startDrive(0, -1, 0);
    else if (e.code === 'ArrowRight' || e.code === 'KeyE') startDrive(0, 1, 0);
    else if (e.code === 'Space') stopDrive();
  });

  window.addEventListener('keyup', (e) => {
    keys[e.code] = false;
    if (['KeyW', 'KeyS', 'KeyA', 'KeyD', 'KeyQ', 'KeyE', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(e.code)) {
      stopDrive();
    }
  });

  // Telemetry Polling
  setInterval(async () => {
    try {
      const res = await fetch('/api/status');
      const data = await res.json();
      const t = data.telemetry;

      // Update badge
      const badge = document.getElementById('badge');
      const connLbl = document.getElementById('conn-lbl');
      if (t.connected) {
        badge.className = 'badge connected';
        connLbl.innerText = 'ONLINE (' + t.state + ')';
      } else {
        badge.className = 'badge';
        connLbl.innerText = 'DISCONNECTED';
      }

      document.getElementById('rpm').innerText = t.rpm;
      document.getElementById('disp-throt').innerText = t.throttle + '%';
      document.getElementById('disp-gear').innerText = t.gear;
      document.getElementById('disp-boost').innerText = t.boost.toFixed(2);

      // Motor bars
      ['m1', 'm2', 'm3', 'm4'].forEach(m => {
        const val = t[m];
        document.getElementById(m + '-val').innerText = val;
        document.getElementById(m + '-bar').style.width = Math.min(100, Math.abs(val)) + '%';
      });

      // Update logs
      if (data.logs && data.logs.length) {
        const box = document.getElementById('logs');
        box.innerHTML = data.logs.map(l => '<div>' + l + '</div>').join('');
        box.scrollTop = box.scrollHeight;
      }
    } catch(e) {}
  }, 200);
</script>

</body>
</html>
"""

class RequestHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        # Silence default HTTP access logging
        pass

    def do_GET(self):
        if self.path == '/' or self.path == '/index.html':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.end_headers()
            self.wfile.write(HTML_PAGE.encode('utf-8'))
        elif self.path == '/api/status':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            with state_lock:
                resp = json.dumps({"telemetry": telemetry_data, "logs": recent_logs})
            self.wfile.write(resp.encode('utf-8'))
        else:
            self.send_response(404)
            self.end_headers()

    def do_POST(self):
        if self.path == '/api/cmd':
            content_len = int(self.headers.get('Content-Length', 0))
            post_body = self.rfile.read(content_len)
            try:
                data = json.loads(post_body.decode('utf-8'))
                cmd = data.get('cmd', '').strip()
                if cmd:
                    log_msg(f">> {cmd}")
                    send_serial(cmd)
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"ok": True, "cmd": cmd}).encode('utf-8'))
            except Exception as e:
                self.send_response(500)
                self.end_headers()
                self.wfile.write(json.dumps({"error": str(e)}).encode('utf-8'))
        else:
            self.send_response(404)
            self.end_headers()

def get_local_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(('8.8.8.8', 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return '127.0.0.1'

if __name__ == '__main__':
    t = threading.Thread(target=serial_reader_thread, daemon=True)
    t.start()

    local_ip = get_local_ip()
    server = HTTPServer(('0.0.0.0', PORT), RequestHandler)
    print("=" * 65)
    print("  STM32 RACECAR WEB SERIAL CONTROLLER SERVER")
    print("=" * 65)
    print(f"  Target Serial Port : {DEFAULT_SERIAL} @ {BAUD} Baud")
    print(f"  Local PC Browser   : http://localhost:{PORT}")
    print(f"  Mobile Phone / LAN : http://{local_ip}:{PORT}")
    print("=" * 65)
    print(" Server is LIVE! Press Ctrl+C in terminal to stop.")
    print("=" * 65)

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server...")
        if ser_device and ser_device.is_open:
            ser_device.close()
