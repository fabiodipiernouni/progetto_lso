package org.internationalwhispers.client.presentation.helper;
import org.internationalwhispers.client.presentation.app.InternationalWhispersApplication;
import org.internationalwhispers.client.presentation.helper.exception.ResourceException;

import java.io.InputStream;
import java.net.URL;

public class ResourceLoader {

    private ResourceLoader() {}

    public static URL loadURL(String path) throws ResourceException {
        URL url = InternationalWhispersApplication.class.getResource("/org/internationalwhispers/client" + (path.startsWith("/") ? "" : "/") + path);
        if(url == null) {
            throw new ResourceException("Resource not found: " + path);
        }
        return url;
    }

    public static String load(String path) {
        return loadURL(path).toString();
    }

    public static InputStream loadStream(String name) throws ResourceException {
        InputStream stream = InternationalWhispersApplication.class.getResourceAsStream("/org/internationalwhispers/client" + (name.startsWith("/") ? "" : "/") + name);
        if(stream == null) {
            throw new ResourceException("Resource not found: " + name);
        }
        return stream;
    }

}
