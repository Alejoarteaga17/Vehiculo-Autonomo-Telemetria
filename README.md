# Cliente Vehículo Autónomo - Telemetría

Cliente en Python y Java para el sistema de telemetría de vehículo autónomo. **Compatible con servidor C usando protocolo JSON**. Permite monitorear datos en tiempo real y controlar el vehículo (modo administrador).
Repositorio asociado al proyecto de la clase de Internet, Arquitectura y Protocolos **#0987**

### Equipo de trabajo:
- Alejandra Suarez.
- Paula Llanos.
- Alejandro Arteaga.
  
El link del video [aquí](https://youtu.be/yQpsJ_7y2bY?si=PgxiX9djD8OG34El).

### Requisitos
- Python 3.7+
- tkinter (incluido con Python)
- Servidor de telemetría ejecutándose
- Linux para iniciar el servidor

## **Quick Start con Servidor C**

### **Para usar con el servidor C real:**

1. **Compilar servidor C(en linux CLI):**
   ```bash
   cd Servidor
   gcc -o server server.c -lpthread
   ```

2. **Ejecutar servidor C:**
   ```bash
   ./server 5000 server.log
   ```

3. **Ejecutar cliente Python compatible:**
   ```bash
   cd Cliente-1
   python main_json.py
   ```

### **Token de Administrador:**
- **Token:** `SECRETO_2025`
- Configurable en `client_config_json.json`

### Funcionalidades Principales
- **Modo Administrador**: Control completo del vehículo + visualización de telemetría
- **Modo Observador**: Solo visualización de telemetría
- **Interfaz gráfica intuitiva** con indicadores de estado
- **Reconexión automática** en caso de pérdida de conexión
- **Configuración externa** mediante archivo JSON
- **Validación de datos** de telemetría
- **Información actualizada** en la GUI se actualiza el tiempo de transmisión de datos

### Comandos (Solo Administrador)
- `SPEED UP` - Acelerar vehículo
- `SLOW DOWN` - Reducir velocidad
- `TURN LEFT` - Girar a la izquierda
- `TURN RIGHT` - Girar a la derecha

### Datos de Telemetría
Formato: `DATA SPEED=30 BATTERY=80 TEMP=27`
- **SPEED**: Velocidad en km/h
- **BATTERY**: Nivel de batería (0-100%)
- **TEMP**: Temperatura en °C


