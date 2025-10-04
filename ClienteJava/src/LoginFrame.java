import javax.swing.*;
import java.awt.*;

public class LoginFrame extends JFrame {
    private Cliente cliente;

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
            cliente.enviarMensaje("{\"t\":\"HELLO\",\"role\":\"OBSERVER\"}");
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
            this,
            passwordField,
            "Introduce la contraseña del administrador",
            JOptionPane.OK_CANCEL_OPTION
        );

        if (option == JOptionPane.OK_OPTION) {
            String password = new String(passwordField.getPassword());
            if (password.equals("SECRETO_2025")) {
                cliente.enviarMensaje("{\"t\":\"HELLO\",\"role\":\"ADMIN\",\"token\":\"SECRETO_2025\"}");
                UICliente ui = new UICliente(cliente, true);
                new TelemetriaListener(cliente.getInputStream(), ui).start();
                dispose();
            } else {
                JOptionPane.showMessageDialog(this, "Contraseña incorrecta. Intenta nuevamente.", "Error", JOptionPane.ERROR_MESSAGE);
                pedirContrasena();
            }
        }
    }
}