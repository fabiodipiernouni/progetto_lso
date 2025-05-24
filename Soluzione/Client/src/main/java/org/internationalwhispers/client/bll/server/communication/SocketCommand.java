package org.internationalwhispers.client.bll.server.communication;
import lombok.Getter;
import org.json.JSONObject;

public class SocketCommand {
    @Getter
    private String method;
    @Getter
    private JSONObject payload;

    private String validateMethod(String method) {
        if (method == null || method.isEmpty()) {
            throw new IllegalArgumentException("Method cannot be null or empty");
        }
        if (method.length() > 15) {
            throw new IllegalArgumentException("Method string is longer than 15 characters");
        }
        return String.format("%-15s", method);
    }

    private void validatePayload(JSONObject payload) {
        if(payload != null && payload.toString().isEmpty()) {
            throw new IllegalArgumentException("Payload cannot be empty");
        }
    }

    public SocketCommand(String string) {
        // split in two strings, first 15 characters are the method, the rest is the payload
        this.method = validateMethod(string.substring(0, 15));
        JSONObject payload = new JSONObject(string.substring(15));
        validatePayload(payload);
        this.payload = payload;
    }

    public SocketCommand(String method, JSONObject payload) {
        this.method = validateMethod(method);
        validatePayload(payload);
        this.payload = payload;
    }

    @Override
    public String toString() {
        if (payload == null) {
            return method;
        }

        return method + payload;
    }

}
