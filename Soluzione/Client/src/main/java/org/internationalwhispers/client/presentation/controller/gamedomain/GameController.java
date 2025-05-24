package org.internationalwhispers.client.presentation.controller.gamedomain;

import javafx.beans.property.Property;
import javafx.fxml.FXML;
import javafx.scene.control.Label;
import javafx.scene.image.ImageView;
import org.internationalwhispers.client.entity.UserDTO;
import org.internationalwhispers.client.presentation.controller.ContainerController;
import org.internationalwhispers.client.presentation.helper.Session;
import org.internationalwhispers.client.presentation.orchestrator.gamedomain.GameOrchestrator;

import java.util.logging.Logger;

public class GameController extends ContainerController {
    private static final Logger LOGGER = Logger.getLogger(GameController.class.getName());

    @FXML
    private Label playerNameLabel;
    @FXML
    private ImageView playerFlagImageView;    @FXML
    public void initialize() {
        // Add an action for playerNameLabel
        playerNameLabel.setOnMouseClicked(event -> onPlayerInfoClick());

        // Add an action for playerFlagImageView
        playerFlagImageView.setOnMouseClicked(event -> onPlayerInfoClick());

        Property<UserDTO> user = Session.getInstance().getUserProperty();
        user.addListener((observable, oldValue, newValue) -> {
            setUser(newValue);
        });
        setUser(user.getValue());
    }    private void onPlayerInfoClick() {
        if(Session.getInstance().getCurrentPage().equals("gametable")) {
            showErrorMessage("It is not possible to modify profile information during the game.");
        }
        else {
            GameOrchestrator.getInstance().playerInfoClicked();
        }
    }

    private void setUser(UserDTO user) {
        playerNameLabel.setText(user.getFullName());
        playerFlagImageView.setImage(user.getLanguage().getFlag());
    }

    public void onLogoutButtonClick() {
        GameOrchestrator.getInstance().logoutButtonClicked();
    }

}
