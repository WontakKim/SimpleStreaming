package com.github.wontakkim.simplestreaming;

public class AVCPublisher {

    private long pointer;

    public void prepare() {
        prepare(0);
    }

    public void prepare(int timeout) {
        pointer = initialize(timeout);
    }

    public void release() {
        release(pointer);
        pointer = 0;
    }

    public int connect(String url) throws IllegalStateException {
        if (pointer == 0)
            throw new IllegalStateException("AVCPublisher is not ready.");

        return connect(pointer, url);
    }

    public int disconnect() throws IllegalStateException {
        if (pointer == 0)
            throw new IllegalStateException("AVCPublisher is not ready.");

        return disconnect(pointer);
    }

    public int isConnected() {
        if (pointer == 0)
            throw new IllegalStateException("AVCPublisher is not ready.");

        return isConnected(pointer);
    }

    public int sendVideoData(byte[] data, int length, long timestamp) {
        if (pointer == 0)
            throw new IllegalStateException("AVCPublisher is not ready.");

        return sendVideoData(pointer, data, length, timestamp);
    }

    public int sendAacData(byte[] data, int length, long timestamp) {
        if (pointer == 0)
            throw new IllegalStateException("AVCPublisher is not ready.");

        return sendAacData(pointer, data, length, timestamp);
    }

    private native long initialize(int timeout);
    private native int release(long pointer);
    private native int connect(long pointer, String url);
    private native int disconnect(long pointer);
    private native int isConnected(long pointer);
    private native int sendVideoData(long pointer, byte[] data, int length, long timestamp);
    private native int sendAacData(long pointer, byte[] data, int length, long timestamp);

    static {
        System.loadLibrary("publisher");
    }
}
