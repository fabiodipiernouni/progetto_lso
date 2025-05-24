package org.internationalwhispers.client.bll.playerdomain;

import javafx.concurrent.Task;
import org.internationalwhispers.client.bll.exception.ServiceException;
import org.internationalwhispers.client.bll.server.RemoteServer;
import org.internationalwhispers.client.bll.server.exception.RemoteServerException;
import org.internationalwhispers.client.bll.server.communication.SocketCommand;
import org.internationalwhispers.client.bll.server.communication.SocketReply;
import org.internationalwhispers.client.entity.UserDTO;
import org.internationalwhispers.client.presentation.helper.ResourceLoader;
import org.json.JSONObject;
import org.yaml.snakeyaml.Yaml;

import java.io.InputStream;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

public class AuthService {
    private static final Logger LOGGER = Logger.getLogger(AuthService.class.getName());
    private static AuthService instance = null;
    private final RemoteServer authMgr;

    private AuthService() throws ServiceException {
        try {
            this.authMgr = new RemoteServer();
            Yaml yaml = new Yaml();
            Map<String, Object> yamlValues;
            try (InputStream inputStream = ResourceLoader.loadStream("application.yaml")) {
                Map<String, Object> config = yaml.load(inputStream);
                config = (Map<String, Object>) config.get("server");
                if (config == null) {
                    throw new IllegalArgumentException("Invalid YAML format: 'server' key not found");
                }
                config = (Map<String, Object>) config.get("authMgr");
                if (config == null) {
                    throw new IllegalArgumentException("Invalid YAML format: authMgr key not found");
                }
                yamlValues = config;
            }

            authMgr.connect(
                (String) yamlValues.get("host"),
                (int) yamlValues.get("port")
            );
        }
        catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Error connecting to authMgr", e);
            throw new ServiceException("Error connecting to authMgr");
        }

    }

    public static AuthService getInstance() throws ServiceException {
        if (instance == null) {
            instance = new AuthService();
        }
        return instance;
    }

    public Task<UserDTO> login(String email, String password) {
        return new Task<>() {

            @Override
            protected UserDTO call() throws ServiceException {
                try {
                    JSONObject payload = new JSONObject();

                    payload.put("email", email);
                    payload.put("password", password);

                    SocketReply reply = authMgr.sendAndReceive(new SocketCommand("login", payload));
                    if(reply.isError()) {
                        LOGGER.severe("Login failed: " + reply.getPayload());
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("Login successful");
                    LOGGER.info(reply.getPayload().toString());

                    JSONObject userJson = (JSONObject) reply.getPayload();
                    return new UserDTO(userJson);
                }
                catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error during login using sockets", e);
                    throw new ServiceException("Error during login request");
                }

            }
        };
    }

    public Task<Void> signup(String name, String surname, String email, String password, String language) {
        return new Task<>() {
            @Override
            protected Void call() throws ServiceException {
                try {
                    JSONObject payload = new JSONObject();
                    payload.put("nome", name);
                    payload.put("cognome", surname);
                    payload.put("email", email);
                    payload.put("password", password);
                    payload.put("lingua", language);

                    SocketReply reply = authMgr.sendAndReceive(new SocketCommand("register", payload));
                    if(reply.isError()) {
                        LOGGER.severe("Signup failed: " + reply.getPayload());
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("Signup successful");
                    return null;

                }
                catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error during signup using sockets", e);
                    throw new ServiceException("Error during signup request");
                }
            }
        };
    }

    public Task<UserDTO> updateUser(UserDTO user, String newName, String newSurname, String newEmail, String newPassword, String newLanguage) {
        return new Task<>() {
            @Override
            protected UserDTO call() throws ServiceException {
                try {
                    JSONObject payload = new JSONObject();
                    if(!newName.isEmpty())
                        payload.put("new_nome", newName);
                    if(!newSurname.isEmpty())
                        payload.put("new_cognome", newSurname);
                    if(!newEmail.isEmpty())
                        payload.put("new_email", newEmail);
                    if(!newPassword.isEmpty())
                        payload.put("new_password", newPassword);
                    if(!newLanguage.isEmpty())
                        payload.put("new_lang", newLanguage);
                    payload.put("email", user.getEmail());


                    SocketReply reply = authMgr.sendAndReceive(new SocketCommand("update_user", payload));
                    if(reply.isError()) {
                        LOGGER.severe("Update failed: " + reply.getPayload());
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("Update successful");
                    JSONObject userJson = (JSONObject) reply.getPayload();
                    return new UserDTO(userJson);

                }
                catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error during update using sockets", e);
                    throw new ServiceException("Error during update request");
                }
            }
        };
    }

}
