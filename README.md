# Cliente Vehículo Autónomo - Telemetría

Cliente en Python y Java para el sistema de telemetría de vehículo autónomo. **Compatible con servidor C usando protocolo JSON**. Permite monitorear datos en tiempo real y controlar el vehículo (modo administrador).
Repositorio asociado al proyecto de la clase de Internet, Arquitectura y Protocolos **#0987**

### Equipo de trabajo:
- Alejandra Suarez.
- Paula Llanos.
- Alejandro Arteaga.

El link del video [aquí](https://youtu.be/yQpsJ_7y2bY?si=PgxiX9djD8OG34El).
## ¿Cómo correr el proyecto?

### Requisitos
- Python 3.7+
- tkinter (incluido con Python)
- Servidor de telemetría ejecutándose
- Linux para iniciar el servidor

## **Clonar el repositorio:**
   ```bash
   git clone https://github.com/Alejoarteaga17/Vehiculo-Autonomo-Telemetria
   ```
- Luego ingresar a la carpeta del repositorio clonado y sigue los pasos para hacer la ejecución del cliente-servidor

## **Quick Start con Servidor C**

### **Para usar con el servidor C real:**

1. **Compilar servidor C(en linux CLI) :**
   ```bash
   cd Servidor
   make
   ```

2. **Ejecutar servidor C:**
   ```bash
   make run
   ```

3. **Ejecutar cliente Python compatible:**
   ```bash
   cd ClientePython
   python Main.py
   ```
   
4. **Ejecutar cliente Python:**
   - **Compilar archivos Java:**
    ```bash
   cd ClientePython
   javac *.java
   ```

   - **Ejecutar Main Java:**
    ```bash
     java Main
   ```

### **Token de Administrador:**
- **Token:** `SECRETO_2025`
- Configurable en `client_config_json.json`

### Funcionalidades Principales
- **Modo Administrador**: Control completo del vehículo sobre la direccion y la velocidad + visualización de telemetría
- **Modo Observador**: Solo visualización de telemetría
- **Interfaz gráfica intuitiva** con indicadores de estado
- **Validación de datos** de telemetría
- **Información actualizada** en la GUI se actualiza el tiempo de transmisión de datos
- **Almacenar Registros** mediante logs almacenamos los registros de cambios en el servidor y en los clientes

### Comandos (Solo Administrador)
- `SPEED UP` - Acelerar vehículo
- `SLOW DOWN` - Reducir velocidad
- `TURN LEFT` - Girar a la izquierda
- `TURN RIGHT` - Girar a la derecha
