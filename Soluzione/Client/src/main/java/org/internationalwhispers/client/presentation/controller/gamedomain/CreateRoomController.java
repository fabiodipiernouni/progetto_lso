package org.internationalwhispers.client.presentation.controller.gamedomain;

import javafx.fxml.FXML;
import javafx.scene.control.ComboBox;
import javafx.scene.control.TextField;
import org.internationalwhispers.client.entity.RoomDTO;
import org.internationalwhispers.client.presentation.orchestrator.gamedomain.GameOrchestrator;

public class CreateRoomController {

    @FXML
    private TextField nameField;

    @FXML
    private ComboBox<String> directionComboBox;


    @FXML
    private void initialize() {
        nameField.setOnKeyPressed(event -> {
            if (event.getCode().toString().equals("ENTER")) {
                onCreateButtonClick();
            }
        });
    }

    public void pickRoomClick() {
        GameOrchestrator.getInstance().pickRoomClicked();
    }

    public void onCreateButtonClick() {
        String name = nameField.getText();
        String direction = directionComboBox.getValue();

        if (name.isEmpty()) {
            GameOrchestrator.getInstance().showErrorMessage("Room name cannot be empty.");
        } else if (name.length() > RoomDTO.MAX_NAME_LENGTH) {
            GameOrchestrator.getInstance().showErrorMessage("Room name cannot exceed " + RoomDTO.MAX_NAME_LENGTH + " characters.");
        } else {
            if (direction == null) {
                GameOrchestrator.getInstance().showErrorMessage("Choose a direction.");
            } else {
                try {
                    GameOrchestrator.getInstance().createRoomRequest(name, direction);
                } catch (Exception e) {
                    GameOrchestrator.getInstance().showErrorMessage("An error occurred while creating the room: " + e.getMessage());
                }
            }
        }

    }
}
