module org.internationalwhispers.client {
    requires javafx.controls;
    requires javafx.fxml;
    requires com.fasterxml.jackson.databind;
    requires org.yaml.snakeyaml;
    requires org.json;
    requires jdk.jconsole;
    requires jdk.compiler;
    requires java.xml.crypto;
    requires java.logging;
    requires static lombok;

    exports org.internationalwhispers.client.presentation.app to javafx.graphics;
    opens org.internationalwhispers.client.presentation.controller.playerdomain to javafx.fxml;
    opens org.internationalwhispers.client.presentation.controller.gamedomain to javafx.fxml;
    opens org.internationalwhispers.client.entity to javafx.base;
    opens org.internationalwhispers.client.presentation.controller to javafx.fxml;
}