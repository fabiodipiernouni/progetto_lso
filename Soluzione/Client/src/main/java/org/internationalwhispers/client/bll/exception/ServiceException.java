package org.internationalwhispers.client.bll.exception;

public class ServiceException extends Exception {

    public ServiceException(String errorMessage) {
        super(errorMessage);
    }
    public ServiceException() {
        super();
    }

}
