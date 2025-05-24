package org.internationalwhispers.client.presentation.controller.playerdomain;

import javafx.fxml.FXML;
import javafx.scene.control.*;
import javafx.scene.image.ImageView;
import org.internationalwhispers.client.entity.LanguageDTO;
import org.internationalwhispers.client.presentation.helper.Session;
import org.internationalwhispers.client.presentation.orchestrator.playerdomain.AuthOrchestrator;

import java.util.logging.Logger;

public class SignupController {
    private static final Logger LOGGER = Logger.getLogger(SignupController.class.getName());

    @FXML
    private TextField nameField;
    @FXML
    private TextField surnameField;
    @FXML
    private TextField emailField;
    @FXML
    private PasswordField passwordField;
    @FXML
    private PasswordField confirmPasswordField;

    @FXML
    private ComboBox<LanguageDTO> languageComboBox;

    @FXML
    public void initialize() {

        for(LanguageDTO language : Session.getInstance().getLanguages().values()) {
            languageComboBox.getItems().add(language);
        }

        languageComboBox.setCellFactory(listView -> new ListCell<LanguageDTO>() {
                @Override
                protected void updateItem(LanguageDTO item, boolean empty) {
                    super.updateItem(item, empty);
                    if (empty || item == null) {
                        setText(null);
                        setGraphic(null);
                    } else {
                        setText(item.getName());
                        ImageView imageView = new ImageView(item.getFlag());
                        imageView.setFitWidth(32);
                        imageView.setFitHeight(16);
                        setGraphic(imageView);
                    }
                }
            }
        );

        nameField.setOnKeyPressed(event -> {
            if (event.getCode().toString().equals("ENTER")) {
                onSignupButtonClick();
            }
        });
        surnameField.setOnKeyPressed(event -> {
            if (event.getCode().toString().equals("ENTER")) {
                onSignupButtonClick();
            }
        });
        emailField.setOnKeyPressed(event -> {
            if (event.getCode().toString().equals("ENTER")) {
                onSignupButtonClick();
            }
        });
        passwordField.setOnKeyPressed(event -> {
            if (event.getCode().toString().equals("ENTER")) {
                onSignupButtonClick();
            }
        });
        confirmPasswordField.setOnKeyPressed(event -> {
            if (event.getCode().toString().equals("ENTER")) {
                onSignupButtonClick();
            }
        });
    }

    @FXML
    private void onSignupButtonClick() {
        String name = nameField.getText();
        String surname = surnameField.getText();
        String email = emailField.getText();
        String password = passwordField.getText();
        String confirmPassword = confirmPasswordField.getText();
        LanguageDTO language = languageComboBox.getValue();

        if (!email.matches("^[\\w-\\.]+@([\\w-]+\\.)+[\\w-]{2,4}$")) {
            AuthOrchestrator.getInstance().showErrorMessage("Invalid email format.");
        }
        else if (name.isEmpty() || password.isEmpty() || surname.isEmpty() || confirmPassword.isEmpty() || language == null) {
            AuthOrchestrator.getInstance().showErrorMessage("All fields are required.");
        } else if (!password.equals(confirmPassword)) {
            AuthOrchestrator.getInstance().showErrorMessage("Passwords do not match.");
        } else {
            AuthOrchestrator.getInstance().signup(name, surname, email, password, language.getLanguageCode());
        }
    }

    @FXML
    private void onBackButtonClick() {
        AuthOrchestrator.getInstance().backButtonClicked();
    }
}