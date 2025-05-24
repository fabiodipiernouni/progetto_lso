package org.internationalwhispers.client.bll.exception;

public class ResourceNotFoundException extends ServiceException {
    public ResourceNotFoundException(String errorMessage) {
        super(errorMessage);
    }

    public ResourceNotFoundException() {
        super();
    }

}
