package org.internationalwhispers.client.presentation.helper;

import javafx.beans.property.Property;
import javafx.beans.property.SimpleObjectProperty;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.collections.ObservableMap;
import lombok.Getter;
import lombok.Setter;
import org.internationalwhispers.client.entity.LanguageDTO;
import org.internationalwhispers.client.entity.RoomDTO;
import org.internationalwhispers.client.entity.UserDTO;

import java.util.HashMap;
import java.util.Map;


public class Session {
    private static Session instance=null;
    private Property<UserDTO> user;
    private Property<RoomDTO> room;
    @Getter
    private ObservableList<RoomDTO> rooms = FXCollections.observableArrayList();
    @Getter
    private ObservableMap<String, LanguageDTO> languages = FXCollections.observableHashMap();
    @Getter
    @Setter
    private String previousPage;
    @Getter
    @Setter
    private String currentPage;
    @Getter
    @Setter
    private String previousDomain;
    @Getter
    @Setter
    private String currentDomain;
    @Getter
    private Map<String,String> dictionary = new HashMap<>(); // EN-GB -> user language

    public Property<UserDTO> getUserProperty() {
        if(user == null)
            user = new SimpleObjectProperty<>(null);

        return user;
    }

    public void setUserProperty(Property<UserDTO> user) {
        getUserProperty().setValue(user.getValue());
    }

    public Property<RoomDTO> getRoomProperty() {
        if(room == null)
            room = new SimpleObjectProperty<>(null);

        return room;
    }

    public void setRoomProperty(Property<RoomDTO> room) {
        getRoomProperty().setValue(room.getValue());
    }


    public static Session getInstance() {
        if(instance == null) {
            instance = new Session();
        }
        return instance;
    }

    public static void destroy() {
        if(instance != null) {
            instance.user = null;
            instance.room = null;
            instance.rooms.clear();
            instance.dictionary.clear();
        }
    }

}
