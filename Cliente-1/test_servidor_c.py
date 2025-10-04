# test_servidor_c.py
"""
Script de testing específico para el servidor C.
Prueba la compatibilidad del cliente Python con el servidor C real.
"""

import subprocess
import time
import sys
import json
from pathlib import Path
from protocolo_json import *

def test_protocolo_json():
    """Prueba las funciones del protocolo JSON"""
    print("🧪 Probando protocolo JSON...")
    
    try:
        # Probar construcción de mensajes
        hello_admin = build_hello_admin()
        hello_observer = build_hello_observer()
        cmd_speed = build_command("SPEED UP")
        
        print(f"✅ HELLO Admin: {hello_admin.strip()}")
        print(f"✅ HELLO Observer: {hello_observer.strip()}")
        print(f"✅ Comando SPEED_UP: {cmd_speed.strip()}")
        
        # Probar parsing
        telemetry_msg = '{"t":"TELEMETRY","speed":45,"battery":75,"temp":28,"dir":"LEFT","ts":1727901234}'
        parsed = parse_json_message(telemetry_msg)
        telemetry = parse_telemetry(parsed)
        print(f"✅ Telemetría parseada: {telemetry}")
        
        return True
        
    except Exception as e:
        print(f"❌ Error en protocolo JSON: {e}")
        return False

def test_formato_servidor_c():
    """Verifica que los mensajes coincidan con el formato esperado por el servidor C"""
    print("\n🔍 Verificando formato del servidor C...")
    
    # Mensajes esperados según server.c
    expected_formats = {
        "HELLO_ADMIN": {"t": "HELLO", "role": "ADMIN", "token": "SECRETO_2025"},
        "HELLO_OBSERVER": {"t": "HELLO", "role": "OBSERVER"},
        "CMD_SPEED_UP": {"t": "CMD", "name": "SPEED_UP"},
        "GET_DATA": {"t": "GET_DATA"},
        "PING": {"t": "PING"}
    }
    
    # Generar y verificar mensajes
    generated_messages = {
        "HELLO_ADMIN": json.loads(build_hello_admin().strip()),
        "HELLO_OBSERVER": json.loads(build_hello_observer().strip()),
        "CMD_SPEED_UP": json.loads(build_command("SPEED UP").strip()),
        "GET_DATA": json.loads(build_get_data().strip()),
        "PING": json.loads(build_ping().strip())
    }
    
    all_ok = True
    for msg_type, expected in expected_formats.items():
        generated = generated_messages[msg_type]
        
        if generated == expected:
            print(f"✅ {msg_type}: Formato correcto")
        else:
            print(f"❌ {msg_type}: Formato incorrecto")
            print(f"   Esperado: {expected}")
            print(f"   Generado: {generated}")
            all_ok = False
    
    return all_ok

def verificar_servidor_c():
    """Verifica si hay un servidor C ejecutándose"""
    print("\n🔍 Verificando si el servidor C está ejecutándose...")
    
    try:
        import socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(2)
        result = sock.connect_ex(('127.0.0.1', 5000))
        sock.close()
        
        if result == 0:
            print("✅ Servidor detectado en puerto 5000")
            return True
        else:
            print("❌ No se detectó servidor en puerto 5000")
            return False
            
    except Exception as e:
        print(f"❌ Error verificando servidor: {e}")
        return False

def test_cliente_json():
    """Prueba básica del cliente JSON"""
    print("\n🧪 Probando cliente JSON...")
    
    try:
        from cliente_json import VehiculoClientJSON
        
        # Crear cliente de prueba
        client = VehiculoClientJSON(admin=True)
        
        # Verificar que se creó correctamente
        assert client.admin == True
        assert client.token == "SECRETO_2025"
        assert client.host == "127.0.0.1"
        assert client.port == 5000
        
        print("✅ Cliente JSON creado exitosamente")
        
        # Test de callback
        messages_received = []
        def test_callback(msg):
            messages_received.append(msg)
        
        client.add_listener(test_callback)
        
        # Simular notificación
        client._notify_listeners("TEST MESSAGE")
        
        if "TEST MESSAGE" in messages_received:
            print("✅ Sistema de callbacks funcionando")
        else:
            print("❌ Error en sistema de callbacks")
            return False
        
        return True
        
    except Exception as e:
        print(f"❌ Error probando cliente JSON: {e}")
        return False

def mostrar_instrucciones_servidor_c():
    """Muestra instrucciones para compilar y ejecutar el servidor C"""
    print("\n📋 INSTRUCCIONES PARA SERVIDOR C:")
    print("=" * 50)
    print()
    print("1. Compilar el servidor C:")
    print("   cd ../Servidor")
    print("   gcc -o server server.c -lpthread")
    print()
    print("2. Ejecutar el servidor:")
    print("   ./server 5000 server.log")
    print()
    print("3. El servidor estará escuchando en puerto 5000")
    print("4. Los logs se guardarán en server.log")
    print()
    print("🔑 Token de administrador: SECRETO_2025")
    print()

def main():
    """Función principal del testing"""
    print("🚗" + "="*60 + "🚗")
    print("🚗      TESTING COMPATIBILIDAD SERVIDOR C              🚗")
    print("🚗" + "="*60 + "🚗")
    print()
    
    # Test 1: Protocolo JSON
    success1 = test_protocolo_json()
    
    # Test 2: Formato servidor C
    success2 = test_formato_servidor_c()
    
    # Test 3: Cliente JSON
    success3 = test_cliente_json()
    
    # Test 4: Verificar servidor
    servidor_running = verificar_servidor_c()
    
    print("\n📊 RESUMEN DE TESTS:")
    print("=" * 30)
    print(f"Protocolo JSON:      {'✅' if success1 else '❌'}")
    print(f"Formato servidor C:  {'✅' if success2 else '❌'}")
    print(f"Cliente JSON:        {'✅' if success3 else '❌'}")
    print(f"Servidor detectado:  {'✅' if servidor_running else '❌'}")
    
    if success1 and success2 and success3:
        print("\n🎉 ¡Todos los tests básicos pasaron!")
        
        if servidor_running:
            print("\n🚀 ¡Listo para probar con servidor C!")
            print("Ejecuta: python main_json.py")
        else:
            print("\n⚠️  Para prueba completa, inicia el servidor C:")
            mostrar_instrucciones_servidor_c()
    else:
        print("\n❌ Algunos tests fallaron. Revisar código.")
    
    print("\n🔧 ARCHIVOS PARA SERVIDOR C:")
    print("- main_json.py          (cliente compatible)")
    print("- cliente_json.py       (implementación JSON)")
    print("- protocolo_json.py     (protocolo JSON)")
    print("- client_config_json.json (configuración)")

if __name__ == "__main__":
    main()