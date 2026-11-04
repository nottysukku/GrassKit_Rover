#!/usr/bin/env python3
"""
GrassKit Rover Wi-Fi Controller
================================
Interactive teleoperation tool for controlling the GrassKit Rover
over Wi-Fi (SoftAP: GrassKit_Rover -> 192.168.4.1:8080).

Usage:
    python rover_wifi_controller.py [--ip 192.168.4.1] [--port 8080] [--cli]
"""

import socket
import sys
import time
import threading
import argparse

try:
    import tkinter as tk
    from tkinter import ttk, messagebox
    TK_AVAILABLE = True
except ImportError:
    TK_AVAILABLE = False


class RoverClient:
    def __init__(self, host="192.168.4.1", port=8080):
        self.host = host
        self.port = port
        self.sock = None
        self.connected = False
        self.rx_thread = None
        self.on_message_cb = None

    def connect(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(3.0)
            self.sock.connect((self.host, self.port))
            self.connected = True
            self.sock.settimeout(0.5)

            self.rx_thread = threading.Thread(target=self._rx_loop, daemon=True)
            self.rx_thread.start()
            return True, "Connected successfully"
        except Exception as e:
            self.connected = False
            return False, str(e)

    def disconnect(self):
        self.connected = False
        if self.sock:
            try:
                self.sock.close()
            except Exception:
                pass
            self.sock = None

    def send_cmd(self, cmd_str):
        if not self.connected or not self.sock:
            return False
        try:
            if not cmd_str.endswith("\n"):
                cmd_str += "\r\n"
            self.sock.sendall(cmd_str.encode("utf-8"))
            return True
        except Exception as e:
            print(f"[ERROR] Send failed: {e}")
            self.disconnect()
            return False

    def _rx_loop(self):
        buf = ""
        while self.connected and self.sock:
            try:
                data = self.sock.recv(1024)
                if not data:
                    print("\n[INFO] Connection closed by rover.")
                    self.connected = False
                    break
                buf += data.decode("utf-8", errors="ignore")
                while "\n" in buf:
                    line, buf = buf.split("\n", 1)
                    line = line.strip()
                    if line:
                        if self.on_message_cb:
                            self.on_message_cb(line)
                        else:
                            print(f"\r[ROVER] {line}\n> ", end="", flush=True)
            except socket.timeout:
                continue
            except Exception:
                break
        self.connected = False


def run_cli_mode(client):
    print("=" * 60)
    print(" GrassKit Rover Wi-Fi CLI Teleoperation")
    print("=" * 60)
    print(f"Connecting to {client.host}:{client.port}...")
    ok, err = client.connect()
    if not ok:
        print(f"[FAIL] Could not connect: {err}")
        print("Please verify that your PC is connected to the 'GrassKit_Rover' Wi-Fi network.")
        return

    print("[SUCCESS] Connected to Rover!")
    print("\nControls:")
    print("  [W] Forward      [S] Backward")
    print("  [A] Spin Left    [D] Spin Right")
    print("  [Space] Brake    [X] Emergency Stop")
    print("  [T<0-100>] Set Throttle (e.g. T60)")
    print("  [STATUS] Read Telemetry")
    print("  [Q] Quit")
    print("=" * 60)

    try:
        while client.connected:
            cmd = input("> ").strip()
            if not cmd:
                continue
            cmd_upper = cmd.upper()

            if cmd_upper == "Q" or cmd_upper == "QUIT":
                client.send_cmd("STOP")
                break
            elif cmd_upper == "W":
                client.send_cmd("FORWARD")
            elif cmd_upper == "S":
                client.send_cmd("BACKWARD")
            elif cmd_upper == "A":
                client.send_cmd("LEFT")
            elif cmd_upper == "D":
                client.send_cmd("RIGHT")
            elif cmd_upper == " " or cmd_upper == "STOP":
                client.send_cmd("STOP")
            elif cmd_upper == "X" or cmd_upper == "ESTOP":
                client.send_cmd("ESTOP")
            elif cmd_upper.startswith("T"):
                client.send_cmd(f"THROTTLE:{cmd_upper[1:]}")
            else:
                client.send_cmd(cmd)
    except KeyboardInterrupt:
        pass
    finally:
        client.send_cmd("STOP")
        client.disconnect()
        print("\nDisconnected.")


def run_gui_mode(client):
    if not TK_AVAILABLE:
        print("Tkinter not available. Falling back to CLI mode.")
        run_cli_mode(client)
        return

    root = tk.Tk()
    root.title("GrassKit Rover — Wi-Fi Teleoperation Panel")
    root.geometry("640x700")
    root.configure(bg="#1E1E2E")

    style = ttk.Style()
    style.theme_use("clam")

    # Header
    header = tk.Label(root, text="GRASSKIT ROVER Wi-Fi CONTROL", font=("Helvetica", 16, "bold"),
                      fg="#89B4FA", bg="#1E1E2E")
    header.pack(pady=10)

    # Connection Status
    conn_frame = tk.Frame(root, bg="#313244", padx=10, pady=8)
    conn_frame.pack(fill="x", padx=15, pady=5)

    status_lbl = tk.Label(conn_frame, text="Status: Disconnected", font=("Helvetica", 11, "bold"),
                          fg="#F38BA8", bg="#313244")
    status_lbl.pack(side="left")

    def toggle_connect():
        if not client.connected:
            ok, err = client.connect()
            if ok:
                status_lbl.config(text=f"Connected: {client.host}:{client.port}", fg="#A6E3A1")
                btn_connect.config(text="Disconnect")
                client.send_cmd("STATUS")
            else:
                messagebox.showerror("Connection Error", f"Failed to connect:\n{err}\n\nMake sure your PC is connected to the 'GrassKit_Rover' Wi-Fi network.")
        else:
            client.disconnect()
            status_lbl.config(text="Status: Disconnected", fg="#F38BA8")
            btn_connect.config(text="Connect")

    btn_connect = tk.Button(conn_frame, text="Connect", command=toggle_connect,
                            bg="#89B4FA", fg="#11111B", font=("Helvetica", 10, "bold"), relief="flat", padx=10)
    btn_connect.pack(side="right")

    # Drive Controls
    drive_frame = tk.LabelFrame(root, text=" Motion Controls ", font=("Helvetica", 12, "bold"),
                                fg="#CDD6F4", bg="#1E1E2E", padx=15, pady=15)
    drive_frame.pack(fill="x", padx=15, pady=10)

    btn_fwd = tk.Button(drive_frame, text="▲ FORWARD", font=("Helvetica", 12, "bold"),
                        bg="#A6E3A1", fg="#11111B", width=12, height=2,
                        command=lambda: client.send_cmd("FORWARD"))
    btn_fwd.grid(row=0, column=1, pady=5)

    btn_left = tk.Button(drive_frame, text="◄ SPIN LEFT", font=("Helvetica", 12, "bold"),
                         bg="#89DCEB", fg="#11111B", width=12, height=2,
                         command=lambda: client.send_cmd("LEFT"))
    btn_left.grid(row=1, column=0, padx=5, pady=5)

    btn_stop = tk.Button(drive_frame, text="■ STOP", font=("Helvetica", 12, "bold"),
                         bg="#F38BA8", fg="#11111B", width=12, height=2,
                         command=lambda: client.send_cmd("STOP"))
    btn_stop.grid(row=1, column=1, padx=5, pady=5)

    btn_right = tk.Button(drive_frame, text="SPIN RIGHT ►", font=("Helvetica", 12, "bold"),
                          bg="#89DCEB", fg="#11111B", width=12, height=2,
                          command=lambda: client.send_cmd("RIGHT"))
    btn_right.grid(row=1, column=2, padx=5, pady=5)

    btn_bwd = tk.Button(drive_frame, text="▼ BACKWARD", font=("Helvetica", 12, "bold"),
                        bg="#FAB387", fg="#11111B", width=12, height=2,
                        command=lambda: client.send_cmd("BACKWARD"))
    btn_bwd.grid(row=2, column=1, pady=5)

    # Speed Throttle Slider
    speed_frame = tk.Frame(drive_frame, bg="#1E1E2E")
    speed_frame.grid(row=3, column=0, columnspan=3, sticky="ew", pady=10)

    tk.Label(speed_frame, text="Throttle %:", font=("Helvetica", 10, "bold"),
             fg="#CDD6F4", bg="#1E1E2E").pack(side="left", padx=5)

    speed_scale = tk.Scale(speed_frame, from_=0, to=100, orient="horizontal",
                           bg="#313244", fg="#CDD6F4", highlightthickness=0, length=380)
    speed_scale.set(45)
    speed_scale.pack(side="left", padx=5)

    def apply_speed(val):
        client.send_cmd(f"THROTTLE:{speed_scale.get()}")

    btn_set_spd = tk.Button(speed_frame, text="Set", command=lambda: apply_speed(speed_scale.get()),
                            bg="#F9E2AF", fg="#11111B", font=("Helvetica", 9, "bold"))
    btn_set_spd.pack(side="left", padx=5)

    # Emergency Actions
    action_frame = tk.Frame(root, bg="#1E1E2E")
    action_frame.pack(fill="x", padx=15, pady=5)

    btn_estop = tk.Button(action_frame, text="🚨 EMERGENCY STOP", font=("Helvetica", 11, "bold"),
                          bg="#RED", fg="white", width=20, height=2,
                          command=lambda: client.send_cmd("ESTOP"))
    btn_estop.pack(side="left", expand=True, padx=5)

    btn_reset = tk.Button(action_frame, text="🔄 RESET FAULT", font=("Helvetica", 11, "bold"),
                          bg="#45475A", fg="#CDD6F4", width=20, height=2,
                          command=lambda: client.send_cmd("RESET"))
    btn_reset.pack(side="right", expand=True, padx=5)

    # Telemetry Log Display
    log_frame = tk.LabelFrame(root, text=" Live Telemetry & Console ", font=("Helvetica", 11, "bold"),
                              fg="#CDD6F4", bg="#1E1E2E", padx=10, pady=5)
    log_frame.pack(fill="both", expand=True, padx=15, pady=10)

    log_text = tk.Text(log_frame, bg="#11111B", fg="#A6E3A1", font=("Consolas", 9), height=10)
    log_text.pack(fill="both", expand=True)

    def handle_incoming_msg(msg):
        def _update():
            log_text.insert(tk.END, msg + "\n")
            log_text.see(tk.END)
        root.after(0, _update)

    client.on_message_cb = handle_incoming_msg

    # Keyboard Binds
    root.bind("<Up>", lambda e: client.send_cmd("FORWARD"))
    root.bind("<Down>", lambda e: client.send_cmd("BACKWARD"))
    root.bind("<Left>", lambda e: client.send_cmd("LEFT"))
    root.bind("<Right>", lambda e: client.send_cmd("RIGHT"))
    root.bind("<space>", lambda e: client.send_cmd("STOP"))

    def on_closing():
        client.disconnect()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_closing)
    root.mainloop()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="GrassKit Rover Wi-Fi Controller")
    parser.add_argument("--ip", default="192.168.4.1", help="Rover IP (default: 192.168.4.1)")
    parser.add_argument("--port", type=int, default=8080, help="Rover TCP Port (default: 8080)")
    parser.add_argument("--cli", action="store_true", help="Force command-line interface mode")
    args = parser.parse_args()

    rover_client = RoverClient(args.ip, args.port)

    if args.cli or not TK_AVAILABLE:
        run_cli_mode(rover_client)
    else:
        run_gui_mode(rover_client)
