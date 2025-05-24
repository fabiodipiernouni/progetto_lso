package org.internationalwhispers.client.bll.server;

import javafx.application.Platform;
import org.internationalwhispers.client.bll.server.communication.SocketCommand;
import org.internationalwhispers.client.bll.server.communication.SocketReply;
import org.internationalwhispers.client.bll.server.exception.RemoteServerException;
import org.internationalwhispers.client.presentation.orchestrator.playerdomain.MainOrchestrator;

import java.io.*;
import java.net.Socket;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.logging.Level;
import java.util.logging.Logger;

public class RemoteServer {
    private static final Logger LOGGER = Logger.getLogger(RemoteServer.class.getName());

    private Socket socket;
    private PrintWriter out;
    private BufferedReader in;

    private final BlockingQueue<SocketReply> replyQueue = new LinkedBlockingQueue<>();
    private final BlockingQueue<SocketCommand> commandQueue = new LinkedBlockingQueue<>();

    private Thread listenerThread;

    public void connect(String host, int port) throws IOException {
        socket = new Socket(host, port);
        out = new PrintWriter(socket.getOutputStream(), true);
        in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        startListening();
    }

    public void startListening()  {
        listenerThread = new Thread(() -> {
            try {
                while (!Thread.currentThread().isInterrupted()) {
                    String line = in.readLine();
                    if (line == null) {
                        throw new RemoteServerException("Connection closed by server");
                    }

                    // Determine if the message is a SocketReply or a SocketCommand
                    try {
                        SocketReply reply = new SocketReply(line);
                        replyQueue.put(reply);
                        LOGGER.info("Received communication: " + line);
                    } catch (IllegalArgumentException e) {
                        SocketCommand command = new SocketCommand(line);
                        commandQueue.put(command);
                        LOGGER.info("Received command: " + line);
                    }

                }
            } catch (Exception e) {
                Thread.currentThread().interrupt();
                LOGGER.log(Level.SEVERE, "Error in listener thread", e);
                Platform.runLater(() -> MainOrchestrator.getInstance().handleFatalError("Server are momentarily not available, please try again later."));
            }
        });
        listenerThread.start();
    }

    private void send(SocketCommand message) throws RemoteServerException {
        try {
            out.println(message.toString());

        } catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Error sending communication to server", e);
            throw new RemoteServerException("Error sending communication to server");
        }
    }

    private SocketReply receiveReply() throws RemoteServerException {
        try {
            // Blocks until a message is available
            return replyQueue.take();
        } catch (InterruptedException e) {
            // Restore the interrupted status of the thread
            Thread.currentThread().interrupt();
            throw new RemoteServerException("Thread interrupted while waiting for a reply");
        }
    }

    public SocketCommand receiveCommand() throws RemoteServerException {
        try {
            // Blocks until a message is available
            return commandQueue.take();
        } catch (InterruptedException e) {
            // Restore the interrupted status of the thread
            Thread.currentThread().interrupt();
            throw new RemoteServerException("Thread interrupted while waiting for a reply");
        }
    }

    public synchronized SocketReply sendAndReceive(SocketCommand message) throws RemoteServerException {
        // the synchronized block ensures that only one thread can send and receive at a time
        
        send(message);
        return receiveReply();
    }

    public void disconnect() throws IOException {

        if (in != null) {
            in.close();
        }
        if (out != null) {
            out.close();
        }
        if (socket != null) {
            socket.close();
        }
        if (listenerThread != null) {
            listenerThread.interrupt();
        }
    }

}