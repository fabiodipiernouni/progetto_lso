package org.internationalwhispers.client.presentation.orchestrator.gamedomain;

import javafx.application.Platform;
import javafx.collections.ObservableList;
import javafx.concurrent.Task;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.stage.Stage;
import org.internationalwhispers.client.bll.exception.ServiceException;
import org.internationalwhispers.client.bll.gamedomain.GameService;
import org.internationalwhispers.client.bll.gamedomain.TranslatorService;
import org.internationalwhispers.client.entity.RoomDTO;
import org.internationalwhispers.client.presentation.controller.gamedomain.GameController;
import org.internationalwhispers.client.presentation.exception.TranslationException;
import org.internationalwhispers.client.presentation.helper.ResourceLoader;
import org.internationalwhispers.client.presentation.helper.Session;
import org.internationalwhispers.client.presentation.orchestrator.playerdomain.MainOrchestrator;

import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

public class GameOrchestrator {
    private static final Logger LOGGER = Logger.getLogger(GameOrchestrator.class.getName());
    public static final int MAX_PLAYERS = 10;
    public static final int MIN_PLAYERS = 4;

    private static GameOrchestrator instance = null;
    private GameController gameController = null;

    private GameOrchestrator() {
    }

    public static GameOrchestrator getInstance() {
        if (instance == null) {
            instance = new GameOrchestrator();
        }
        return instance;
    }

    private void showPage(String domain, String page) {
        gameController.loadView(domain, page);
    }

    public void showErrorMessage(String error) {
        gameController.showErrorMessage(error);
    }

    public void showErrorMessageAndExit(String error) {
        gameController.showErrorMessage(error, event -> Platform.exit());
    }

    public void startGame() {
        try {

            FXMLLoader fxmlLoader = new FXMLLoader(ResourceLoader.loadURL("/presentation/views/gamedomain/game-view.fxml"));

            Stage stage = MainOrchestrator.getInstance().getStage();

            // Save current stage dimensions and position
            double width = stage.getWidth();
            double height = stage.getHeight();
            double x = stage.getX();
            double y = stage.getY();

            // Create or load your new scene
            Scene scene = new Scene(fxmlLoader.load());

            // Set the scene
            stage.setScene(scene);

            // Restore original size and position
            stage.setWidth(width);
            stage.setHeight(height);
            stage.setX(x);
            stage.setY(y);

            stage.show();
            this.gameController = fxmlLoader.getController();
            pickRoomClicked();
        } catch (IOException e) {
            LOGGER.log(Level.SEVERE, "Error loading game view", e);
            showErrorMessage("An internal error happened when trying to start the game");
        }
    }

    public void logoutButtonClicked() {
        try {
            MainOrchestrator.getInstance().onDisconnectRequest();
        } catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Error logging out", e);
            showErrorMessage("Error logging out: " + e.getMessage());
        }
    }

    public void createRoomClicked() {
        showPage("gamedomain", "createroom");
    }

    public void pickRoomClicked() {
        showPage("gamedomain", "pickroom");
    }

    public void joinRoomClicked(RoomDTO room) {

        try {
            Session session = Session.getInstance();

            Task<Void> joinRoomTask = GameService.getInstance().joinRoom(room);
            joinRoomTask.setOnSucceeded(event -> {
                session.getRoomProperty().setValue(room);
                gameController.loadView("gamedomain", "gametable");
            });
            joinRoomTask.setOnFailed(event -> {
                Throwable e = joinRoomTask.getException();
                LOGGER.log(Level.SEVERE, "Error joining room", e);
                showErrorMessage("Error joining room: " + room.getName() + ": " + e.getMessage());
            });
            MainOrchestrator.runTask(joinRoomTask);
        } catch (ServiceException e) {
            LOGGER.log(Level.SEVERE, "Error joining room", e);
            showErrorMessage("An internal error happened when trying to join room: " + room.getName() + ": " + e.getMessage());
        }

    }

    public void createRoomRequest(String name, String direction) {
        try {
            Session session = Session.getInstance();
            Task<RoomDTO> createRoomTask = GameService.getInstance().createRoom(name, direction);
            createRoomTask.setOnSucceeded(event -> {
                RoomDTO room = createRoomTask.getValue();
                session.getRoomProperty().setValue(room);
                gameController.loadView("gamedomain","gametable");
            });
            createRoomTask.setOnFailed(event -> {
                Throwable e = createRoomTask.getException();
                LOGGER.log(Level.SEVERE, "Error creating room", e);
                showErrorMessage("Error creating room: " + e.getMessage());
            });
            MainOrchestrator.runTask(createRoomTask);
        } catch (ServiceException e) {
            LOGGER.log(Level.SEVERE, "Error creating room", e);
            showErrorMessage("An internal error happened when trying to create room: " + e.getMessage());
        }

    }

    public void leaveRoomClicked() {
        try {
            Session session = Session.getInstance();
            Task<Void> leaveRoomTask = GameService.getInstance().leaveRoom(session.getRoomProperty().getValue());
            leaveRoomTask.setOnSucceeded(event -> {
                session.getRoomProperty().setValue(null);
                gameController.loadView("gamedomain", "pickroom");
            });
            leaveRoomTask.setOnFailed(event -> {
                Throwable e = leaveRoomTask.getException();
                LOGGER.log(Level.SEVERE, "Error leaving room", e);
                showErrorMessage("Error leaving room: " + e.getMessage());
            });
            MainOrchestrator.runTask(leaveRoomTask);
        } catch (ServiceException e) {
            LOGGER.log(Level.SEVERE, "Error leaving room", e);
            showErrorMessage("An internal error happened when trying to leave room: " + e.getMessage());
        }

    }

    private void sendMessage(String message) {
        try {
            Task<Void> sendMessageTask = GameService.getInstance().sendMessage(message);
            sendMessageTask.setOnFailed(sendEvent -> {
                Throwable e = sendMessageTask.getException();
                LOGGER.log(Level.SEVERE, "Error sending communication", e);
                showErrorMessage("Error sending the message: "+e.getMessage());
            });
            MainOrchestrator.runTask(sendMessageTask);
        } catch (ServiceException e) {
            LOGGER.log(Level.SEVERE, "Error sending communication", e);
            showErrorMessage("An internal error happened when trying to send the message: " + e.getMessage());
        }
    }

    public void sendMessageClicked(String message) {

        Session session = Session.getInstance();
        Map<String, String> dictionary = session.getDictionary();
        if(dictionary.containsValue(message)) {
            for (Map.Entry<String, String> entry : dictionary.entrySet()) {
                if (entry.getValue().equals(message)) {
                    sendMessage(entry.getKey());
                    return;
                }
            }
        }

        try {
            Task<String> translateTask = getTranslateTask(
                    message,
                    session.getUserProperty().getValue().getLanguage().getLanguageCode(),
                    "EN-GB"
            );
            translateTask.setOnFailed(event -> {
                Throwable e = translateTask.getException();
                LOGGER.log(Level.SEVERE, "Error translating message", e);
                showErrorMessage("Error translating message: " + e.getMessage());
            });
            translateTask.setOnSucceeded(translateEvent -> {
                String translatedMessage = translateTask.getValue();
                dictionary.put(translatedMessage, message);
                sendMessage(translatedMessage);
            });
            MainOrchestrator.runTask(translateTask);
        }
        catch (TranslationException e) {
            LOGGER.log(Level.SEVERE, "Error translating message", e);
            showErrorMessage("An internal error happened when trying to translate the message: " + e.getMessage());
        }

    }

    public void startGameClicked() {
        try {
            Session session = Session.getInstance();
            Task<Void> startGameTask = GameService.getInstance().startGame(session.getRoomProperty().getValue());
            startGameTask.setOnFailed(event -> {
                Throwable e = startGameTask.getException();
                LOGGER.log(Level.SEVERE, "Error starting game", e);
                showErrorMessage("Error starting the game: "+e.getMessage());
            });
            MainOrchestrator.runTask(startGameTask);
        } catch (ServiceException e) {
            LOGGER.log(Level.SEVERE, "Error starting game", e);
            showErrorMessage("An internal error happened when trying to start the game: " + e.getMessage());
        }

    }

    public void pickRoomReady() {
        updateRoomsList();
    }

    private void updateRoomsList() {
        try {
            Task<List<RoomDTO>> getRoomsTask = GameService.getInstance().getRooms();
            getRoomsTask.setOnSucceeded(event -> {
                LOGGER.log(Level.INFO, "Rooms retrieved successfully");
                Session session = Session.getInstance();
                session.getRooms().clear();
                session.getRooms().addAll(getRoomsTask.getValue());
            });
            getRoomsTask.setOnFailed(event -> {
                Throwable e = getRoomsTask.getException();
                LOGGER.log(Level.SEVERE, "Error getting rooms", e);
                showErrorMessage("Error getting the list of available rooms: "+e.getMessage());
            });
            MainOrchestrator.runTask(getRoomsTask);
        } catch (ServiceException e) {
            LOGGER.log(Level.SEVERE, "Error getting rooms", e);
            showErrorMessage("An internal error happened when trying to get the list of available rooms: " + e.getMessage());
        }
    }

    public void updateRoom(RoomDTO room) {
        Session session = Session.getInstance();

        // Update the current room
        if (session.getRoomProperty().getValue() != null && session.getRoomProperty().getValue().getId() == room.getId()) {
            session.getRoomProperty().setValue(room);
        }

        // Update the rooms list
        ObservableList<RoomDTO> rooms = session.getRooms();
        for (int i = 0; i < rooms.size(); i++) {
            if (rooms.get(i).getId() == room.getId()) {
                rooms.set(i, room);
                return;
            }
        }
        // If the room is not found, add it to the list
        rooms.add(room);
    }

    public void removeRoom(int roomId) {
        Session session = Session.getInstance();

        // Update the current room
        if (session.getRoomProperty().getValue() != null && session.getRoomProperty().getValue().getId() == roomId) {
            session.getRoomProperty().setValue(null);
        }

        // Update the rooms list
        ObservableList<RoomDTO> rooms = session.getRooms();
        for (int i = 0; i < rooms.size(); i++) {
            if (rooms.get(i).getId() == roomId) {
                LOGGER.info("Removing room " + roomId);
                rooms.remove(i);
                break;
            }
        }
    }

    public void disconnect() {
        if(Session.getInstance().getUserProperty().getValue() == null) {
            LOGGER.log(Level.WARNING, "User is null, cannot disconnect");
            return;
        }

        try {
            LOGGER.log(Level.INFO, "Disconnecting");
            Task<Void> disconnectTask = GameService.getInstance().disconnect();
            disconnectTask.setOnFailed(event -> {
                Throwable e = disconnectTask.getException();
                LOGGER.log(Level.SEVERE, "Error disconnecting", e);
                gameController.showErrorMessage("Error disconnecting: "+e.getMessage());
            });
            disconnectTask.setOnSucceeded(event -> {
                Session.destroy();
                MainOrchestrator.getInstance().onDisconnectSuccess();
            });
            MainOrchestrator.runTask(disconnectTask);
        } catch (ServiceException e) {
            LOGGER.log(Level.SEVERE, "Error disconnecting", e);
            gameController.showErrorMessage("An internal error happened when trying to disconnect: " + e.getMessage());
        }
    }

    public Task<String> getTranslateTask(String message, String langFrom, String langTo) throws TranslationException {
        try {
            return TranslatorService.getInstance().translate(message, langFrom, langTo);
        } catch (ServiceException e) {
            throw new TranslationException("An internal error happened when trying to translate the message: " + e.getMessage());
        }
    }

    public void playerInfoClicked() {
        showPage("playerdomain","profile");
    }

    public void backButtonClicked() {
        gameController.backView();
    }

}