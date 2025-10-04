# Cliente de Telemetría para Vehículo Autónomo

Cliente Python compatible con el servidor C (JSON). Ahora permite seleccionar el **rol** (ADMIN u OBSERVER) desde la terminal antes de abrir la GUI. Si eres OBSERVER solo ves datos; si eres ADMIN además puedes enviar comandos.

## Estructura de Archivos

Estructura relevante (esta versión simplificada usa `cliente.py` y `main.py`):

- `main.py` - Punto de entrada (selección de rol y lanzamiento GUI)
- `cliente.py` - Implementación del cliente (parsing de TELEMETRY/DATA, comandos, GUI)
- `test_servidor_c.py` - Script de pruebas básicas (si mantienes versión avanzada JSON)

## Configuración

Edita el archivo `client_config_json.json` para configurar:
- IP del servidor (por defecto: "127.0.0.1")
- Puerto (por defecto: 8080)
- Token de autenticación (por defecto: "SECRETO_2025")

```json
{
    "server_ip": "127.0.0.1",
    "server_port": 8080,
    "token": "SECRETO_2025",
    "reconnect_interval": 5,
    "max_retries": 3
}
```

## Instrucciones de Ejecución

### 1. Compilar y ejecutar el servidor C

```bash
cd ../Servidor
gcc -o server server.c protocol.c
./server
```

### 2. Ejecutar el cliente Python

Desde Windows PowerShell o Linux:

```bash
cd Cliente-1
python main.py
```

Opcionalmente puedes pasar el rol directamente:

```bash
python main.py admin
python main.py observer
```

Si seleccionas admin y no introduces token, se usará `SECRETO_2025` por defecto.

### 3. Usar la interfaz

1. La interfaz se abrirá automáticamente
2. El cliente intentará conectarse al servidor
3. Una vez conectado, puedes:
   - Enviar comandos (SPEED_UP, SPEED_DOWN, BRAKE)
   - Ver telemetría en tiempo real
   - Monitorear el estado de conexión

## Comandos Disponibles (solo ADMIN)

- **SPEED_UP**: Aumentar velocidad
- **SLOW_DOWN**: Reducir velocidad
- **TURN_LEFT**: Girar izquierda
- **TURN_RIGHT**: Girar derecha

## Protocolo JSON

El cliente usa protocolo JSON compatible con el servidor C:

### HELLO
```json
{"t":"HELLO","role":"ADMIN","token":"SECRETO_2025"}
{"t":"HELLO","role":"OBSERVER"}
```

### TELEMETRY (periódico)
```json
{"t":"TELEMETRY","speed":40,"battery":98,"temp":25,"dir":"STRAIGHT","ts":1728000000}
```

### DATA (respuesta a GET_DATA)
```json
{"t":"DATA","speed":40,"battery":98,"temp":25,"dir":"STRAIGHT","ts":1728000001}
```

### Comando
```json
{"t":"CMD","name":"SPEED_UP"}
```

### Respuestas a comando
```json
{"t":"ACK","name":"SPEED_UP"}
{"t":"NACK","name":"SPEED_UP","reason":"speed-limit"}
```

### Errores
```json
{"t":"ERR","code":"not-authorized"}
{"t":"ERR","code":"bad-auth"}
{"t":"ERR","code":"bad-cmd"}
```

## Pruebas

Para probar la compatibilidad con el servidor C:

```bash
python test_servidor_c.py
```

## Requisitos

- Python 3.7+
- tkinter (incluido con Python)
- Servidor C compilado y ejecutándose

## Solución de Problemas
## Nuevas Funcionalidades

- Etiqueta "Última actualización" mostrando hora y si proviene de DATA (consulta) o TELEMETRY (broadcast periódico).
- Logging local en carpeta `logs/` (CSV por defecto, formato JSONL si cambias `log_format="json"` al crear `VehiculoClient`).
    - Archivos: `telemetria_YYYYMMDD.csv` con columnas: epoch,human,origin,speed,battery,temp,dir
    - Origen indica si vino de TELEMETRY o DATA.

Para cambiar el intervalo de sondeo (GET_DATA) ajusta `poll_interval` (por defecto 10s). El servidor envía TELEMETRY cada `TELEMETRY_INTERVAL` definido en `protocol.h` (10 por defecto). Con la modificación realizada, la batería baja gradualmente y la temperatura fluctúa para que veas cambios.


1. **No aparecen datos rápidos**: El servidor envía TELEMETRY cada 10s. El cliente manda `GET_DATA` inicial y luego cada `poll_interval` (por defecto 10s). Puedes reducirlo en `cliente.py`.
2. **Valores raros**: Asegúrate de estar usando los nombres correctos (SPEED_UP, SLOW_DOWN, TURN_LEFT, TURN_RIGHT). La versión antigua usaba espacios.
3. **Error bad-auth**: Token incorrecto para rol ADMIN.
4. **Botones deshabilitados**: Estás en modo OBSERVER (reinicia como admin).
5. **Cierre abrupto**: Si el servidor se cae, el estado pasa a "Desconectado"; reinicia el servidor y el cliente.