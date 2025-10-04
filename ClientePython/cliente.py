import socket
import threading
import tkinter as tk
from tkinter import messagebox
import time
import os
import csv

class VehiculoClient:
    def __init__(self, host="127.0.0.1", port=8080, admin=False, token=None,
                 log_dir="logs", log_format="csv"):
        self.host = host
        self.port = port
        self.admin = admin
        self.token = token or ("SECRETO_2025" if admin else None)
        self.log_dir = log_dir
        self.log_format = log_format.lower()  # 'csv' o 'json'
        self._log_file = None
        self._log_writer = None
        self._ensure_logger()

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._recv_thread = None
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
        frame = tk.Frame(self.root, bd=1, relief=tk.GROOVE, padx=5, pady=5)
        frame.pack(pady=8)
        if self.admin:
            buttons = [
                ("Speed +", "SPEED UP"),
                ("Speed -", "SLOW DOWN"),
                ("Turn Left", "TURN LEFT"),
                ("Turn Right", "TURN RIGHT"),
            ]
            for i, (label, cmd) in enumerate(buttons):
                b = tk.Button(frame, text=label, width=12, command=lambda c=cmd: self.send_cmd(c))
                b.grid(row=i // 2, column=i % 2, padx=4, pady=4)
                self.cmd_buttons.append(b)

        # Botón LIST USERS (útil para ambos roles)
        btn_list = tk.Button(self.root, text="List Users", command=self.list_users)
        btn_list.pack(pady=4)

        # Cerrar limpio
        self.root.protocol("WM_DELETE_WINDOW", self.close)

    # ================= Conexión =================
    def connect(self):
        try:
            self.sock.connect((self.host, self.port))
            self.status_label.config(text="Estado: Conectado (enviando HELLO)")
            self._send_line("HELLO TLP/1.0")
            self._send_line("SUBSCRIBE")
            if self.admin:
                self._send_line(f"AUTH ADMIN {self.token}")

            # Thread recepción
            self._recv_thread = threading.Thread(target=self._recv_loop, daemon=True)
            self._recv_thread.start()

            self.root.mainloop()
        except Exception as e:
            messagebox.showerror("Error de conexión", str(e))

    # ================= Recepción =================
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

    def _handle_line(self, line: str):
        # Ejemplos válidos:
        #   OK Welcome to TLP/1.0
        #   OK AUTH ADMIN
        #   ERROR AUTH bad_token
        #   TELEMETRY speed=12 battery=92 temp=25 dir=LEFT ts=1759525432
        #   USERS 3 \n USER 127.0.0.1:53231 OBSERVER 1759... \n ...
        if line.startswith("TELEMETRY"):
            data = self._parse_telemetry_line(line)
            if data:
                self._update_telemetry(data, origin="TELEMETRY")
            return

        if line.startswith("USERS "):
            # Mostramos en status el número
            try:
                n = int(line.split()[1])
                self._set_status(f"Usuarios conectados: {n}")
            except Exception:
                self._set_status(line)
            return
        if line.startswith("USER "):
            # Puedes imprimirlos en consola o mostrarlos en un popup
            print(line)
            return

        if line.startswith("OK"):
            self._set_status(line)
            return

        if line.startswith("ERROR"):
            self._set_status(line)
            return

        # Desconocido/otros
        print("[INFO]", line)

    # ================= Envío / Peticiones =================
    def _send_line(self, s: str):
        try:
            # aseguramos \n
            if not s.endswith("\n"):
                s += "\n"
            self.sock.sendall(s.encode())
        except Exception as e:
            messagebox.showerror("Error de envío", str(e))

    def send_cmd(self, cmd_text):
        if not self.admin:
            messagebox.showwarning("No autorizado", "Rol OBSERVER no puede enviar comandos")
            return
        if cmd_text not in ("SPEED UP", "SLOW DOWN", "TURN LEFT", "TURN RIGHT"):
            messagebox.showwarning("Comando inválido", cmd_text)
            return
        self._send_line(f"COMMAND {cmd_text}")

    def list_users(self):
        self._send_line("LIST USERS")

    # ================= Procesamiento TELEMETRY =================
    def _parse_telemetry_line(self, line: str):
        # Formato:
        # TELEMETRY speed=12 battery=92 temp=25 dir=LEFT ts=1759525432
        try:
            parts = line.split()
            if parts[0] != "TELEMETRY":
                return None
            kv = {}
            for p in parts[1:]:
                if "=" in p:
                    k, v = p.split("=", 1)
                    kv[k] = v
            data = {
                "speed": int(kv.get("speed", "0")),
                "battery": int(kv.get("battery", "0")),
                "temp": int(kv.get("temp", "0")),
                "dir": kv.get("dir", "--"),
                "ts": int(kv.get("ts", str(int(time.time())))),
            }
            return data
        except Exception as e:
            print("[PARSE] Línea inválida:", line, "err:", e)
            return None

    def _update_telemetry(self, msg, origin):
        # (tkinter no es 100% thread-safe; usamos after para actualizar desde el hilo RX)
        ts = msg.get('ts', int(time.time()))
        human = time.strftime('%H:%M:%S', time.localtime(ts))
        def apply():
            self.speed_label.config(text=f"Velocidad: {msg.get('speed','--')}")
            self.battery_label.config(text=f"Batería: {msg.get('battery','--')}")
            self.temp_label.config(text=f"Temperatura: {msg.get('temp','--')}")
            self.dir_label.config(text=f"Dirección: {msg.get('dir','--')}")
            self.last_update_label.config(text=f"Última actualización: {human} ({origin})")
        self.root.after(0, apply)
        self._log_telemetry(msg, origin)

    def _set_status(self, text):
        self.root.after(0, lambda: self.status_label.config(text=f"Estado: {text}"))

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
                # mantenemos jsonl para logs
                import json
                record = {"ts": epoch, "human": human, "origin": origin, "speed": speed,
                          "battery": battery, "temp": temp, "dir": direction}
                self._log_file.write(__import__('json').dumps(record, ensure_ascii=False) + '\n')
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
