package org.internationalwhispers.client.presentation.orchestrator.playerdomain;

import javafx.concurrent.Service;
import javafx.concurrent.Task;
import javafx.concurrent.WorkerStateEvent;
import javafx.scene.Cursor;
import javafx.scene.image.Image;
import javafx.stage.Stage;
import lombok.Getter;
import org.internationalwhispers.client.bll.exception.ServiceException;
import org.internationalwhispers.client.bll.gamedomain.GameService;
import org.internationalwhispers.client.bll.gamedomain.TranslatorService;
import org.internationalwhispers.client.entity.LanguageDTO;
import org.internationalwhispers.client.presentation.helper.ResourceLoader;
import org.internationalwhispers.client.presentation.helper.Session;
import org.internationalwhispers.client.presentation.orchestrator.gamedomain.GameOrchestrator;

import java.util.Map;
import java.util.logging.Logger;

public class MainOrchestrator {

    private static final Logger LOGGER = Logger.getLogger(MainOrchestrator.class.getName());
    private static MainOrchestrator instance = null;
    @Getter
    private Stage stage = null;

    private MainOrchestrator() {}

    public static MainOrchestrator getInstance() {
        if (instance == null) {
            instance = new MainOrchestrator();
        }
        return instance;
    }

    public void startApplication(Stage stage) {
        this.stage = stage;

        // Set the application icon
        stage.getIcons().add(new Image(ResourceLoader.loadStream("/presentation/images/icons/icon.png")));

        // Start authentication
        AuthOrchestrator.getInstance().startAuth();
    }

    public void authReady() {
        // Retrieve languages from the server
        try {
            Task<Map<String, LanguageDTO>> languagesTask = TranslatorService.getInstance().getLanguages();
            languagesTask.setOnSucceeded(event -> Session.getInstance().getLanguages().putAll(languagesTask.getValue()));
            languagesTask.setOnFailed(event -> {
                Throwable exception = languagesTask.getException();
                if (exception != null) {
                    LOGGER.severe("Error retrieving languages: " + exception.getMessage());
                    AuthOrchestrator.getInstance().showErrorMessage("Error retrieving languages, please try again later");
                }
            });
            runTask(languagesTask);
        }
        catch (ServiceException e) {
            LOGGER.severe("Error retrieving languages: " + e.getMessage());
            AuthOrchestrator.getInstance().showErrorMessage("An internal error happened when trying to retrieve the list of languages" + e.getMessage());
        }
    }

    public void onSuccessfullyAuthenticated() {
        LOGGER.info("Successfully authenticated");

        // Comunicate to the server that the user is authenticated
        try {
            Task<Void> setUserTask = GameService.getInstance().setUser(Session.getInstance().getUserProperty().getValue().getEmail());
            setUserTask.setOnSucceeded(event -> GameOrchestrator.getInstance().startGame());
            setUserTask.setOnFailed(event -> {
                Throwable e = setUserTask.getException();
                if (e != null) {
                    LOGGER.severe("Error setting user: " + e.getMessage());
                    AuthOrchestrator.getInstance().showErrorMessage("An internal error happened when trying to login: " + e.getMessage());
                }
                GameOrchestrator.getInstance().disconnect();
            });
            runTask(setUserTask);
        }
        catch (ServiceException e) {
            LOGGER.severe("Error setting user: " + e.getMessage());
            AuthOrchestrator.getInstance().showErrorMessage("An internal error happened when trying to login: " + e.getMessage());
            onDisconnectSuccess();
        }

    }

    public void onDisconnectRequest() {
        GameOrchestrator.getInstance().disconnect();
    }

    public void onDisconnectSuccess() {
        AuthOrchestrator.getInstance().startAuth();
    }

    public static <T> void runTask(Task<T> task) {
        task.setOnRunning(event -> {
            getInstance().getStage().getScene().setCursor(Cursor.WAIT); // Change cursor to wait
            getInstance().getStage().getScene().getRoot().setDisable(true); // Disable mouse events
        });

        // Add new handlers without overriding existing handlers
        task.addEventHandler(WorkerStateEvent.WORKER_STATE_SUCCEEDED, event -> {
            getInstance().getStage().getScene().setCursor(Cursor.DEFAULT);
            getInstance().getStage().getScene().getRoot().setDisable(false);
        });
        task.addEventHandler(WorkerStateEvent.WORKER_STATE_FAILED, event -> {
            getInstance().getStage().getScene().setCursor(Cursor.DEFAULT);
            getInstance().getStage().getScene().getRoot().setDisable(false);
        });

        new Service<T>() {
            @Override
            protected Task<T> createTask() {
                return task;
            }
        }.start();
    }

    public void handleFatalError(String error) {
        try {
            LOGGER.severe("Fatal error: " + error);
            AuthOrchestrator.getInstance().showErrorMessageAndExit(error);
        } catch (Exception e) {
            GameOrchestrator.getInstance().showErrorMessageAndExit("A fatal error happened, please report the issue to the administrators: " + e.getMessage());
        }

    }
}
