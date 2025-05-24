package org.internationalwhispers.client.bll.gamedomain;

import javafx.concurrent.Task;
import org.internationalwhispers.client.bll.exception.ServiceException;
import org.internationalwhispers.client.bll.server.RemoteServer;
import org.internationalwhispers.client.bll.server.communication.SocketCommand;
import org.internationalwhispers.client.bll.server.communication.SocketReply;
import org.internationalwhispers.client.bll.server.exception.RemoteServerException;
import org.internationalwhispers.client.entity.LanguageDTO;
import org.internationalwhispers.client.presentation.helper.ResourceLoader;
import org.json.JSONArray;
import org.json.JSONObject;
import org.yaml.snakeyaml.Yaml;

import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

public class TranslatorService {
    private static final Logger LOGGER = Logger.getLogger(TranslatorService.class.getName());
    private static TranslatorService instance = null;
    private final RemoteServer translatorSrv;

    private TranslatorService() throws ServiceException {
        try {
            this.translatorSrv = new RemoteServer();
            Yaml yaml = new Yaml();
            Map<String, Object> yamlValues;
            try (InputStream inputStream = ResourceLoader.loadStream("application.yaml")) {
                Map<String, Object> config = yaml.load(inputStream);
                config = (Map<String, Object>) config.get("server");
                if (config == null) {
                    throw new IllegalArgumentException("Invalid YAML format: 'server' key not found");
                }
                config = (Map<String, Object>) config.get("translatorSrv");
                if (config == null) {
                    throw new IllegalArgumentException("Invalid YAML format: translatorSrv key not found");
                }
                yamlValues = config;
            }

            translatorSrv.connect(
                    (String) yamlValues.get("host"),
                    (int) yamlValues.get("port")
            );
        }
        catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Error connecting to translatorSrv", e);
            throw new ServiceException("Error connecting to translatorSrv");
        }

    }

    public static TranslatorService getInstance() throws ServiceException {
        if (instance == null) {
            instance = new TranslatorService();
        }
        return instance;
    }

    public Task<String> translate(String message, String langFrom, String langTo) {
        return new Task<>() {
            @Override
            protected String call() throws ServiceException {
                if(langFrom.equals(langTo)) {
                    LOGGER.info("No translation needed, same language");
                    return message;
                }
                if(message == null || message.isEmpty()) {
                    LOGGER.info("No translation needed, empty message");
                    return message;
                }
                try {
                    JSONObject payload = new JSONObject();
                    payload.put("message", message);
                    payload.put("lang_from", langFrom);
                    payload.put("lang_to", langTo);
                    LOGGER.info("Translating message: " + payload);

                    SocketReply reply = translatorSrv.sendAndReceive(new SocketCommand("translate", payload));
                    if(reply.isError()) {
                        LOGGER.severe("Translate failed: " + payload);
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("Translate successful");

                    JSONObject translatedMessage = (JSONObject) reply.getPayload();
                    return translatedMessage.getString("translated_msg");

                } catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error translating message", e);
                    throw new ServiceException("Cannot translate message.");
                }
            }
        };
    }

    public Task<Map<String, LanguageDTO>> getLanguages() {
        return new Task<>() {
            @Override
            protected Map<String, LanguageDTO> call() throws ServiceException {
                try {
                    SocketReply reply = translatorSrv.sendAndReceive(new SocketCommand("get_languages", null));
                    if(reply.isError()) {
                        LOGGER.severe("Get languages failed: " + reply.getPayload());
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("Get languages successful");

                    JSONArray languagesJson = (JSONArray) reply.getPayload();
                    Map<String, LanguageDTO> languages = new HashMap<>();

                    for (int i = 0; i < languagesJson.length(); i++) {
                        JSONObject languageJson = languagesJson.getJSONObject(i);
                        String language = languageJson.getString("language");
                        String name = languageJson.getString("name");
                        LanguageDTO languageDTO = new LanguageDTO(language, name);
                        languages.put(language, languageDTO);
                    }

                    return languages;

                } catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error getting languages", e);
                    throw new ServiceException("Cannot get languages.");
                }
            }
        };
    }
}
