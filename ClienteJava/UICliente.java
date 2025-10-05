import javax.swing.*;
import java.awt.*;
import java.time.LocalTime;
import java.time.format.DateTimeFormatter;

public class UICliente extends JFrame {
    private JLabel lblVelocidad, lblBateria, lblTemperatura, lblDireccion, lblUltimaActualizacion;
    private JButton btnSpeedUp, btnSlowDown, btnLeft, btnRight, btnListUsers;
    private final Cliente cliente;
    private final DateTimeFormatter formatoHora = DateTimeFormatter.ofPattern("HH:mm:ss");

    public UICliente(Cliente cliente, boolean esAdmin) {
        this.cliente = cliente;

        setTitle("Panel de Vehículo Autónomo");
        setSize(480, 380);
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setLocationRelativeTo(null);
        setLayout(new BorderLayout(10, 10));

        JLabel titulo = new JLabel("Monitoreo de Vehículo Autónomo", SwingConstants.CENTER);
        titulo.setFont(new Font("Arial", Font.BOLD, 18));
        add(titulo, BorderLayout.NORTH);

        JPanel panelDatos = new JPanel(new GridLayout(5, 1, 5, 5));
        lblVelocidad = new JLabel("Velocidad: -- km/h", SwingConstants.CENTER);
        lblBateria = new JLabel("Batería: -- %", SwingConstants.CENTER);
        lblTemperatura = new JLabel("Temperatura: -- °C", SwingConstants.CENTER);
        lblDireccion = new JLabel("Dirección: --", SwingConstants.CENTER);
        lblUltimaActualizacion = new JLabel("Última actualización: --", SwingConstants.CENTER);

        Font datosFont = new Font("Consolas", Font.PLAIN, 15);
        lblVelocidad.setFont(datosFont);
        lblBateria.setFont(datosFont);
        lblTemperatura.setFont(datosFont);
        lblDireccion.setFont(datosFont);
        lblUltimaActualizacion.setFont(new Font("Arial", Font.ITALIC, 13));

        panelDatos.add(lblVelocidad);
        panelDatos.add(lblBateria);
        panelDatos.add(lblTemperatura);
        panelDatos.add(lblDireccion);
        panelDatos.add(lblUltimaActualizacion);
        add(panelDatos, BorderLayout.CENTER);

        JPanel panelInferior = new JPanel(new GridLayout(esAdmin ? 2 : 1, 2, 10, 10));
        btnListUsers = new JButton("LIST USERS");
        panelInferior.add(btnListUsers);
        btnListUsers.addActionListener(e -> cliente.listUsers());

        if (esAdmin) {
            btnSpeedUp = new JButton("SPEED UP");
            btnSlowDown = new JButton("SLOW DOWN");
            btnLeft = new JButton("TURN LEFT");
            btnRight = new JButton("TURN RIGHT");

            Font f = new Font("Arial", Font.BOLD, 14);
            btnSpeedUp.setFont(f); btnSlowDown.setFont(f);
            btnLeft.setFont(f);     btnRight.setFont(f);

            panelInferior.add(btnSpeedUp);
            panelInferior.add(btnSlowDown);
            panelInferior.add(btnLeft);
            panelInferior.add(btnRight);

            btnSpeedUp.addActionListener(e -> {
                cliente.command("SPEED UP");
                LoggerCliente.info("Comando enviado: SPEED UP");
            });

            btnSlowDown.addActionListener(e -> {
                cliente.command("SLOW DOWN");
                LoggerCliente.info("Comando enviado: SLOW DOWN");
            });

            btnLeft.addActionListener(e -> {
                cliente.command("TURN LEFT");
                LoggerCliente.info("Comando enviado: TURN LEFT");
            });

            btnRight.addActionListener(e -> {
                cliente.command("TURN RIGHT");
                LoggerCliente.info("Comando enviado: TURN RIGHT");
            });
        }

        add(panelInferior, BorderLayout.SOUTH);
        setVisible(true);
    }

    public void actualizarDatos(String velocidad, String bateria, String temperatura, String direccion) {
        lblVelocidad.setText("Velocidad: " + velocidad + " km/h");
        lblBateria.setText("Batería: " + bateria + " %");
        lblTemperatura.setText("Temperatura: " + temperatura + " °C");
        lblDireccion.setText("Dirección: " + direccion);
        lblUltimaActualizacion.setText("Última actualización: " + LocalTime.now().format(formatoHora));
    }
}
