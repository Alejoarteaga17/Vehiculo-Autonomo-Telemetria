import java.io.*;
import java.net.*;

public class Cliente {
    private Socket socket;
    private BufferedReader in;
    private PrintWriter out;

    public void conectar(String host, int puerto) throws IOException {
        socket = new Socket(host, puerto);
        System.out.println("Conectado al servidor " + host + ":" + puerto);
        in  = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
        out = new PrintWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"), true);
    }

    // ===== Envío de texto (una línea por mensaje) =====
    public void enviarLinea(String line) { out.println(line); }

    // Helpers del protocolo
    public void hello()                      { enviarLinea("HELLO TLP/1.0"); }
    public void subscribe()                  { enviarLinea("SUBSCRIBE"); }
    public void authAdmin(String token)      { enviarLinea("AUTH ADMIN " + token); }
    public void listUsers()                  { enviarLinea("LIST USERS"); }
    public void command(String cmdWithSpace) { enviarLinea("COMMAND " + cmdWithSpace); }
    // Ejemplos: command("SPEED UP"), command("SLOW DOWN"), command("TURN LEFT"), command("TURN RIGHT")

    // ===== Recepción =====
    public String recibirLinea() throws IOException { return in.readLine(); }
    public BufferedReader getInputStream() { return in; }

    public void cerrar() throws IOException {
        try { in.close(); }  catch (Exception ignore) {}
        try { out.close(); } catch (Exception ignore) {}
        try { socket.close(); } catch (Exception ignore) {}
    }
}
