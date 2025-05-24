package org.internationalwhispers.client.presentation.app;

import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.collections.ListChangeListener;
import javafx.collections.MapChangeListener;
import javafx.scene.Scene;
import javafx.stage.Stage;

import javafx.application.Application;
import org.internationalwhispers.client.bll.playerdomain.AppService;
import org.internationalwhispers.client.entity.LanguageDTO;
import org.internationalwhispers.client.entity.RoomDTO;
import org.internationalwhispers.client.presentation.helper.Session;
import org.internationalwhispers.client.presentation.orchestrator.gamedomain.GameOrchestrator;
import org.internationalwhispers.client.presentation.orchestrator.playerdomain.AuthOrchestrator;
import org.internationalwhispers.client.presentation.orchestrator.playerdomain.MainOrchestrator;

public class InternationalWhispersApplication extends Application {

    @Override
    public void start(Stage stage) {
        AppService.startApplication(stage);

        Runtime.getRuntime().addShutdownHook(new Thread(AppService::closeApplication));

        // The following code is only for testing purposes
        Parameters params = getParameters();
        final String email = params.getRaw().size() > 0 ? params.getRaw().get(0) : null;
        final String password = params.getRaw().size() > 1 ? params.getRaw().get(1) : null;
        final String command = params.getRaw().size() > 2 ? params.getRaw().get(2) : null;

        if (email == null && password == null)
            return;
        Session session = Session.getInstance();

        session.getLanguages().addListener(new MapChangeListener<String, LanguageDTO>() {
                    @Override
                    public void onChanged(Change<? extends String, ? extends LanguageDTO> c) {

                        if (c.wasAdded()) {
                            session.getLanguages().removeListener(this);

                            AuthOrchestrator.getInstance().login(email, password);

                            if (command == null)
                                return;

                            if (command.equals("createRoom")) {
                                MainOrchestrator.getInstance().getStage().sceneProperty().addListener(new ChangeListener<>() {
                                    @Override
                                    public void changed(ObservableValue<? extends Scene> observable, javafx.scene.Scene oldValue, javafx.scene.Scene newValue) {
                                        if (newValue != null) {
                                            GameOrchestrator.getInstance().createRoomRequest("Test Room", "CLOCKWISE");
                                        }

                                        MainOrchestrator.getInstance().getStage().sceneProperty().removeListener(this);
                                    }
                                });
                            }
                            else if (command.equals("joinRoom")) {
                                session.getRooms().addListener(new ListChangeListener<>() {
                                    @Override
                                    public void onChanged(Change<? extends RoomDTO> change) {
                                        while (change.next()) { // Iterate through the changes
                                            if (change.wasAdded()) { // Check if there are added elements
                                                for (RoomDTO room : change.getAddedSubList()) {
                                                    if (room.getId() == 0) {
                                                        GameOrchestrator.getInstance().joinRoomClicked(room);
                                                        session.getRooms().removeListener(this);
                                                        return;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                });
                            }



                        }
                    }
                }
        );
    }

    @Override
    public void stop() {
        AppService.closeApplication();
    }

    public static void main(String[] args) {
        launch(args);
    }

}