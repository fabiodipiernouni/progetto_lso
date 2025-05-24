package org.internationalwhispers.client.bll.playerdomain;

import javafx.stage.Stage;
import org.internationalwhispers.client.presentation.orchestrator.gamedomain.GameOrchestrator;
import org.internationalwhispers.client.presentation.orchestrator.playerdomain.MainOrchestrator;

public class AppService {
    private AppService() {}

    public static void startApplication(Stage stage) {
        MainOrchestrator.getInstance().startApplication(stage);
    }

    public static void closeApplication() {
        GameOrchestrator.getInstance().disconnect();
        Thread.getAllStackTraces().keySet().forEach(thread -> {
            if (!thread.isDaemon() && !thread.getName().startsWith("JavaFX")) {
                thread.interrupt();
            }
        });

    }
}
