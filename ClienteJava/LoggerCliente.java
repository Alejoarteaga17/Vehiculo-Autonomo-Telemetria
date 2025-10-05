import java.io.IOException;
import java.util.logging.*;

public class LoggerCliente {
    private static Logger logger = Logger.getLogger("ClienteJava");

    static {
        try {
            // Crear carpeta logs si no existe
            java.nio.file.Files.createDirectories(java.nio.file.Paths.get("logs"));

            // Configurar archivo de logs
            FileHandler fh = new FileHandler("logs/cliente_java.log", true); // true = append
            fh.setFormatter(new SimpleFormatter());

            logger.addHandler(fh);
            logger.setUseParentHandlers(true); // También muestra en consola
        } catch (IOException e) {
            System.err.println("No se pudo inicializar el logger: " + e.getMessage());
        }
    }

    public static void info(String msg) {
        logger.info(msg);
    }

    public static void error(String msg) {
        logger.severe(msg);
    }
}