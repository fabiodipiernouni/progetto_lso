package org.internationalwhispers.client.presentation.controller.playerdomain;

import javafx.beans.property.Property;
import javafx.fxml.FXML;
import javafx.scene.control.ComboBox;
import javafx.scene.control.ListCell;
import javafx.scene.control.PasswordField;
import javafx.scene.control.TextField;
import javafx.scene.image.ImageView;
import org.internationalwhispers.client.entity.LanguageDTO;
import org.internationalwhispers.client.entity.UserDTO;
import org.internationalwhispers.client.presentation.helper.Session;
import org.internationalwhispers.client.presentation.orchestrator.gamedomain.GameOrchestrator;
import org.internationalwhispers.client.presentation.orchestrator.playerdomain.AuthOrchestrator;

import java.util.logging.Logger;

public class ProfileController {
    private static final Logger LOGGER = Logger.getLogger(ProfileController.class.getName());

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

        Session session = Session.getInstance();

        // Initialize the language combo box with available languages
        for (LanguageDTO language : session.getLanguages().values()) {
            languageComboBox.getItems().add(language);
        }

        Property<UserDTO> userProperty = session.getUserProperty();

        userProperty.addListener((obs, oldUser, newUser) -> setUser(newUser));
        setUser(userProperty.getValue());


    }

    private void setUser(UserDTO user) {

        // Set the password fields to empty, as they should not be pre-filled for security reasons
        passwordField.setText("");
        confirmPasswordField.setText("");

        if (user == null) {
            nameField.setText("");
            surnameField.setText("");
            emailField.setText("");
        }
        else {
            nameField.setText(user.getName());
            surnameField.setText(user.getSurname());
            emailField.setText(user.getEmail());

            // Set the selected language to the user's current language
            languageComboBox.setValue(user.getLanguage());
        }

    }

    public void onUpdateUserButtonClick() {
        String name = nameField.getText();
        String surname = surnameField.getText();
        String email = emailField.getText();
        String password = passwordField.getText();
        String confirmPassword = confirmPasswordField.getText();
        LanguageDTO language = languageComboBox.getValue();

        boolean atLeastOne = false;

        if (!name.isEmpty()) {
            if (name.length() > UserDTO.MAX_NAME_LENGTH) {
                AuthOrchestrator.getInstance().showErrorMessage("Name cannot exceed " + UserDTO.MAX_NAME_LENGTH + " characters.");
                return;
            }
            atLeastOne = true;
        }
        if (!surname.isEmpty()) {
            if (surname.length() > UserDTO.MAX_SURNAME_LENGTH) {
                AuthOrchestrator.getInstance().showErrorMessage("Surname cannot exceed " + UserDTO.MAX_SURNAME_LENGTH + " characters.");
                return;
            }
            atLeastOne = true;
        }
        if (!email.isEmpty()) {
            if(!email.matches("^[\\w-\\.]+@([\\w-]+\\.)+[\\w-]{2,4}$")) {
                AuthOrchestrator.getInstance().showErrorMessage("Invalid email format.");
                return;
            }

            if (email.length() > UserDTO.MAX_EMAIL_LENGTH) {
                AuthOrchestrator.getInstance().showErrorMessage("Email cannot exceed " + UserDTO.MAX_EMAIL_LENGTH + " characters.");
                return;
            }
            atLeastOne = true;
        }
        if (!password.isEmpty()) {
            if (!password.equals(confirmPassword)) {
                AuthOrchestrator.getInstance().showErrorMessage("Passwords do not match.");
                return;
            }
            atLeastOne = true;
        }
        if (language == null) {
            AuthOrchestrator.getInstance().showErrorMessage("Please select a language.");
            return;
        }
        if (!atLeastOne) {
            AuthOrchestrator.getInstance().showErrorMessage("Please fill at least one field to update your profile.");
            return;
        }

        AuthOrchestrator.getInstance().updateUserRequest(name, surname, email, password, language);

    }

    public void onBackButtonClick() {
        GameOrchestrator.getInstance().backButtonClicked();
    }
}
