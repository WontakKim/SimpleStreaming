package com.github.wontakkim.simplestreaming;

import android.media.MediaCodec;
import android.media.MediaFormat;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.concurrent.LinkedBlockingQueue;

public abstract class MediaEncoder {

    protected MediaCodec codec;

    private boolean loop;
    private Thread workThread;

    private LinkedBlockingQueue<byte[]> queue = new LinkedBlockingQueue();

    synchronized public void start() throws IOException {
        if (loop)
            return;

        codec = buildMediaCodec();

        workThread = new Thread(new EncodeRunnable());
        loop = true;
        workThread.start();
    }

    synchronized public void stop() {
        if (workThread != null) {
            workThread.interrupt();
            try {
                workThread.join();
            } catch (InterruptedException e) {
                workThread.interrupt();
            }
            loop = false;
            workThread = null;
        }
    }

    public void putData(byte[] data) {
        if (!loop)
            return;

        try {
            queue.put(data);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    protected abstract MediaFormat buildMediaFormat();

    protected abstract MediaCodec buildMediaCodec() throws IOException;

    protected abstract void onEncodedFrame(ByteBuffer buffer, final MediaCodec.BufferInfo bufferInfo);

    private class EncodeRunnable implements Runnable {

        @Override
        public void run() {
            MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();

            codec.configure(buildMediaFormat(), null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            codec.start();

            while (loop && !Thread.interrupted()) {
                try {
                    int inputIndex = codec.dequeueInputBuffer(-1);
                    if (inputIndex >= 0) {
                        byte[] data = queue.take();
                        ByteBuffer buffer = codec.getInputBuffer(inputIndex);
                        buffer.clear();
                        buffer.put(data, 0, data.length);

                        long pts = System.nanoTime() / 1000L;
                        codec.queueInputBuffer(inputIndex, 0, data.length, pts, 0);
                    }

                    int outputIndex = codec.dequeueOutputBuffer(bufferInfo, 0);
                    if (outputIndex >= 0) {
                        ByteBuffer buffer = codec.getOutputBuffer(outputIndex);
                        buffer.position(bufferInfo.offset);
                        buffer.limit(bufferInfo.offset + bufferInfo.size);
                        onEncodedFrame(buffer, bufferInfo);
                        codec.releaseOutputBuffer(outputIndex, false);
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    break;
                }
            }

            queue.clear();

            codec.stop();
            codec.release();
            codec = null;
        }
    }
}
