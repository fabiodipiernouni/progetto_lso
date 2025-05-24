package org.internationalwhispers.client.presentation.orchestrator.playerdomain;

import javafx.application.Platform;
import javafx.beans.property.Property;
import javafx.concurrent.Task;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.stage.Stage;
import org.internationalwhispers.client.bll.exception.ServiceException;
import org.internationalwhispers.client.bll.playerdomain.AuthService;
import org.internationalwhispers.client.entity.LanguageDTO;
import org.internationalwhispers.client.entity.UserDTO;
import org.internationalwhispers.client.presentation.controller.playerdomain.AuthController;
import org.internationalwhispers.client.presentation.helper.ResourceLoader;
import org.internationalwhispers.client.presentation.helper.Session;

import java.util.logging.Level;
import java.util.logging.Logger;

public class AuthOrchestrator {
    private static final Logger LOGGER = Logger.getLogger(AuthOrchestrator.class.getName());


    private static AuthOrchestrator instance = null;
    private AuthController authController = null;

    private AuthOrchestrator() {}

    public static AuthOrchestrator getInstance() {
        if (instance == null) {
            instance = new AuthOrchestrator();
        }
        return instance;
    }

    public void showErrorMessage(String error) {
        authController.showErrorMessage(error);
    }

    public void showErrorMessageAndExit(String error) {
        authController.showErrorMessage(error, event -> Platform.exit());
    }

    private void showPage(String page) {
        authController.loadView("playerdomain", page);
    }

    public void startAuth() {
        try {
            FXMLLoader fxmlLoader = new FXMLLoader(ResourceLoader.loadURL("/presentation/views/playerdomain/auth-view.fxml"));

            Stage stage = MainOrchestrator.getInstance().getStage();

            double width;
            double height;

            if(stage.getScene() == null) {
                width = 700.0;
                height = 500.0;
            }
            else {
                // Save current stage dimensions and position
                width = stage.getWidth();
                height = stage.getHeight();
            }

            // Create or load your new scene
            Scene scene = new Scene(fxmlLoader.load());

            // Set the scene
            stage.setScene(scene);

            // Restore original size and position
            stage.setWidth(width);
            stage.setHeight(height);

            stage.show();
            authController = fxmlLoader.getController();
            showPage("login");
            MainOrchestrator.getInstance().authReady();
        } catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Error loading auth view", e);
        }
    }

    public void signupLinkClicked() {
        showPage("signup");
    }

    public void backButtonClicked() {
        showPage("login");
    }

    public void login(String username, String password) {
        try {
            Task<UserDTO> loginTask = AuthService.getInstance().login(username, password);
            loginTask.setOnFailed(event -> {
                Throwable e = loginTask.getException();
                LOGGER.log(Level.SEVERE, "Error logging in", e);
                authController.showErrorMessage("Error logging in: "+e.getMessage());
            });
            loginTask.setOnSucceeded(event -> {
                UserDTO user = loginTask.getValue();
                Session.getInstance().getUserProperty().setValue(user);
                MainOrchestrator.getInstance().onSuccessfullyAuthenticated();
            });
            MainOrchestrator.runTask(loginTask);
        } catch (ServiceException e) {
            LOGGER.log(Level.SEVERE, "Error logging in", e);
            authController.showErrorMessage("An internal error happened when trying to log in:" + e.getMessage());
        }

    }

    public void signup(String name, String surname, String email, String password, String language) {
        try {
            Task<Void> signupTask = AuthService.getInstance().signup(name, surname, email, password, language);
            signupTask.setOnFailed(event -> {
                Throwable e = signupTask.getException();
                LOGGER.log(Level.SEVERE, "Error signing up", e);
                authController.showErrorMessage("Error signing up: "+e.getMessage());
            });
            signupTask.setOnSucceeded(event -> {
                authController.showInfoMessage("Signup successful. You can now login.");
                showPage("login");
            });
            MainOrchestrator.runTask(signupTask);
        } catch (ServiceException e) {
            LOGGER.log(Level.SEVERE, "Error signing up", e);
            authController.showErrorMessage("An internal error happened when trying to sign up" + e.getMessage());
        }

    }

    public void updateUserRequest(String name, String surname, String email, String password, LanguageDTO language) {
        try {
            Property<UserDTO> user = Session.getInstance().getUserProperty();

            Task<UserDTO> updateUserTask = AuthService.getInstance().updateUser(user.getValue(), name, surname, email, password, language.getLanguageCode());
            updateUserTask.setOnFailed(event -> {
                Throwable e = updateUserTask.getException();
                LOGGER.log(Level.SEVERE, "Error updating user", e);
                authController.showErrorMessage("Error updating user: " + e.getMessage());
            });
            updateUserTask.setOnSucceeded(event -> {
                authController.showInfoMessage("User updated successfully.");
                UserDTO updatedUser = updateUserTask.getValue();
                Session.getInstance().getUserProperty().setValue(updatedUser);
            });
            MainOrchestrator.runTask(updateUserTask);
        } catch (ServiceException e) {
            LOGGER.log(Level.SEVERE, "Error updating user", e);
            authController.showErrorMessage("An internal error happened when trying to update user" + e.getMessage());
        }
    }
}
