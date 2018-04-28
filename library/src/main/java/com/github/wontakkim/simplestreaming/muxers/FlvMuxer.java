package com.github.wontakkim.simplestreaming.muxers;

public class FlvMuxer {

    private long pointer;

    public void prepare() {
        pointer = initialize();
    }

    public void release() {
        release(pointer);
        pointer = 0;
    }

    public int start(String url, int timeout) throws IllegalStateException {
        if (pointer == 0)
            throw new IllegalStateException("FlvMuxer is not ready.");

        return start(pointer, url, timeout);
    }

    public int stop() throws IllegalStateException {
        if (pointer == 0)
            throw new IllegalStateException("FlvMuxer is not ready.");

        return stop(pointer);
    }

    public int isPlaying() {
        if (pointer == 0)
            throw new IllegalStateException("FlvMuxer is not ready.");

        return isPlaying(pointer);
    }

    public int writeVideoData(byte[] data, int length, long timestamp) {
        if (pointer == 0)
            throw new IllegalStateException("FlvMuxer is not ready.");

        return writeVideoData(pointer, data, length, timestamp);
    }

    public int writeAudioData(byte[] data, int length, long timestamp) {
        if (pointer == 0)
            throw new IllegalStateException("FlvMuxer is not ready.");

        return writeAudioData(pointer, data, length, timestamp);
    }

    private native long initialize();
    private native int release(long pointer);
    private native int start(long pointer, String url, int timeout);
    private native int stop(long pointer);
    private native int isPlaying(long pointer);
    private native int writeVideoData(long pointer, byte[] data, int length, long timestamp);
    private native int writeAudioData(long pointer, byte[] data, int length, long timestamp);

    static {
        System.loadLibrary("simplestreaming");
    }
}