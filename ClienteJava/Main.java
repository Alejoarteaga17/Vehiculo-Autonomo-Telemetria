//Main.java
public class Main {
    public static void main(String[] args) {
        try {
            Cliente cliente = new Cliente();
            cliente.conectar("127.0.0.1", 8080); // Cambia puerto según tu servidor C

            new LoginFrame(cliente);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}