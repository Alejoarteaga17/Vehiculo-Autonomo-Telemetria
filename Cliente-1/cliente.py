"""Cliente Python compatible con el servidor C (protocolo JSON).

Principales correcciones respecto a versión anterior:
- Mensaje HELLO ahora usa {"t":"HELLO","role":"...","token":"SECRETO_2025"} para admin.
- Comandos usan campo "name" y nombres con guion bajo (p.ej. SPEED_UP) según servidor.
- Se parsean mensajes de tipo TELEMETRY y DATA (el servidor emite TELEMETRY periódico
  y DATA cuando respondemos a GET_DATA).
- Se solicita un GET_DATA inicial tras el HELLO para obtener estado inmediato.
- Se soporta modo OBSERVER sin botones.
"""

import socket
import threading
import json
import tkinter as tk
from tkinter import messagebox
import time
import os
import csv


class VehiculoClient:
    def __init__(self, host="127.0.0.1", port=5000, admin=False, token=None, auto_poll=True, poll_interval=10,
                 log_dir="logs", log_format="csv"):
        self.host = host
        self.port = port
        self.admin = admin
        self.token = token or ("SECRETO_2025" if admin else None)
        self.auto_poll = auto_poll  # si True, pedirá GET_DATA periódicamente además de TELEMETRY
        self.poll_interval = poll_interval
        self.log_dir = log_dir
        self.log_format = log_format.lower()  # 'csv' o 'json'
        self._log_file = None
        self._log_writer = None
        self._ensure_logger()

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._recv_thread = None
        self._poll_thread = None
        self._stop = threading.Event()

        # GUI
        self.root = tk.Tk()
        self.root.title(f"Cliente Vehículo Autónomo ({'Admin' if admin else 'Observer'})")
        self.speed_label = tk.Label(self.root, text="Velocidad: --")
        self.battery_label = tk.Label(self.root, text="Batería: --")
        self.temp_label = tk.Label(self.root, text="Temperatura: --")
        self.dir_label = tk.Label(self.root, text="Dirección: --")
        self.status_label = tk.Label(self.root, text="Estado: Conectando...")
        self.last_update_label = tk.Label(self.root, text="Última actualización: --")
        self.speed_label.pack(pady=3)
        self.battery_label.pack(pady=3)
        self.temp_label.pack(pady=3)
        self.dir_label.pack(pady=3)
        self.last_update_label.pack(pady=3)
        self.status_label.pack(pady=6)

        # Frame de comandos (solo admin)
        self.cmd_buttons = []
        if self.admin:
            frame = tk.Frame(self.root, bd=1, relief=tk.GROOVE, padx=5, pady=5)
            frame.pack(pady=8)
            buttons = [
                ("Speed +", "SPEED_UP"),
                ("Speed -", "SLOW_DOWN"),
                ("Turn Left", "TURN_LEFT"),
                ("Turn Right", "TURN_RIGHT"),
            ]
            for i, (label, cmd) in enumerate(buttons):
                b = tk.Button(frame, text=label, width=12, command=lambda c=cmd: self.send_cmd(c))
                b.grid(row=i // 2, column=i % 2, padx=4, pady=4)
                self.cmd_buttons.append(b)

        # Cerrar limpio
        self.root.protocol("WM_DELETE_WINDOW", self.close)

    # ================= Conexión =================
    def connect(self):
        try:
            self.sock.connect((self.host, self.port))
            self.status_label.config(text="Estado: Conectado (enviando HELLO)")
            hello = {"t": "HELLO", "role": "ADMIN", "token": self.token} if self.admin else {"t": "HELLO", "role": "OBSERVER"}
            self._send_json(hello)
            # Thread recepción
            self._recv_thread = threading.Thread(target=self._recv_loop, daemon=True)
            self._recv_thread.start()
            # GET_DATA inicial para tener datos instantáneos
            self.request_data()
            # Thread polling periódico opcional
            if self.auto_poll:
                self._poll_thread = threading.Thread(target=self._poll_loop, daemon=True)
                self._poll_thread.start()
            self.root.mainloop()
        except Exception as e:
            messagebox.showerror("Error de conexión", str(e))

    def _poll_loop(self):
        while not self._stop.is_set():
            time.sleep(self.poll_interval)
            if self._stop.is_set():
                break
            try:
                self.request_data()
            except Exception:
                pass

    def _recv_loop(self):
        buffer = ""
        try:
            while not self._stop.is_set():
                chunk = self.sock.recv(2048)
                if not chunk:
                    break
                buffer += chunk.decode(errors='replace')
                while "\n" in buffer:
                    line, buffer = buffer.split("\n", 1)
                    line = line.strip()
                    if line:
                        self._handle_line(line)
        except Exception as e:
            print("[RX] Error:", e)
        finally:
            self.status_label.config(text="Estado: Desconectado")

    # ================= Envío / Peticiones =================
    def _send_json(self, obj):
        data = json.dumps(obj, separators=(',', ':')) + "\n"
        self.sock.sendall(data.encode())

    def send_cmd(self, name):
        if not self.admin:
            messagebox.showwarning("No autorizado", "Rol OBSERVER no puede enviar comandos")
            return
        try:
            self._send_json({"t": "CMD", "name": name})
        except Exception as e:
            messagebox.showerror("Error enviando comando", str(e))

    def request_data(self):
        self._send_json({"t": "GET_DATA"})

    # ================= Procesamiento mensajes =================
    def _handle_line(self, line: str):
        try:
            msg = json.loads(line)
        except json.JSONDecodeError:
            print("[WARN] JSON inválido:", line)
            return

        t = msg.get("t")
        if t in ("DATA", "TELEMETRY"):
            self._update_telemetry(msg, t)
        elif t == "HELLO_OK":
            role = msg.get("role", "?")
            self.status_label.config(text=f"Estado: Autenticado como {role}")
        elif t in ("ACK", "NACK"):
            # Mostrar breve feedback en etiqueta status
            if t == "ACK":
                self.status_label.config(text=f"ACK {msg.get('name','')} OK")
            else:
                reason = msg.get("reason", "?")
                self.status_label.config(text=f"NACK {msg.get('name','')} ({reason})")
        elif t == "ERR":
            self.status_label.config(text=f"ERROR: {msg.get('code','?')}")
        else:
            # Otros (USERS, PONG, etc.)
            print("[INFO]", msg)

    def _update_telemetry(self, msg, origin):
        self.speed_label.config(text=f"Velocidad: {msg.get('speed','--')}")
        self.battery_label.config(text=f"Batería: {msg.get('battery','--')}")
        self.temp_label.config(text=f"Temperatura: {msg.get('temp','--')}")
        self.dir_label.config(text=f"Dirección: {msg.get('dir','--')}")
        ts = msg.get('ts', int(time.time()))
        human = time.strftime('%H:%M:%S', time.localtime(ts))
        self.last_update_label.config(text=f"Última actualización: {human} ({origin})")
        # Log local
        self._log_telemetry(msg, origin)

    # ================= Logging =================
    def _ensure_logger(self):
        try:
            if not os.path.isdir(self.log_dir):
                os.makedirs(self.log_dir, exist_ok=True)
            fname = time.strftime('telemetria_%Y%m%d')
            if self.log_format == 'csv':
                path = os.path.join(self.log_dir, fname + '.csv')
                fresh = not os.path.exists(path)
                self._log_file = open(path, 'a', newline='', encoding='utf-8')
                self._log_writer = csv.writer(self._log_file)
                if fresh:
                    self._log_writer.writerow(['epoch','human','origin','speed','battery','temp','dir'])
            else:
                path = os.path.join(self.log_dir, fname + '.jsonl')
                self._log_file = open(path, 'a', encoding='utf-8')
        except Exception as e:
            print('[LOG] No se pudo inicializar logging:', e)
            self._log_file = None

    def _log_telemetry(self, msg, origin):
        if not self._log_file:
            return
        try:
            epoch = msg.get('ts', int(time.time()))
            human = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(epoch))
            speed = msg.get('speed')
            battery = msg.get('battery')
            temp = msg.get('temp')
            direction = msg.get('dir')
            if self.log_format == 'csv':
                if self._log_writer:
                    self._log_writer.writerow([epoch, human, origin, speed, battery, temp, direction])
                    self._log_file.flush()
            else:
                record = {"ts": epoch, "human": human, "origin": origin, "speed": speed,
                          "battery": battery, "temp": temp, "dir": direction}
                self._log_file.write(json.dumps(record, ensure_ascii=False) + '\n')
                self._log_file.flush()
        except Exception as e:
            print('[LOG] Error escribiendo telemetría:', e)

    # ================= Cierre =================
    def close(self):
        self._stop.set()
        try:
            self.sock.close()
        except Exception:
            pass
        try:
            if self._log_file:
                self._log_file.close()
        except Exception:
            pass
        self.root.destroy()

