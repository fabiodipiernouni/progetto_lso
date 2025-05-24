package org.internationalwhispers.client.presentation.controller.playerdomain;

import javafx.fxml.FXML;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import org.internationalwhispers.client.presentation.controller.ContainerController;
import org.internationalwhispers.client.presentation.helper.ResourceLoader;

import java.util.logging.Logger;

public class AuthController extends ContainerController {
    private static final Logger LOGGER = Logger.getLogger(AuthController.class.getName());

    @FXML
    private ImageView logoImageView;

    @FXML
    public void initialize() {
        logoImageView.setImage(new Image(ResourceLoader.loadStream("/presentation/images/icons/logo.png")));
    }

}
