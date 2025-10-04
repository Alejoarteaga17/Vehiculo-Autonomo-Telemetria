import java.io.*;
import java.net.*;

public class Cliente {
    private Socket socket;
    private BufferedReader in;
    private PrintWriter out;

    public void conectar(String host, int puerto) throws IOException {
        socket = new Socket(host, puerto);
        System.out.println("Conectado al servidor " + host + ":" + puerto);

        in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        out = new PrintWriter(socket.getOutputStream(), true);
    }

    public void enviarMensaje(String json) {
        out.println(json);
    }

    public String recibirMensaje() throws IOException {
        return in.readLine();
    }

    public BufferedReader getInputStream() {
        return in;
    }

    public void cerrar() throws IOException {
        in.close();
        out.close();
        socket.close();
    }
}