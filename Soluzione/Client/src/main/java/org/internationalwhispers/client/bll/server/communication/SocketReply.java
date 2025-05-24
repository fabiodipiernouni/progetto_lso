package org.internationalwhispers.client.bll.server.communication;

import lombok.Getter;
import org.json.JSONObject;

public class SocketReply {
    @Getter
    private String result;
    private Object payload;

    public SocketReply(String message) {
        try {
            JSONObject json = new JSONObject(message);
            this.result = json.getString("esito");

            if (json.isNull("payload")) {
                this.payload = null;
            } else {
                this.payload = json.get("payload");
            }
        }
        catch (Exception e) {

            throw new IllegalArgumentException("Invalid communication format");
        }
    }

    public boolean isError() {
        return !result.equals("OK");
    }

    public Object getPayload() {
        return payload;
    }

}
