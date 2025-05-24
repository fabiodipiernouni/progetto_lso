package org.internationalwhispers.client.bll.gamedomain;

import javafx.concurrent.Task;
import lombok.Getter;
import org.internationalwhispers.client.bll.exception.ResourceNotFoundException;
import org.internationalwhispers.client.bll.exception.ServiceException;
import org.internationalwhispers.client.bll.server.CommandsDispatcher;
import org.internationalwhispers.client.bll.server.RemoteServer;
import org.internationalwhispers.client.bll.server.communication.SocketCommand;
import org.internationalwhispers.client.bll.server.communication.SocketReply;
import org.internationalwhispers.client.bll.server.exception.RemoteServerException;
import org.internationalwhispers.client.entity.RoomDTO;
import org.internationalwhispers.client.presentation.helper.ResourceLoader;
import org.json.JSONArray;
import org.json.JSONObject;
import org.yaml.snakeyaml.Yaml;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

public class GameService {
    private static final Logger LOGGER = Logger.getLogger(GameService.class.getName());
    private static GameService instance = null;
    @Getter
    private final RemoteServer gameRulesMgr;

    private GameService() throws ServiceException {
        try {
            this.gameRulesMgr = new RemoteServer();
            Yaml yaml = new Yaml();
            Map<String, Object> yamlValues;
            try (InputStream inputStream = ResourceLoader.loadStream("application.yaml")) {
                Map<String, Object> config = yaml.load(inputStream);
                config = (Map<String, Object>) config.get("server");
                if (config == null) {
                    throw new IllegalArgumentException("Invalid YAML format: 'server' key not found");
                }
                config = (Map<String, Object>) config.get("gameRulesMgr");
                if (config == null) {
                    throw new IllegalArgumentException("Invalid YAML format: gameRulesMgr key not found");
                }
                yamlValues = config;
            }

            gameRulesMgr.connect(
                    (String) yamlValues.get("host"),
                    (int) yamlValues.get("port")
            );
        }
        catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Error connecting to gameRulesMgr", e);
            throw new ServiceException("Error connecting to gameRulesMgr");
        }

        // Start the dispatcher thread
        CommandsDispatcher.getInstance(gameRulesMgr);
    }

    public static GameService getInstance() throws ServiceException {
        if (instance == null) {
            instance = new GameService();
        }
        return instance;
    }

    public Task<RoomDTO> createRoom(String name, String direction) {
        return new Task<>() {
            @Override
            protected RoomDTO call() throws ServiceException {
                try {
                    JSONObject payload = new JSONObject();
                    payload.put("nome_stanza", name);
                    payload.put("ordine_gioco", RoomDTO.GameDirection.fromString(direction).getMapping());

                    SocketReply reply = gameRulesMgr.sendAndReceive(new SocketCommand("create_room", payload));
                    if(reply.isError()) {
                        LOGGER.severe("Create room failed: " + reply.getPayload());
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("Create room successful");

                    JSONObject roomJson = (JSONObject) reply.getPayload();
                    return new RoomDTO(roomJson);

                } catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error creating room", e);
                    throw new ServiceException("Cannot create the room.");
                }
            }
        };
    }

    public Task<Void> leaveRoom(RoomDTO room) {
        return new Task<>() {
            @Override
            protected Void call() throws ServiceException {
                try {
                    JSONObject payload = new JSONObject();
                    payload.put("id_stanza", room.getId());
                    SocketReply reply = gameRulesMgr.sendAndReceive(new SocketCommand("leave_room", payload));
                    if(reply.isError()) {
                        LOGGER.severe("Leave room failed: " + reply.getPayload());
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("Leave room successful");
                    return null;

                } catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error leaving room", e);
                    throw new ServiceException("Cannot leave the room.");
                }
            }
        };
    }

    public Task<Void> joinRoom(RoomDTO room) {
        return new Task<>() {
            @Override
            protected Void call() throws ServiceException {
                try {
                    JSONObject payload = new JSONObject();
                    payload.put("id_stanza", room.getId());
                    SocketReply reply = gameRulesMgr.sendAndReceive(new SocketCommand("join_room", payload));
                    if(reply.isError()) {
                        LOGGER.severe("Join room failed: " + reply.getPayload());
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("Join room successful");
                    return null;

                } catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error joining room", e);
                    throw new ServiceException("Cannot join the room.");
                }
            }
        };
    }

    public Task<Void> sendMessage(String message) {
        return new Task<>() {
            @Override
            protected Void call() throws ServiceException {
                try {
                    JSONObject payload = new JSONObject();
                    payload.put("messaggio", message);
                    SocketReply reply = gameRulesMgr.sendAndReceive(new SocketCommand("send_message", payload));
                    if(reply.isError()) {
                        LOGGER.severe("Send message failed: " + reply.getPayload());
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("Send message successful");

                } catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error sending communication to the room", e);
                    throw new ServiceException("Cannot send communication to the room.");
                }
                return null;
            }
        };
    }

    public Task<Void> startGame(RoomDTO room) {
        return new Task<>() {
            @Override
            protected Void call() throws ServiceException {
                try {
                    JSONObject payload = new JSONObject();
                    payload.put("id_stanza", room.getId());
                    SocketReply reply = gameRulesMgr.sendAndReceive(new SocketCommand("start_game", payload));
                    if(reply.isError()) {
                        LOGGER.severe("Game start failed: " + reply.getPayload());
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("Game start successful");
                    return null;

                } catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error starting game in the room", e);
                    throw new ServiceException("Cannot start game in the room.");
                }
            }
        };
    }

    public Task<List<RoomDTO>> getRooms() {
        return new Task<>() {
            @Override
            protected List<RoomDTO> call() throws ServiceException {
                try {
                    SocketReply reply = gameRulesMgr.sendAndReceive(new SocketCommand("get_rooms", null));
                    if(reply.isError()) {
                        LOGGER.severe("Get rooms failed: " + reply.getPayload());
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("Get rooms successful");

                    List<RoomDTO> rooms = new ArrayList<>();
                    for (Object roomObj : (JSONArray) reply.getPayload()) {
                        JSONObject roomJson = (JSONObject) roomObj;
                        rooms.add(new RoomDTO(roomJson));
                    }
                    return rooms;

                } catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error getting rooms", e);
                    throw new ServiceException("Cannot get the list of rooms.");
                }
            }
        };
    }

    public Task<Void> setUser(String email) {
        return new Task<>() {

            @Override
            protected Void call() throws ServiceException {
                try {
                    LOGGER.info("Setting user: " + email);

                    JSONObject payload = new JSONObject();

                    payload.put("email", email);
                    LOGGER.info("Payload JSON: " + payload);

                    SocketReply reply = gameRulesMgr.sendAndReceive(new SocketCommand("set_user", payload));
                    if(reply.isError()) {
                        LOGGER.severe("set_user failed: " + reply.getPayload());
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("set_user successful");
                    LOGGER.info(reply.getPayload().toString());

                    return null;
                }
                catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error during set user using sockets", e);
                    throw new ServiceException("Error during set user request");
                }

            }
        };
    }

    public Task<Void> disconnect() {

        return new Task<>() {
            @Override
            protected Void call() throws ServiceException {
                try {
                    SocketReply reply = gameRulesMgr.sendAndReceive(new SocketCommand("disconnect", null));
                    if(reply.isError()) {
                        LOGGER.severe("Disconnect failed: " + reply.getPayload());
                        throw new ServiceException(reply.getPayload().toString());
                    }
                    LOGGER.info("Disconnect successful");

                    return null;
                }
                catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error during disconnect using sockets", e);
                    throw new ServiceException("Error during disconnect request");
                }
            }
        };

    }

    public Task<RoomDTO> getRoom(int roomId) {
        return new Task<>() {
            @Override
            protected RoomDTO call() throws ServiceException {
                try {
                    LOGGER.info("Getting room with ID: " + roomId);
                    JSONObject payload = new JSONObject();
                    payload.put("id_stanza", roomId);
                    SocketReply reply = gameRulesMgr.sendAndReceive(new SocketCommand("get_room", payload));
                    if(reply.isError()) {
                        LOGGER.severe("Get room failed: " + reply.getPayload());
                        if(reply.getResult().equals("ROOM_NOT_FOUND")) {
                            throw new ResourceNotFoundException("Room not found");
                        }
                        else {
                            throw new ServiceException(reply.getPayload().toString());
                        }
                    }
                    LOGGER.info("Get room successful: "+reply.getPayload().toString());

                    JSONObject roomJson = (JSONObject) reply.getPayload();
                    return new RoomDTO(roomJson);

                } catch (RemoteServerException e) {
                    LOGGER.log(Level.SEVERE, "Error getting room", e);
                    throw new ServiceException("Cannot get the room.");
                }
            }
        };
    }
}
