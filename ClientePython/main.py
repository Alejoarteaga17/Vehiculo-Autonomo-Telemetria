import sys
from cliente import VehiculoClient

def seleccionar_rol_desde_args():
    if len(sys.argv) >= 2:
        role = sys.argv[1].strip().lower()
        if role in ("admin", "observer"):
            return role
    # pedir al usuario
    while True:
        r = input("Selecciona rol [admin/observer]: ").strip().lower()
        if r in ("admin", "observer"):
            return r
        print("Entrada inválida. Escribe 'admin' o 'observer'.")

def solicitar_token():
    t = input("Ingresa token admin (Enter para usar SECRETO_2025): ").strip()
    return t or "SECRETO_2025"

def main():
    role = seleccionar_rol_desde_args()
    admin = (role == "admin")
    token = solicitar_token() if admin else None
    print(f"Iniciando cliente como {role.upper()}...")
    cliente = VehiculoClient(admin=admin, token=token)
    cliente.connect()

if __name__ == "__main__":
    main()
