import java.io.BufferedReader;
import java.io.IOException;

public class TelemetriaListener extends Thread {
    private final BufferedReader in;
    private final UICliente ui;

    public TelemetriaListener(BufferedReader in, UICliente ui) {
        this.in = in;
        this.ui = ui;
    }

    @Override
    public void run() {
        try {
            String line;
            while ((line = in.readLine()) != null) {
                System.out.println("Servidor → " + line);
                LoggerCliente.info("Servidor → " + line);

                if (line.startsWith("TELEMETRY")) {
                    // Ej: TELEMETRY speed=12 battery=92 temp=25 dir=LEFT ts=1759525432
                    String velocidad = "--", bateria = "--", temperatura = "--", direccion = "--";

                    String[] parts = line.split("\\s+");
                    for (int i = 1; i < parts.length; i++) {
                        int eq = parts[i].indexOf('=');
                        if (eq <= 0) continue;
                        String k = parts[i].substring(0, eq);
                        String v = parts[i].substring(eq + 1);
                        switch (k) {
                            case "speed":   velocidad   = v; break;
                            case "battery": bateria     = v; break;
                            case "temp":    temperatura = v; break;
                            case "dir":     direccion   = v; break;
                            default: break;
                        }
                    }
                    ui.actualizarDatos(velocidad, bateria, temperatura, direccion);
                    continue;
                }
            }
        } catch (IOException e) {
            LoggerCliente.error("Error escuchando telemetría: " + e.getMessage());
            System.out.println("Error escuchando telemetría: " + e.getMessage());
        }
    }
}
