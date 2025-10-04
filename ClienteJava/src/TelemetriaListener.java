import java.io.BufferedReader;
import java.io.IOException;

public class TelemetriaListener extends Thread {
    private BufferedReader in;
    private UICliente ui;

    public TelemetriaListener(BufferedReader in, UICliente ui) {
        this.in = in;
        this.ui = ui;
    }

    @Override
    public void run() {
        try {
            String mensaje;
            while ((mensaje = in.readLine()) != null) {
                System.out.println("Servidor → " + mensaje);

                if (mensaje.contains("TELEMETRY")) {
                    String velocidad = "--", bateria = "--", temperatura = "--", direccion = "--";

                    if (mensaje.contains("speed")) {
                        velocidad = mensaje.split("\"speed\":")[1].split(",")[0];
                    }
                    if (mensaje.contains("battery")) {
                        bateria = mensaje.split("\"battery\":")[1].split(",")[0];
                    }
                    if (mensaje.contains("\"temp\":")) {
                        temperatura = mensaje.split("\"temp\":")[1].split(",")[0];
                    }
                    if (mensaje.contains("dir")) {
                        direccion = mensaje.split("\"dir\":\"")[1].split("\"")[0];
                    }

                    ui.actualizarDatos(velocidad, bateria, temperatura, direccion);
                }
            }
        } catch (IOException e) {
            System.out.println("Error escuchando telemetría: " + e.getMessage());
        }
    }
}