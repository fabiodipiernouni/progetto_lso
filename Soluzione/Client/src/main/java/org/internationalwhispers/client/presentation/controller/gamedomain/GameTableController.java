package org.internationalwhispers.client.presentation.controller.gamedomain;

import javafx.beans.property.Property;
import javafx.fxml.FXML;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.control.Tooltip;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Pane;
import javafx.scene.image.ImageView;
import javafx.scene.layout.VBox;
import javafx.util.Duration;
import org.internationalwhispers.client.entity.GiocatoreDTO;
import org.internationalwhispers.client.entity.RoomDTO;
import org.internationalwhispers.client.entity.UserDTO;
import org.internationalwhispers.client.presentation.helper.Session;
import org.internationalwhispers.client.presentation.orchestrator.gamedomain.GameOrchestrator;

public class GameTableController {
    private static final java.util.logging.Logger LOGGER = java.util.logging.Logger.getLogger(GameTableController.class.getName());

    @FXML
    private Pane pane;
    @FXML
    private Button startButton;
    @FXML
    private Button sendButton;
    @FXML
    private Label roomNameLabel;
    @FXML
    private Label playerCount;
    @FXML
    private Label directionArrow;
    @FXML
    private TextField currentMessageField;
    @FXML
    private TextField messageField;
    @FXML
    private GridPane gameTableGrid;

    @FXML
    public void initialize() {

        for(int i=0; i<GameOrchestrator.MAX_PLAYERS; i++) {
            HBox slot = (HBox) pane.lookup("#slot" + i);
            VBox playerBox = (VBox) slot.getChildren().getFirst();

            Label playerName = (Label) playerBox.getChildren().get(0);
            Label playerStatus = (Label) playerBox.getChildren().get(1);
            Label playerInfo = (Label) playerBox.getChildren().get(2);

            playerName.textProperty().addListener((observable, oldValue, newValue) -> setTooltip(newValue, playerName));

            playerStatus.textProperty().addListener((observable, oldValue, newValue) -> setTooltip(newValue, playerStatus));

            playerInfo.textProperty().addListener((observable, oldValue, newValue) -> setTooltip(newValue, playerInfo));

        }

        Property<RoomDTO> roomProperty = Session.getInstance().getRoomProperty();
        roomProperty.addListener((obs, oldRoom, newRoom) -> setRoom(newRoom));
        setRoom(roomProperty.getValue());

        messageField.setOnKeyPressed(event -> {
            if (event.getCode().toString().equals("ENTER")) {
                onSendButtonClick();
            }
        });
    }

    private static void setTooltip(String text, Node playerName) {
        if(text.isEmpty()) {
            Tooltip.install(playerName, null);
        }

        Tooltip tooltip = new Tooltip(text);
        tooltip.setShowDelay(Duration.millis(0));
        tooltip.setShowDuration(Duration.INDEFINITE);
        Tooltip.install(playerName, tooltip);
    }

    private void setRoom(RoomDTO room) {
        if(room == null)
            return;

        hide(startButton);
        hide(sendButton);
        hide(messageField);
        hide(currentMessageField);

        UserDTO user = Session.getInstance().getUserProperty().getValue();

        // HEADER
        roomNameLabel.setText(room.getName());
        StringBuilder playerCountText = new StringBuilder();
        playerCountText
                .append(room.getPlayersNumber())
                .append("/")
                .append(GameOrchestrator.MAX_PLAYERS);
        if(room.getStatus() == RoomDTO.GameStatus.WAITING_PLAYERS || room.getStatus() == RoomDTO.GameStatus.GAME_FINISHED) {
            if(room.getPlayersNumber() < GameOrchestrator.MIN_PLAYERS) {
                playerCountText
                        .append(" (need ")
                        .append(GameOrchestrator.MIN_PLAYERS - room.getPlayersNumber())
                        .append(" more)");
            }
            else {
                if(room.getHost().equals(user)) {
                    playerCountText
                            .append("(you can start the game)");
                    show(startButton);
                }
                else {
                    playerCountText
                            .append("(ready to start)");
                }
            }
        }

        gameTableGrid.setStyle("-fx-background-color: #1a7e2c;");
        int queueCount=0;
        for(UserDTO u : room.getQueue()) {
            queueCount++;
            if(u.equals(user)) {
                playerCountText.append("Your position in queue: ").append(queueCount);
                gameTableGrid.setStyle("-fx-background-color: grey;");
            }
        }
        if(queueCount>0)
            playerCountText.append(" (queue count: " + queueCount + ")");

        playerCount.setText(playerCountText.toString());

        // DIRECTION
        if(room.getGameDirection() == RoomDTO.GameDirection.CLOCKWISE) {
            directionArrow.setText("⟳");
        }
        else {
            directionArrow.setText("⟲");
        }

        for(int i=0; i<GameOrchestrator.MAX_PLAYERS; i++) {
            HBox slot = (HBox) pane.lookup("#slot" + i);
            slot.getStyleClass().remove("player-box-active");

            VBox playerBox = (VBox) slot.getChildren().getFirst();

            Label playerName = (Label) playerBox.getChildren().get(0);
            Label playerStatus = (Label) playerBox.getChildren().get(1);
            Label playerInfo = (Label) playerBox.getChildren().get(2);

            ImageView playerFlag = (ImageView) slot.getChildren().get(1);
            playerFlag.setFitWidth(32);
            playerFlag.setFitHeight(16);

            GiocatoreDTO player = room.getPlayers().get(i);

            if(player != null) {

                playerName.setText(player.getUser().getFullName());

                playerStatus.setText("Waiting...");
                switch (room.getStatus()) {
                    case RoomDTO.GameStatus.GAME_IN_PROGRESS:
                        if (room.getCurrentPlayer() != null && room.getCurrentPlayer().equals(player.getUser())) {
                            playerStatus.setText("Writing...");
                            slot.getStyleClass().add("player-box-active");
                        } else {

                            if (!player.getMessage().isEmpty()) {
                                playerStatus.setText("Has sent a message");
                            }
                        }
                        break;
                    case RoomDTO.GameStatus.GAME_FINISHED:
                        if(player.getStepNumber() != null) {
                            // if it doesn't enter this if it means that the user joined after the game ended
                            final int lastStep = player.getStepNumber();

                            StringBuilder currentMessage = new StringBuilder();
                            for(GiocatoreDTO p : room.getPlayers()) {
                                if(p != null && p.getStepNumber() != null && p.getStepNumber() <= lastStep) {
                                    currentMessage.append(p.getMessage()).append(" ");
                                }
                            }

                            if(!currentMessage.isEmpty())
                                currentMessage.setLength(currentMessage.length() - 1); // remove last space

                            playerStatus.setText(lastStep + ": " + currentMessage);

                        }
                }

                String playerInfoText = "";
                if(player.getUser().equals(user)) {
                    playerInfoText += "(You)";
                }
                if(player.getUser().equals(room.getHost())) {
                    playerInfoText += "(Host)";
                }
                playerInfo.setText(playerInfoText);

                playerFlag.setImage(player.getUser().getLanguage().getFlag());
                playerFlag.setFitWidth(32);
                playerFlag.setFitHeight(16);
            }
            else {
                playerName.setText("Empty slot");
                playerStatus.setText("");
                playerInfo.setText("");
                slot.getStyleClass().remove("player-box-active");
                playerFlag.setImage(null);
            }

            if(playerStatus.getText().isEmpty()) {
                hide(playerStatus);
            }
            else {
                show(playerStatus);
            }
            if(playerInfo.getText().isEmpty()) {
                hide(playerInfo);
            }
            else {
                show(playerInfo);
            }
        }


        // MESSAGE FIELD
        if(room.getCurrentPlayer() != null && room.getCurrentPlayer().equals(user)) {
            show(messageField);

            messageField.clear();
            currentMessageField.clear();

            show(sendButton);


            int lastStep = -1;
            StringBuilder currentMessage = new StringBuilder();
            for(GiocatoreDTO player : room.getPlayers()) {
                if(player != null && player.getStepNumber() != null) {
                    currentMessage.append(player.getMessage()).append(" ");
                    if(player.getStepNumber() > lastStep) {
                        lastStep = player.getStepNumber();
                    }
                }
            }

            if(!currentMessage.isEmpty()) {
                currentMessage.setLength(currentMessage.length() - 1); // remove last space
                currentMessageField.setText(currentMessage.toString());
                show(currentMessageField);
            }
            messageField.requestFocus();

        }
        else {
            hide(messageField);
            hide(sendButton);
        }

    }

    public void hide(Node element) {
        element.setVisible(false);
        element.setManaged(false);
    }

    public void show(Node element) {
        element.setVisible(true);
        element.setManaged(true);
    }

    @FXML
    public void onLeaveButtonClick() {
        GameOrchestrator.getInstance().leaveRoomClicked();
    }

    @FXML
    public void onStartButtonClick() {
        GameOrchestrator.getInstance().startGameClicked();
    }

    @FXML
    public void onSendButtonClick() {
        String message = messageField.getText().trim();
        if (message.isEmpty()) {
            GameOrchestrator.getInstance().showErrorMessage("Message cannot be empty");
        } else if (!message.matches("\\w+")) {
            // Check that the message contains only one word
            GameOrchestrator.getInstance().showErrorMessage("Message must contain only one word");
        } else if (message.length() > GiocatoreDTO.MAX_MESSAGE_LENGTH) {
            GameOrchestrator.getInstance().showErrorMessage("Message can't be longer than " + GiocatoreDTO.MAX_MESSAGE_LENGTH + " characters");
        }
        else {
            GameOrchestrator.getInstance().sendMessageClicked(message);
        }
    }
}