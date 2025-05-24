package org.internationalwhispers.client.entity;

import lombok.Getter;
import lombok.NonNull;
import org.internationalwhispers.client.presentation.helper.Session;
import org.json.JSONObject;

@Getter
public class UserDTO {
    public static final int MAX_NAME_LENGTH = 127;
    public static final int MAX_SURNAME_LENGTH = 127;
    public static final int MAX_EMAIL_LENGTH = 127;

    private int id;
    private String name;
    private String surname;
    private String email;
    @NonNull
    private LanguageDTO language;

    public UserDTO(JSONObject userJson) {
        this.id = userJson.getInt("id_utente");
        this.name = userJson.getString("nome");
        this.surname = userJson.getString("cognome");
        this.email = userJson.getString("email");
        this.language = Session.getInstance().getLanguages().get(userJson.getString("codice_lingua"));
    }

    public boolean equals(UserDTO otherUserDTO) {
        return email.equals(otherUserDTO.email);
    }

    public String getFullName() {
        return name + " " + surname;
    }
}
