#!/usr/bin/env python3
"""
================================================================================
Race Car Demonstrator - PC & Bluetooth Control Application
================================================================================
Target Hardware:
  - Controller: STM32 NUCLEO-L073RZ
  - Motor Driver: ST X-STM32MP-RBT01 (Dual STSPIN948, 4 DC Motors, Mecanum)
  - Audio: MAX98357A I2S Class-D Mono Amplifier + Speaker
  - Wireless: X-NUCLEO-IDB05A2 / Bluetooth UART / ST-LINK VCP

Features:
  - Real-time Tachometer / RPM Canvas Gauge
  - Throttle & Steering Sliders
  - 4-Wheel Mecanum Omnidirectional Strafe Controls
  - Virtual 6-Speed Gearbox Selector
  - Turbo Boost & Rev Blip Controls
  - Big Hardware/Software Emergency Stop
  - Live Telemetry Parser & Diagnostic Console
================================================================================
"""

import sys
import time
import math
import threading
import tkinter as tk
from tkinter import ttk, messagebox

try:
    import serial
    import serial.tools.list_ports
    SERIAL_AVAILABLE = True
except ImportError:
    SERIAL_AVAILABLE = False


class RaceCarControllerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("🏎️ STM32 Race Car & Mecanum Sound Demonstrator")
        self.root.geometry("980x740")
        self.root.configure(bg="#121216")

        self.ser = None
        self.rx_thread = None
        self.is_running = False

        # Vehicle State
        self.current_rpm = 900.0
        self.throttle_val = 0
        self.steer_val = 0
        self.strafe_val = 0
        self.gear_val = 1
        self.turbo_val = False
        self.status_state = "DISCONNECTED"

        self.create_ui()
        self.update_port_list()

    def create_ui(self):
        # 1. Top Connection Bar
        top_bar = tk.Frame(self.root, bg="#1c1d24", padx=10, pady=8)
        top_bar.pack(fill=tk.X, side=tk.TOP)

        tk.Label(top_bar, text="SERIAL / BLUETOOTH PORT:", fg="#8b949e", bg="#1c1d24", font=("Consolas", 10, "bold")).pack(side=tk.LEFT, padx=5)

        self.port_combobox = ttk.Combobox(top_bar, width=24, state="readonly")
        self.port_combobox.pack(side=tk.LEFT, padx=5)

        tk.Label(top_bar, text="BAUD:", fg="#8b949e", bg="#1c1d24", font=("Consolas", 10)).pack(side=tk.LEFT, padx=5)
        self.baud_combobox = ttk.Combobox(top_bar, width=10, values=["115200", "9600"], state="readonly")
        self.baud_combobox.set("115200")
        self.baud_combobox.pack(side=tk.LEFT, padx=5)

        self.btn_refresh = tk.Button(top_bar, text="🔄 REFRESH", bg="#2a2e39", fg="#e6edf3", font=("Segoe UI", 9), relief=tk.FLAT, command=self.update_port_list)
        self.btn_refresh.pack(side=tk.LEFT, padx=6)

        self.btn_connect = tk.Button(top_bar, text="⚡ CONNECT", bg="#238636", fg="white", font=("Segoe UI", 9, "bold"), relief=tk.FLAT, command=self.toggle_connection)
        self.btn_connect.pack(side=tk.LEFT, padx=6)

        self.lbl_status = tk.Label(top_bar, text="● DISCONNECTED", fg="#f85149", bg="#1c1d24", font=("Consolas", 10, "bold"))
        self.lbl_status.pack(side=tk.RIGHT, padx=10)

        # 2. Main Dashboard Split (Left: Gauge & Shifter, Right: Driving Controls & Log)
        main_body = tk.Frame(self.root, bg="#121216", padx=10, pady=10)
        main_body.pack(fill=tk.BOTH, expand=True)

        left_col = tk.Frame(main_body, bg="#181a20", bd=1, relief=tk.SOLID, padx=12, pady=12)
        left_col.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 6))

        right_col = tk.Frame(main_body, bg="#181a20", bd=1, relief=tk.SOLID, padx=12, pady=12)
        right_col.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(6, 0))

        # --- LEFT COLUMN: TACHOMETER & ENGINE DASHBOARD ---
        tk.Label(left_col, text="RACE CAR ENGINE SOUND DASHBOARD", fg="#58a6ff", bg="#181a20", font=("Segoe UI", 12, "bold")).pack(anchor=tk.W)

        # Tachometer Canvas Gauge
        self.tacho_canvas = tk.Canvas(left_col, width=320, height=220, bg="#121216", highlightthickness=0)
        self.tacho_canvas.pack(pady=10)
        self.draw_tachometer(900.0)

        # Digital Readouts Frame
        readouts_frame = tk.Frame(left_col, bg="#181a20")
        readouts_frame.pack(fill=tk.X, pady=5)

        self.lbl_rpm_digital = tk.Label(readouts_frame, text="900 RPM", fg="#3fb950", bg="#181a20", font=("Consolas", 20, "bold"))
        self.lbl_rpm_digital.pack(side=tk.LEFT, expand=True)

        self.lbl_gear_digital = tk.Label(readouts_frame, text="GEAR: 1", fg="#f0883e", bg="#181a20", font=("Consolas", 20, "bold"))
        self.lbl_gear_digital.pack(side=tk.RIGHT, expand=True)

        # Gear Selector Buttons
        gear_frame = tk.LabelFrame(left_col, text="SEQUENTIAL GEARBOX", fg="#8b949e", bg="#181a20", font=("Segoe UI", 9, "bold"))
        gear_frame.pack(fill=tk.X, pady=8)

        btn_row = tk.Frame(gear_frame, bg="#181a20")
        btn_row.pack(pady=6)
        for g in range(1, 7):
            b = tk.Button(btn_row, text=f"G{g}", width=4, bg="#2a2e39", fg="white", font=("Consolas", 10, "bold"),
                          command=lambda gear=g: self.set_gear(gear))
            b.pack(side=tk.LEFT, padx=3)

        b_auto = tk.Button(btn_row, text="AUTO", width=6, bg="#1f6feb", fg="white", font=("Consolas", 9, "bold"),
                           command=lambda: self.send_command("GEAR:AUTO\r\n"))
        b_auto.pack(side=tk.LEFT, padx=4)

        # Special Effects Row (Rev Blip & Turbo Boost)
        fx_frame = tk.Frame(left_col, bg="#181a20")
        fx_frame.pack(fill=tk.X, pady=6)

        self.btn_turbo = tk.Button(fx_frame, text="🚀 TURBO: OFF", bg="#30363d", fg="#8b949e", font=("Segoe UI", 10, "bold"), width=16, command=self.toggle_turbo)
        self.btn_turbo.pack(side=tk.LEFT, padx=4, expand=True, fill=tk.X)

        self.btn_rev = tk.Button(fx_frame, text="💨 REV ENGINE", bg="#d29922", fg="#0d1117", font=("Segoe UI", 10, "bold"), width=16, command=self.rev_engine)
        self.btn_rev.pack(side=tk.RIGHT, padx=4, expand=True, fill=tk.X)

        # Big Emergency Stop Button
        self.btn_estop = tk.Button(left_col, text="⛔ EMERGENCY STOP", bg="#da3633", fg="white", font=("Segoe UI", 14, "bold"), height=2, command=self.trigger_estop)
        self.btn_estop.pack(fill=tk.X, pady=(12, 4))

        self.btn_reset = tk.Button(left_col, text="RE-ARM / RESET", bg="#21262d", fg="#8b949e", font=("Segoe UI", 9), command=self.trigger_reset)
        self.btn_reset.pack(fill=tk.X)

        # --- RIGHT COLUMN: DRIVE CONTROLS & TELEMETRY ---
        tk.Label(right_col, text="MECANUM 4WD & RACE CAR CONTROLS", fg="#58a6ff", bg="#181a20", font=("Segoe UI", 12, "bold")).pack(anchor=tk.W)

        # Mode Selection
        mode_frame = tk.Frame(right_col, bg="#181a20")
        mode_frame.pack(fill=tk.X, pady=6)
        self.drive_mode_var = tk.StringVar(value="RACECAR")
        tk.Radiobutton(mode_frame, text="🏎️ Race Car Mode (Throttle + Steer)", variable=self.drive_mode_var, value="RACECAR",
                       fg="#e6edf3", bg="#181a20", selectcolor="#181a20", font=("Segoe UI", 9), command=self.update_drive_mode).pack(side=tk.LEFT, padx=6)
        tk.Radiobutton(mode_frame, text="🛸 Mecanum Omni Mode (Strafe)", variable=self.drive_mode_var, value="MECANUM",
                       fg="#e6edf3", bg="#181a20", selectcolor="#181a20", font=("Segoe UI", 9), command=self.update_drive_mode).pack(side=tk.LEFT, padx=6)

        # Sliders Frame
        sliders_frame = tk.Frame(right_col, bg="#181a20")
        sliders_frame.pack(fill=tk.X, pady=4)

        # Throttle Slider
        tk.Label(sliders_frame, text="THROTTLE (%):", fg="#8b949e", bg="#181a20", font=("Consolas", 9)).grid(row=0, column=0, sticky="w", pady=2)
        self.scale_throttle = tk.Scale(sliders_frame, from_=0, to=100, orient=tk.HORIZONTAL, bg="#181a20", fg="#3fb950",
                                       highlightthickness=0, command=self.on_slider_change)
        self.scale_throttle.grid(row=0, column=1, sticky="ew", padx=6)

        # Steering Slider
        tk.Label(sliders_frame, text="STEERING (%):", fg="#8b949e", bg="#181a20", font=("Consolas", 9)).grid(row=1, column=0, sticky="w", pady=2)
        self.scale_steer = tk.Scale(sliders_frame, from_=-100, to=100, orient=tk.HORIZONTAL, bg="#181a20", fg="#58a6ff",
                                    highlightthickness=0, command=self.on_slider_change)
        self.scale_steer.grid(row=1, column=1, sticky="ew", padx=6)

        # Strafe Slider
        tk.Label(sliders_frame, text="STRAFE (%):", fg="#8b949e", bg="#181a20", font=("Consolas", 9)).grid(row=2, column=0, sticky="w", pady=2)
        self.scale_strafe = tk.Scale(sliders_frame, from_=-100, to=100, orient=tk.HORIZONTAL, bg="#181a20", fg="#bc8cff",
                                     highlightthickness=0, command=self.on_slider_change)
        self.scale_strafe.grid(row=2, column=1, sticky="ew", padx=6)

        sliders_frame.columnconfigure(1, weight=1)

        # Mecanum Strafe Direction Pad
        pad_frame = tk.LabelFrame(right_col, text="MECANUM DIRECTIONAL PAD", fg="#8b949e", bg="#181a20", font=("Segoe UI", 9, "bold"))
        pad_frame.pack(fill=tk.X, pady=6)

        grid_pad = tk.Frame(pad_frame, bg="#181a20")
        grid_pad.pack(pady=4)

        tk.Button(grid_pad, text="▲ FWD", width=8, bg="#238636", fg="white", font=("Segoe UI", 9, "bold"),
                  command=lambda: self.quick_drive(60, 0, 0)).grid(row=0, column=1, padx=3, pady=3)
        tk.Button(grid_pad, text="◀ STRAFE L", width=10, bg="#1f6feb", fg="white", font=("Segoe UI", 9, "bold"),
                  command=lambda: self.quick_drive(0, 0, -60)).grid(row=1, column=0, padx=3, pady=3)
        tk.Button(grid_pad, text="⏹ STOP", width=8, bg="#30363d", fg="#f85149", font=("Segoe UI", 9, "bold"),
                  command=self.quick_stop).grid(row=1, column=1, padx=3, pady=3)
        tk.Button(grid_pad, text="STRAFE R ▶", width=10, bg="#1f6feb", fg="white", font=("Segoe UI", 9, "bold"),
                  command=lambda: self.quick_drive(0, 0, 60)).grid(row=1, column=2, padx=3, pady=3)
        tk.Button(grid_pad, text="▼ REV", width=8, bg="#8957e5", fg="white", font=("Segoe UI", 9, "bold"),
                  command=lambda: self.quick_drive(-50, 0, 0)).grid(row=2, column=1, padx=3, pady=3)

        # Terminal / Log Box
        tk.Label(right_col, text="COMMUNICATION LOG:", fg="#8b949e", bg="#181a20", font=("Consolas", 9)).pack(anchor=tk.W, pady=(8, 2))
        self.txt_log = tk.Text(right_col, height=10, bg="#0d1117", fg="#c9d1d9", font=("Consolas", 8), bd=0)
        self.txt_log.pack(fill=tk.BOTH, expand=True)

    def draw_tachometer(self, rpm):
        self.tacho_canvas.delete("all")
        cx, cy, r = 160, 160, 120

        # Draw outer dial arc (from 140 deg to 40 deg, clockwise)
        self.tacho_canvas.create_arc(cx - r, cy - r, cx + r, cy + r, start=40, extent=260,
                                     style=tk.ARC, outline="#30363d", width=12)

        # Redline zone (6000 to 7200 RPM)
        self.tacho_canvas.create_arc(cx - r, cy - r, cx + r, cy + r, start=40, extent=42,
                                     style=tk.ARC, outline="#da3633", width=12)

        # Scale tick marks (0 to 8 x 1000 RPM)
        for i in range(9):
            val_rpm = i * 1000
            angle_deg = 220 - (val_rpm / 8000.0) * 260
            rad = math.radians(angle_deg)
            x1 = cx + (r - 18) * math.cos(rad)
            y1 = cy - (r - 18) * math.sin(rad)
            x2 = cx + (r - 4) * math.cos(rad)
            y2 = cy - (r - 4) * math.sin(rad)
            col = "#da3633" if val_rpm >= 6000 else "#8b949e"
            self.tacho_canvas.create_line(x1, y1, x2, y2, fill=col, width=2)

            tx = cx + (r - 32) * math.cos(rad)
            ty = cy - (r - 32) * math.sin(rad)
            self.tacho_canvas.create_text(tx, ty, text=str(i), fill=col, font=("Segoe UI", 9, "bold"))

        # Needle Calculation
        clamped_rpm = max(0, min(8000, rpm))
        needle_angle_deg = 220 - (clamped_rpm / 8000.0) * 260
        needle_rad = math.radians(needle_angle_deg)
        nx = cx + (r - 14) * math.cos(needle_rad)
        ny = cy - (r - 14) * math.sin(needle_rad)

        self.tacho_canvas.create_line(cx, cy, nx, ny, fill="#ff7b72", width=4, capstyle=tk.ROUND)
        self.tacho_canvas.create_oval(cx - 10, cy - 10, cx + 10, cy + 10, fill="#e6edf3", outline="#30363d")

    def update_port_list(self):
        if not SERIAL_AVAILABLE:
            self.port_combobox["values"] = ["No pyserial installed"]
            return
        ports = serial.tools.list_ports.comports()
        port_names = [f"{p.device} ({p.description})" for p in ports]
        self.port_combobox["values"] = port_names
        if port_names:
            self.port_combobox.current(0)

    def toggle_connection(self):
        if self.ser and self.ser.is_open:
            self.disconnect_serial()
        else:
            self.connect_serial()

    def connect_serial(self):
        if not SERIAL_AVAILABLE:
            messagebox.showerror("Error", "pyserial library not found. Run 'pip install pyserial'")
            return

        port_str = self.port_combobox.get().split()[0] if self.port_combobox.get() else ""
        if not port_str:
            messagebox.showwarning("Warning", "Please select a serial / Bluetooth COM port.")
            return

        try:
            baud = int(self.baud_combobox.get())
            self.ser = serial.Serial(port_str, baud, timeout=0.1)
            self.is_running = True
            self.rx_thread = threading.Thread(target=self.rx_loop, daemon=True)
            self.rx_thread.start()

            self.btn_connect.config(text="🔌 DISCONNECT", bg="#da3633")
            self.lbl_status.config(text=f"● CONNECTED ({port_str})", fg="#3fb950")
            self.log(f"Connected to {port_str} at {baud} baud.")
            self.send_command("STATUS\r\n")
        except Exception as e:
            messagebox.showerror("Connection Error", str(e))

    def disconnect_serial(self):
        self.is_running = False
        if self.ser:
            try:
                self.ser.close()
            except Exception:
                pass
            self.ser = None
        self.btn_connect.config(text="⚡ CONNECT", bg="#238636")
        self.lbl_status.config(text="● DISCONNECTED", fg="#f85149")
        self.log("Disconnected.")

    def rx_loop(self):
        while self.is_running and self.ser and self.ser.is_open:
            try:
                line = self.ser.readline().decode("utf-8", errors="ignore").strip()
                if line:
                    self.root.after(0, self.handle_rx_line, line)
            except Exception:
                break
        self.root.after(0, self.disconnect_serial)

    def handle_rx_line(self, line):
        self.log(f"<< {line}")
        # Parse STATUS or TELEMETRY:
        # e.g.: TELE:RPM=3450,THROT=45%,GEAR=3,BOOST=0.6...
        if "RPM=" in line:
            try:
                parts = dict(item.split("=") for item in line.replace("TELE:", "").replace("STATUS:", "").split(","))
                if "RPM" in parts:
                    rpm = float(parts["RPM"])
                    self.current_rpm = rpm
                    self.draw_tachometer(rpm)
                    self.lbl_rpm_digital.config(text=f"{int(rpm)} RPM")
                if "GEAR" in parts:
                    self.lbl_gear_digital.config(text=f"GEAR: {parts['GEAR']}")
            except Exception:
                pass

    def send_command(self, cmd_str):
        if self.ser and self.ser.is_open:
            try:
                self.ser.write(cmd_str.encode("utf-8"))
                self.log(f">> {cmd_str.strip()}")
            except Exception as e:
                self.log(f"TX Error: {e}")

    def on_slider_change(self, _=None):
        th = self.scale_throttle.get()
        st = self.scale_steer.get()
        strf = self.scale_strafe.get()
        cmd = f"DRIVE:{th},{st},{strf}\r\n"
        self.send_command(cmd)

    def quick_drive(self, th, st, strf):
        self.scale_throttle.set(th)
        self.scale_steer.set(st)
        self.scale_strafe.set(strf)
        self.on_slider_change()

    def quick_stop(self):
        self.scale_throttle.set(0)
        self.scale_steer.set(0)
        self.scale_strafe.set(0)
        self.send_command("STOP\r\n")

    def set_gear(self, gear):
        self.send_command(f"GEAR:{gear}\r\n")

    def toggle_turbo(self):
        self.turbo_val = not self.turbo_val
        if self.turbo_val:
            self.btn_turbo.config(text="🚀 TURBO: ON", bg="#1f6feb", fg="white")
            self.send_command("TURBO:ON\r\n")
        else:
            self.btn_turbo.config(text="🚀 TURBO: OFF", bg="#30363d", fg="#8b949e")
            self.send_command("TURBO:OFF\r\n")

    def rev_engine(self):
        self.send_command("REV\r\n")

    def trigger_estop(self):
        self.send_command("ESTOP\r\n")
        self.scale_throttle.set(0)
        self.scale_steer.set(0)
        self.scale_strafe.set(0)

    def trigger_reset(self):
        self.send_command("RESET\r\n")

    def update_drive_mode(self):
        mode = self.drive_mode_var.get()
        self.send_command(f"MODE:{mode}\r\n")

    def log(self, msg):
        self.txt_log.insert(tk.END, f"{msg}\n")
        self.txt_log.see(tk.END)


if __name__ == "__main__":
    tk_root = tk.Tk()
    app = RaceCarControllerApp(tk_root)
    tk_root.mainloop()
