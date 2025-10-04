import javax.swing.*;
import java.awt.*;

public class LoginFrame extends JFrame {
    private final Cliente cliente;

    public LoginFrame(Cliente cliente) {
        this.cliente = cliente;
        setTitle("Conexión Vehículo Autónomo");
        setSize(400, 200);
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setLocationRelativeTo(null);
        setLayout(new GridLayout(3, 1));

        JLabel lblPregunta = new JLabel("¿Deseas ingresar como administrador?", SwingConstants.CENTER);
        JButton btnAdmin = new JButton("Administrador");
        JButton btnObserver = new JButton("Observador");

        add(lblPregunta);
        add(btnAdmin);
        add(btnObserver);

        btnObserver.addActionListener(e -> {
            // Protocolo de texto
            cliente.hello();
            cliente.subscribe();
            UICliente ui = new UICliente(cliente, false);
            new TelemetriaListener(cliente.getInputStream(), ui).start();
            dispose();
        });

        btnAdmin.addActionListener(e -> pedirContrasena());

        setVisible(true);
    }

    private void pedirContrasena() {
        JPasswordField passwordField = new JPasswordField();
        int option = JOptionPane.showConfirmDialog(
            this, passwordField, "Introduce la contraseña del administrador",
            JOptionPane.OK_CANCEL_OPTION
        );

        if (option == JOptionPane.OK_OPTION) {
            String token = new String(passwordField.getPassword());
            if (token.isEmpty()) token = "SECRETO_2025"; // sugerencia por defecto
            // Handshake en texto
            cliente.hello();
            cliente.subscribe();
            cliente.authAdmin(token);

            UICliente ui = new UICliente(cliente, true);
            new TelemetriaListener(cliente.getInputStream(), ui).start();
            dispose();
        }
    }
}
