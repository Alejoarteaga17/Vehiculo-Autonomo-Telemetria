public class Main {
    public static void main(String[] args) {
        try {
            Cliente cliente = new Cliente();
            cliente.conectar("172.26.208.26", 8080); // Cambia puerto según tu servidor C

            new LoginFrame(cliente);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}