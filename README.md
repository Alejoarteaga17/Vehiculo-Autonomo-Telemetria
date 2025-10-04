# Cliente Vehículo Autónomo - Telemetría

Cliente en Python para el sistema de telemetría de vehículo autónomo. **Compatible con servidor C usando protocolo JSON**. Permite monitorear datos en tiempo real y controlar el vehículo (modo administrador).

## 🚨 **IMPORTANTE: Compatibilidad con Servidor C**

Este proyecto ahora incluye **dos versiones del cliente**:

### **1. Cliente Original (Protocolo Texto)**
- Para testing y desarrollo sin servidor
- Protocolo simple: `CMD SPEED UP`, `DATA SPEED=30`
- Compatible con servidor simulado Python

### **2. Cliente JSON (Compatible con Servidor C)**
- **Compatible con el servidor C real**
- Protocolo JSON: `{"t":"CMD","name":"SPEED_UP"}`
- Autenticación con token: `SECRETO_2025`

## 🚀 **Quick Start con Servidor C**

### **Para usar con el servidor C real:**

1. **Compilar servidor C:**
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

## Características

### Funcionalidades Principales
- **Modo Administrador**: Control completo del vehículo + visualización de telemetría
- **Modo Observador**: Solo visualización de telemetría
- **Interfaz gráfica intuitiva** con indicadores de estado
- **Reconexión automática** en caso de pérdida de conexión
- **Sistema de logging** completo para debugging
- **Configuración externa** mediante archivo JSON
- **Validación de datos** de telemetría
- **Manejo robusto de errores**

### Mejoras Implementadas
- ✅ Manejo de errores y excepciones robusto
- ✅ Reconexión automática con reintentos configurables
- ✅ Sistema de logging para debugging y monitoreo
- ✅ Validación de datos de telemetría
- ✅ Interfaz gráfica mejorada con indicadores de estado
- ✅ Configuración externa mediante JSON
- ✅ Separación clara entre cliente admin y observador
- ✅ Protocolos de comunicación validados

## Estructura del Proyecto

```
Cliente-1/
├── main.py                 # Punto de entrada principal
├── cliente.py              # Lógica principal del cliente
├── interfaz.py             # Interfaz gráfica (tkinter)
├── protocolo.py            # Protocolo de comunicación
├── config.py               # Manejo de configuración
├── client_config.json      # Archivo de configuración
├── cliente_observador.py   # Script para modo observador
└── client.log             # Archivo de logs (generado automáticamente)
```

## Testing y Desarrollo

### 🎭 Opciones de Testing Disponibles

¡Ahora puedes probar TODA la funcionalidad sin necesidad de servidor!

#### 1. **Modo Demo** (Recomendado para empezar)
```bash
python main_demo.py
```
- ✅ Funciona sin servidor
- ✅ Telemetría realista en tiempo real
- ✅ Simulación completa de comandos
- ✅ Cambio automático de escenarios
- ✅ Perfecto para demos y presentaciones

#### 2. **Servidor Simulado Completo**
```bash
python servidor_simulado.py
```
- ✅ Servidor completo que simula el real
- ✅ Maneja múltiples clientes
- ✅ Autenticación real
- ✅ Telemetría realista
- 🔑 Credenciales: `admin/1234`, `test/test123`, `demo/demo`

#### 3. **Suite de Testing Completa**
```bash
python test_completo.py
```
- ✅ Menú interactivo con todas las opciones
- ✅ Tests automáticos
- ✅ Ayuda detallada
- ✅ Verificación de todas las funcionalidades

#### 4. **Generador de Datos Solo**
```bash
python generador_datos.py
```
- ✅ Prueba generación de telemetría
- ✅ Diferentes escenarios de simulación
- ✅ Datos realistas y variables

### 🚀 Quick Start para Testing

1. **Para testing con servidor C REAL**:
   ```bash
   # Terminal 1: Servidor C
   cd Servidor
   gcc -o server server.c -lpthread
   ./server 5000 server.log
   
   # Terminal 2: Cliente Python
   cd Cliente-1
   python main_json.py
   ```

2. **Para demo rápida (sin servidor)**:
   ```bash
   python main_demo.py
   ```

3. **Para testing con servidor simulado Python**:
   ```bash
   # Terminal 1: Servidor simulado
   python servidor_simulado.py
   
   # Terminal 2: Cliente original
   python main.py
   ```

4. **Para testing automático**:
   ```bash
   python test_completo.py
   ```

5. **Para testing específico servidor C**:
   ```bash
   python test_servidor_c.py
   ```

### 📊 Escenarios de Simulación

El sistema incluye diferentes escenarios realistas:

- **🏙️ CIUDAD**: Velocidades variables, paradas frecuentes
- **🛣️ AUTOPISTA**: Velocidades altas y constantes
- **🔋 BATERÍA BAJA**: Modo ahorro de energía
- **🌡️ SOBRECALENTAMIENTO**: Temperaturas elevadas
- **⚡ CONDUCCIÓN AGRESIVA**: Cambios bruscos, alto consumo
- **🍃 ECO MODE**: Eficiencia energética máxima

### 🔧 Configuración para Testing

En `client_config.json` puedes ajustar:
```json
{
    "server": {
        "host": "127.0.0.1",    // Para servidor local
        "port": 5000
    },
    "client": {
        "mode": "admin",        // "admin" o "observer"
        "auto_connect": true    // Conexión automática
    }
}
```

## Instalación y Uso

### Requisitos
- Python 3.7+
- tkinter (incluido con Python)
- Servidor de telemetría ejecutándose

### Configuración

1. **Editar configuración** (opcional):
   Modifica `client_config.json` para ajustar:
   ```json
   {
       "server": {
           "host": "127.0.0.1",
           "port": 5000,
           "timeout": 10,
           "max_retries": 3,
           "reconnect_delay": 5
       },
       "admin": {
           "username": "admin",
           "password": "1234"
       },
       "client": {
           "mode": "admin",        // "admin" o "observer"
           "auto_connect": false,
           "log_level": "INFO"
       }
   }
   ```

### Ejecución

1. **Modo Administrador** (control + telemetría):
   ```bash
   python main.py
   ```

2. **Modo Observador** (solo telemetría):
   ```bash
   python cliente_observador.py
   ```

## Protocolo de Comunicación

### Comandos (Solo Administrador)
- `SPEED UP` - Acelerar vehículo
- `SLOW DOWN` - Reducir velocidad
- `TURN LEFT` - Girar a la izquierda
- `TURN RIGHT` - Girar a la derecha
- `STOP` - Parada de emergencia
- `STATUS` - Solicitar estado actual

### Datos de Telemetría
Formato: `DATA SPEED=30 BATTERY=80 TEMP=27`
- **SPEED**: Velocidad en km/h
- **BATTERY**: Nivel de batería (0-100%)
- **TEMP**: Temperatura en °C

### Autenticación (Administrador)
Formato: `AUTH username password`

## Características de la Interfaz

### Indicadores de Estado
- 🔴 **Conexión**: Rojo (desconectado) / Verde (conectado)
- 🔐 **Autenticación**: Estado de autenticación para admins
- 📊 **Telemetría**: Velocidad, batería y temperatura en tiempo real

### Controles de Administrador
- Botones de control de velocidad y dirección
- Botón de parada de emergencia
- Solicitud de estado del vehículo

### Registro de Mensajes
- Log en tiempo real de todas las comunicaciones
- Mensajes de error y estado
- Historial de comandos enviados

## Manejo de Errores

### Errores de Conexión
- Reintentos automáticos configurables
- Notificación visual del estado de conexión
- Logging detallado de errores de red

### Errores de Datos
- Validación de rangos de telemetría
- Manejo de mensajes malformados
- Filtrado de datos inválidos

### Errores de Autenticación
- Validación de credenciales
- Indicadores de estado de autenticación
- Manejo de fallos de autenticación

## Logging

Los logs se guardan en:
- **Archivo**: `client.log`
- **Consola**: Salida estándar
- **Niveles**: DEBUG, INFO, WARNING, ERROR, CRITICAL

## Personalización

### Cambiar Modo de Operación
En `client_config.json`:
```json
{
    "client": {
        "mode": "observer"  // Cambiar a "admin" para modo administrador
    }
}
```

### Configurar Servidor
```json
{
    "server": {
        "host": "192.168.1.100",  // IP del servidor
        "port": 8080              // Puerto del servidor
    }
}
```

### Ajustar Credenciales
```json
{
    "admin": {
        "username": "tu_usuario",
        "password": "tu_contraseña"
    }
}
```

## Solución de Problemas

### Error de Conexión
1. Verificar que el servidor esté ejecutándose
2. Comprobar host y puerto en la configuración
3. Revisar firewall y conectividad de red

### Error de Autenticación
1. Verificar credenciales en `client_config.json`
2. Confirmar con el administrador del servidor
3. Revisar logs para detalles específicos

### Problemas de Interfaz
1. Verificar que tkinter esté instalado
2. Comprobar resolución de pantalla
3. Revisar logs de la aplicación

## Desarrollo y Contribución

### Estructura del Código
- **cliente.py**: Maneja la comunicación con el servidor
- **interfaz.py**: Interfaz gráfica de usuario
- **protocolo.py**: Definición del protocolo de comunicación
- **config.py**: Sistema de configuración

### Testing
Para probar sin servidor:
1. Ejecutar en modo observador
2. Revisar logs de conexión
3. Verificar manejo de errores

## Notas Importantes

- ⚠️ **Solo un administrador** puede controlar el vehículo simultáneamente
- 🔄 **Reconexión automática** activa por defecto
- 📝 **Logs detallados** para debugging y auditoría
- 🔒 **Validación de datos** para prevenir comandos inválidos

## Próximas Mejoras

- [ ] Cifrado de comunicaciones
- [ ] Interfaz web alternativa
- [ ] Grabación y reproducción de sesiones
- [ ] Alertas de telemetría crítica
- [ ] Métricas de rendimiento

---
*Desarrollado para el proyecto de Vehículo Autónomo - Telemetría*
Repositorio asociado al proyecto de telemetría de la clase de Internet, Arquitectura y Protocolos **#0987**
### Equipo de trabajo:
- Alejandra Suarez.
- Paula Llanos.
- Alejandro Arteaga.
  
El link del video [aquí]().
