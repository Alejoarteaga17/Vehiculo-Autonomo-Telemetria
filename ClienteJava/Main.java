//Main.java
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import java.io.IOException;

public class Main {
    public static void main(String[] args) {
        String host = "127.0.0.1";
        int puerto = 8080;
        try {
            Cliente cliente = new Cliente();
            cliente.conectar(host, puerto);
            LoggerCliente.info("Conectado al servidor " + host + ":" + puerto);
            System.out.println("Conectado al servidor " + host + ":" + puerto);
            SwingUtilities.invokeLater(() -> new LoginFrame(cliente));

        } catch (IOException e) {
            LoggerCliente.error("Error de conexión al servidor: " + e.getMessage());
            SwingUtilities.invokeLater(() -> {
                JOptionPane.showMessageDialog(
                    null,
                    "No se pudo conectar al servidor en " + host + ":" + puerto +
                    "\nVerifica que el servidor este activo y accesible.",
                    "Error de Conexion",
                    JOptionPane.ERROR_MESSAGE
                );
            });

            System.err.println("Error de conexion: " + e.getMessage());
        }
    }
}