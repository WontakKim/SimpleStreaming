package com.github.wontakkim.simplestreaming;

public class AVCPublisher {

    public native long initialize(String url, int timeout);

    public native int release(long cptr);

    public native int connect(long cptr);

    public native int sendVideoData(long cptr, byte[] data, int length, long timestamp);

    public native int sendAacData(long cptr, byte[] data, int length, long timestamp);

    static {
        System.loadLibrary("publisher");
    }
}
