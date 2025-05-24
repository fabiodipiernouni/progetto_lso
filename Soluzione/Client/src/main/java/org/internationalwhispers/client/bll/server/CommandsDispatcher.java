package org.internationalwhispers.client.bll.server;

import javafx.concurrent.Task;
import org.internationalwhispers.client.bll.exception.ResourceNotFoundException;
import org.internationalwhispers.client.bll.exception.ServiceException;
import org.internationalwhispers.client.bll.gamedomain.GameService;
import org.internationalwhispers.client.bll.server.communication.SocketCommand;
import org.internationalwhispers.client.bll.server.exception.RemoteServerException;
import org.internationalwhispers.client.entity.RoomDTO;
import org.internationalwhispers.client.presentation.orchestrator.gamedomain.GameOrchestrator;
import org.json.JSONException;

import java.util.logging.Level;
import java.util.logging.Logger;

public class CommandsDispatcher {
    private final static Logger LOGGER = Logger.getLogger(CommandsDispatcher.class.getName());
    private final Thread dispatcherThread;
    private static CommandsDispatcher instance = null;

    private RemoteServer gameRulesMgr = null;

    private CommandsDispatcher(RemoteServer gameRulesMgr) {
        this.gameRulesMgr = gameRulesMgr;

        // Initialize the dispatcher thread
        dispatcherThread = new Thread(() -> {

            while (!Thread.currentThread().isInterrupted()) {
                try {
                    SocketCommand command = gameRulesMgr.receiveCommand();
                    dispatchCommand(command);
                } catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error receiving command from server", e);
                }
            }

        });
        dispatcherThread.start();
    }

    public static CommandsDispatcher getInstance(RemoteServer gameRulesMgr) {
        if (instance == null) {
            instance = new CommandsDispatcher(gameRulesMgr);
        }
        return instance;
    }

    public void dispatchCommand(SocketCommand command) {
        // Implement the logic to dispatch the command to the appropriate handler
        // This could involve checking the command type and calling the corresponding method
        LOGGER.log(Level.INFO, "Dispatching command: " + command.getMethod());

        switch(command.getMethod()) {
            case "refresh        ":
                try {
                    int roomId = command.getPayload().getInt("id_stanza");
                    Task<RoomDTO> getRoomTask = GameService.getInstance().getRoom(roomId);
                    getRoomTask.setOnSucceeded(event -> {
                        RoomDTO room = getRoomTask.getValue();
                        GameOrchestrator.getInstance().updateRoom(room);
                    });
                    getRoomTask.setOnFailed(event -> {
                        Throwable exception = getRoomTask.getException();
                        if(exception instanceof ResourceNotFoundException) {
                            LOGGER.log(Level.INFO, "Room not found", exception);
                            GameOrchestrator.getInstance().removeRoom(roomId);
                        }
                        else {
                            LOGGER.log(Level.SEVERE, "Error refreshing room", exception);
                        }
                    });
                    getRoomTask.run();
                }
                catch (JSONException e) {
                    LOGGER.log(Level.SEVERE, "Error parsing room ID", e);
                }
                catch (ServiceException e) {
                    LOGGER.log(Level.SEVERE, "Error getting room", e);
                }
        }

    }

}
