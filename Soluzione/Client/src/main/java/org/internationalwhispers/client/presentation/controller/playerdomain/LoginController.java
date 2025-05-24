package org.internationalwhispers.client.presentation.controller.playerdomain;

import javafx.fxml.FXML;
import javafx.scene.control.PasswordField;
import javafx.scene.control.TextField;
import org.internationalwhispers.client.presentation.orchestrator.playerdomain.AuthOrchestrator;

public class LoginController {

    @FXML
    private TextField emailField;

    @FXML
    private PasswordField passwordField;

    @FXML
    public void initialize() {
        emailField.setOnKeyPressed(event -> {
            if (event.getCode().toString().equals("ENTER")) {
                onLoginButtonClick();
            }
        });
        passwordField.setOnKeyPressed(event -> {
            if (event.getCode().toString().equals("ENTER")) {
                onLoginButtonClick();
            }
        });
    }

    @FXML
    private void onLoginButtonClick() {
        String email = emailField.getText();
        String password = passwordField.getText();

        if (email.isEmpty() || password.isEmpty()) {
            AuthOrchestrator.getInstance().showErrorMessage("Username and password cannot be empty.");
        } else {
            if(!email.matches("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")) {
                AuthOrchestrator.getInstance().showErrorMessage("Invalid email format.");
            }
            else
                AuthOrchestrator.getInstance().login(email, password);
        }
    }

    @FXML
    private void onSignupLinkClick() {
        AuthOrchestrator.getInstance().signupLinkClicked();
    }
}