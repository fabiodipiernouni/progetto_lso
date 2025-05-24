package org.internationalwhispers.client.entity;

import lombok.Getter;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

@Getter
public class RoomDTO {

    public enum GameDirection {
        CLOCKWISE,
        COUNTERCLOCKWISE;

        public String getMapping() {
            return switch (this) {
                case CLOCKWISE -> "ORARIO";
                case COUNTERCLOCKWISE -> "ANTIORARIO";
            };
        }

        public static GameDirection fromString(String name) {
            return switch (name.toUpperCase()) {
                case "ORARIO", "CLOCKWISE" -> CLOCKWISE;
                case "ANTIORARIO", "COUNTERCLOCKWISE" -> COUNTERCLOCKWISE;
                default -> throw new IllegalArgumentException("Invalid game direction: " + name);
            };
        }
    }

    public enum GameStatus {
        WAITING_PLAYERS,
        GAME_IN_PROGRESS,
        GAME_FINISHED;

        public String getMapping() {
            return switch (this) {
                case WAITING_PLAYERS -> "IN_ATTESA_DI_GIOCATORI";
                case GAME_IN_PROGRESS -> "GIOCO_IN_CORSO";
                case GAME_FINISHED -> "GIOCO_TERMINATO";
            };
        }

        public static GameStatus fromString(String name) {
            return switch (name.toUpperCase()) {
                case "IN_ATTESA_DI_GIOCATORI", "WAITING_PLAYERS" -> WAITING_PLAYERS;
                case "GIOCO_IN_CORSO", "GAME_IN_PROGRESS" -> GAME_IN_PROGRESS;
                case "GIOCO_TERMINATO", "GAME_FINISHED" -> GAME_FINISHED;
                default -> throw new IllegalArgumentException("Invalid game status: " + name);
            };
        }
    }

    public static final int MAX_NAME_LENGTH = 127;

    private int id;
    private UserDTO host;
    private String name;
    private GameDirection gameDirection;
    private GameStatus status;
    private int playersNumber=0;
    private UserDTO currentPlayer=null;
    private List<GiocatoreDTO> players = new ArrayList<>();
    private List<UserDTO> queue = new ArrayList<>();

    public RoomDTO(JSONObject roomJson) {
        this.id = roomJson.getInt("id_stanza");
        this.host = new UserDTO(roomJson.getJSONObject("utente_host"));
        this.name = roomJson.getString("nome");
        this.gameDirection = GameDirection.fromString(roomJson.getString("ordine_gioco"));
        this.status = GameStatus.fromString(roomJson.getString("stato_gioco"));
        this.playersNumber = roomJson.getInt("numero_giocatori");
        if(!roomJson.isNull("giocatore_corrente"))
            this.currentPlayer = new UserDTO(roomJson.getJSONObject("giocatore_corrente"));

        for(Object obj : roomJson.getJSONArray("giocatori")) {
            JSONObject giocatoreJson = (JSONObject) obj;

            if(giocatoreJson.isNull("utente")) {
                this.players.add(null);
            }
            else {
                this.players.add(new GiocatoreDTO(giocatoreJson));
            }
        }

        for(Object obj : roomJson.getJSONArray("coda")) {
            JSONObject uteneJson = (JSONObject) obj;
            this.queue.add(new UserDTO(uteneJson));
        }

    }

}
