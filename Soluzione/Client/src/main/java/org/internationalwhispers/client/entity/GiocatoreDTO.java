package org.internationalwhispers.client.entity;

import javafx.concurrent.Task;
import lombok.Getter;
import org.internationalwhispers.client.bll.gamedomain.TranslatorService;
import org.internationalwhispers.client.presentation.helper.Session;
import org.json.JSONObject;

import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@Getter
public class GiocatoreDTO {

    public static final int MAX_MESSAGE_LENGTH = 50;

    private UserDTO user;
    private String message;
    private Integer stepNumber=null;

    private String translate(String message) {
        Session session = Session.getInstance();
        Map<String,String> dictionary = session.getDictionary();
        if(dictionary.containsKey(message)) {
            return dictionary.get(message);
        }

        try(ExecutorService executor = Executors.newSingleThreadExecutor()) {
            Task<String> translateTask = TranslatorService.getInstance().translate(message, "EN-GB", session.getUserProperty().getValue().getLanguage().getLanguageCode());
            executor.execute(translateTask);
            String translatedMessage = translateTask.get();
            dictionary.put(message, translatedMessage);
            return translatedMessage;
        }
        catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return message + " (translation interrupted)";
        }
        catch (Exception e) {
            return message + " (translation error)";
        }
    }

    public GiocatoreDTO(JSONObject userJson) {
        this.user = new UserDTO(userJson.getJSONObject("utente"));
        this.message = translate(userJson.getString("messaggio"));
        if(!userJson.isNull("stepNumber")) {
            this.stepNumber = userJson.getInt("stepNumber");
        }
    }
}
