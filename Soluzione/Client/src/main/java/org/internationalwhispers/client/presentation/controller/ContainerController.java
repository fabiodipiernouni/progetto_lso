package org.internationalwhispers.client.presentation.controller;

import javafx.collections.ObservableList;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Node;
import javafx.scene.control.Alert;
import javafx.scene.control.DialogEvent;
import javafx.scene.layout.Pane;
import org.internationalwhispers.client.presentation.helper.ResourceLoader;
import org.internationalwhispers.client.presentation.helper.Session;
import org.internationalwhispers.client.presentation.orchestrator.playerdomain.MainOrchestrator;

import java.util.logging.Level;
import java.util.logging.Logger;

public class ContainerController {
    private static final Logger LOGGER = Logger.getLogger(ContainerController.class.getName());

    @FXML
    private Pane contentPane;

    protected ContainerController(){}

    private void showMessage(String message, Alert.AlertType alertType, EventHandler<DialogEvent> onCloseRequest) {
        LOGGER.info("Showing " + alertType + " communication: " + message);
        Alert alert = new Alert(alertType);
        alert.setTitle(String.valueOf(alertType));
        alert.setHeaderText(null);
        alert.setContentText(message);

        if (onCloseRequest != null) {
            alert.setOnCloseRequest(onCloseRequest);
        }

        alert.show();
    }

    private void showMessage(String message, Alert.AlertType alertType) {
        showMessage(message, alertType, null);
    }

    public void showErrorMessage(String error, EventHandler<DialogEvent> onCloseRequest) {
        showMessage(error, Alert.AlertType.ERROR, onCloseRequest);
    }

    public void showErrorMessage(String error) {
        showMessage(error, Alert.AlertType.ERROR);
    }

    public void showWarningMessage(String message, EventHandler<DialogEvent> onCloseRequest) {
        showMessage(message, Alert.AlertType.WARNING, onCloseRequest);
    }

    public void showInfoMessage(String message) {
        showMessage(message, Alert.AlertType.INFORMATION);
    }

    public void backView() {
        Session session = Session.getInstance();
        loadView(session.getPreviousDomain(), session.getPreviousPage());
    }

    public void loadView(String domain, String page) {
        try {
            MainOrchestrator.getInstance().getStage().setTitle("International Whispers - " + page.substring(0, 1).toUpperCase() + page.substring(1));
            Pane newLoadedPane = FXMLLoader.load(ResourceLoader.loadURL("/presentation/views/"+domain+"/" + page + "-view.fxml"));
            ObservableList<Node> contentPaneChildren = contentPane.getChildren();
            contentPaneChildren.clear();
            contentPaneChildren.add(newLoadedPane);

            Session session = Session.getInstance();
            session.setPreviousPage(session.getCurrentPage());
            session.setCurrentPage(page);
            session.setPreviousDomain(session.getCurrentDomain());
            session.setCurrentDomain(domain);

        } catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Error loading view", e);
        }
    }
}
