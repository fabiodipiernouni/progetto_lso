package org.internationalwhispers.client.presentation.controller.gamedomain;

import javafx.application.Platform;
import javafx.beans.property.ReadOnlyObjectWrapper;
import javafx.collections.ListChangeListener;
import javafx.fxml.FXML;
import javafx.scene.control.*;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.scene.image.ImageView;
import javafx.scene.layout.HBox;
import org.internationalwhispers.client.entity.RoomDTO;
import org.internationalwhispers.client.presentation.helper.Session;
import org.internationalwhispers.client.presentation.orchestrator.gamedomain.GameOrchestrator;

public class PickRoomController {

    @FXML
    private TableView<RoomDTO> roomTableView;

    @FXML
    private TableColumn<RoomDTO, String> roomNameColumn;

    @FXML
    private TableColumn<RoomDTO, HBox> hostColumn;

    @FXML
    private TableColumn<RoomDTO, String> playersColumn;

    @FXML
    private TableColumn<RoomDTO, String> statusColumn;

    @FXML
    private TableColumn<RoomDTO, Button> joinColumn;

    @FXML
    public void initialize() {

        roomNameColumn.setCellValueFactory(new PropertyValueFactory<>("name"));
        hostColumn.setCellValueFactory(cellData -> {
            HBox hbox = new HBox(5); // 5 is the spacing between the username amd the flag
            hbox.getStyleClass().add("table-column");
            ImageView flag = new ImageView();
            flag.setFitWidth(32);
            flag.setFitHeight(16);
            flag.setImage(cellData.getValue().getHost().getLanguage().getFlag());
            Label username = new Label(cellData.getValue().getHost().getFullName());
            hbox.getChildren().addAll(username, flag);
            return new ReadOnlyObjectWrapper<>(hbox);
        });
        hostColumn.setComparator((HBox hbox1, HBox hbox2) -> {
            Label label1 = (Label) hbox1.getChildren().get(0);
            Label label2 = (Label) hbox2.getChildren().get(0);
            return label1.getText().compareTo(label2.getText());
        });
        playersColumn.setCellValueFactory(cellData -> {
            String playersText = String.valueOf(cellData.getValue().getPlayersNumber());
            playersText += " / " + GameOrchestrator.MAX_PLAYERS;
            if (cellData.getValue().getPlayersNumber() < GameOrchestrator.MIN_PLAYERS && cellData.getValue().getStatus() == RoomDTO.GameStatus.WAITING_PLAYERS) {
                playersText += " (need " + (GameOrchestrator.MIN_PLAYERS - cellData.getValue().getPlayersNumber()) + " more)";
            }

            return new ReadOnlyObjectWrapper<>(playersText);
        });
        playersColumn.setComparator((String s1, String s2) -> {
            int players1 = Integer.parseInt(s1.split("/")[0].trim());
            int players2 = Integer.parseInt(s2.split("/")[0].trim());
            return Integer.compare(players1, players2);
        });
        statusColumn.setCellValueFactory(new PropertyValueFactory<>("status"));
        joinColumn.setCellValueFactory(cellData -> {
            Button joinButton = new Button("Join");
            joinButton.getStyleClass().add("join-button");
            joinButton.setOnAction(event ->
                GameOrchestrator.getInstance().joinRoomClicked(cellData.getValue())
            );
            return new ReadOnlyObjectWrapper<>(joinButton);
        });

        Session.getInstance().getRooms().addListener((ListChangeListener<? super RoomDTO>) observable -> {
            roomTableView.refresh();
        });
        roomTableView.setItems(Session.getInstance().getRooms());

        Platform.runLater(() -> GameOrchestrator.getInstance().pickRoomReady());

    }

    public void createRoomClick() {
        GameOrchestrator.getInstance().createRoomClicked();
    }
}
