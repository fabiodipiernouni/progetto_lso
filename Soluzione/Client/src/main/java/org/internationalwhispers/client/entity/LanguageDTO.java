package org.internationalwhispers.client.entity;

import javafx.scene.image.Image;
import lombok.Getter;
import org.internationalwhispers.client.presentation.helper.ResourceLoader;
import org.internationalwhispers.client.presentation.helper.exception.ResourceException;

@Getter
public class LanguageDTO {
    public static final int MAX_LANGUAGE_CODE_LENGTH = 10;

    private String languageCode;
    private String name;
    private Image flag;

    public LanguageDTO(String languageCode, String name) {
        this.languageCode = languageCode;
        this.name = name;
        try {
            this.flag = new Image(ResourceLoader.loadStream("/presentation/images/flags/" + languageCode + ".png"));
        } catch (ResourceException e) {
            this.flag = new Image(ResourceLoader.loadStream("/presentation/images/flags/DEFAULT.png"));
        }
    }

    @Override
    public String toString() {
        return name;
    }
}
