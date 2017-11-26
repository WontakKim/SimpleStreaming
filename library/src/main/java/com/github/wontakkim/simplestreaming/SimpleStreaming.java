package com.github.wontakkim.simplestreaming;

import android.graphics.Rect;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.concurrent.LinkedBlockingQueue;

import static android.content.res.Configuration.ORIENTATION_PORTRAIT;

public class SimpleStreaming implements VideoEncoder.Callback, AudioEncoder.Callback {

    private static final String TAG = "SIMPLE_STREAMING";

    private String url;

    private MediaCodecInfo mediaCodecInfo;
    private int videoColorFormat;

    private YuvI420Converter yuvI420Converter;

    private VideoEncoder videoEncoder;
    private AudioEncoder audioEncoder;

    private FlvMuxer flvMuxer;
    private boolean isPlaying;

    private int previewWidth, previewHeight;
    private int orientation = ORIENTATION_PORTRAIT;
    private boolean isFrontCamera = true;
    private double aspectRatio;

    private LinkedBlockingQueue<Runnable> runnables = new LinkedBlockingQueue();

    private Thread workThread;
    private boolean loop;

    public SimpleStreaming(int width, int height) {
        videoColorFormat = chooseVideoEncoder();

        previewWidth = width;
        previewHeight = height;
    }

    public void setUrl(String url) {
        this.url = url;
    }

    public void setFrontCamera(boolean isFront) {
        isFrontCamera = isFront;
    }

    public void setOrientation(int orientation) {
        this.orientation = orientation;
    }

    public void setAspectRatio(double ratio) {
        aspectRatio = ratio;
    }

    public void setAudioEncoder(AudioEncoder encoder) {
        audioEncoder = encoder;
    }

    public void prepare() {
        flvMuxer = new FlvMuxer();
        flvMuxer.prepare();

        Rect rc = getOutputRectangle();

        yuvI420Converter = new YuvI420Converter(rc.width(), rc.height());

        videoEncoder = new VideoEncoder(rc.width(), rc.height());
        audioEncoder = new AudioEncoder.Builder().build();

        videoEncoder.setCallback(this);
        audioEncoder.setCallback(this);

        workThread = new Thread("simple-streaming-thread") {
            @Override
            public void run() {
                while (loop && !Thread.interrupted()) {
                    try {
                        Runnable runnable = runnables.take();
                        runnable.run();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        };

        loop = true;
        workThread.start();

        Log.d(TAG, "Prepared SimpleStreaming.");
        Log.d(TAG, "\tV - output_width : " + rc.width() + "\toutput_height : " + rc.height());
    }

    public void start() {
        try {
            videoEncoder.start();
            audioEncoder.start();
        } catch (IOException e) {
            stop();
            return;
        }

        int ret = flvMuxer.start(url, 3000);
        if (ret < 0) {
            return;
        }

        isPlaying = true;
    }

    public void stop() {
        if (videoEncoder != null)
            videoEncoder.stop();

        if (audioEncoder != null)
            audioEncoder.stop();

        if (isPlaying) {
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    flvMuxer.release();
                    isPlaying = false;
                    loop = false;
                    workThread.interrupt();
                }
            };
            runnables.add(runnable);
        }
    }

    public void putVideoData(byte[] data) {
        if (!isPlaying)
            return;

        byte[] proceededData = processVideoData(data);
        videoEncoder.putData(proceededData);
    }

    @Override
    public void onEncodedVideoFrame(ByteBuffer buffer, MediaCodec.BufferInfo bufferInfo) {
        if (!isPlaying) {
            return;
        }

        final byte[] bytes = new byte[bufferInfo.size];
        buffer.get(bytes);

        try {
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    flvMuxer.writeVideoData(bytes, bytes.length, bufferInfo.presentationTimeUs / 1000L);
                }
            };
            runnables.put(runnable);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void putAudioData(byte[] data) {
        if (!isPlaying) {
            return;
        }

        audioEncoder.putData(data);
    }

    @Override
    public void onEncodedAudioFrame(ByteBuffer buffer, MediaCodec.BufferInfo bufferInfo) {
        if (!isPlaying) {
            return;
        }

        final byte[] bytes = new byte[bufferInfo.size];
        buffer.get(bytes);

        try {
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    flvMuxer.writeAudioData(bytes, bytes.length, bufferInfo.presentationTimeUs / 1000L);
                }
            };
            runnables.put(runnable);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private byte[] processVideoData(byte[] data) {
        boolean flip = false;
        boolean isPortrait = orientation == ORIENTATION_PORTRAIT;
        int portraitRotation = (isFrontCamera) ? 270 : 90;
        int rotate = isPortrait ? portraitRotation : 0;
        Rect rc = getCropRectangle();

        switch (videoColorFormat) {
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar:
                return yuvI420Converter.NV21ToI420Scaled(data, previewWidth, previewHeight, flip, rotate, rc.left, rc.top, rc.width(), rc.height());
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar:
                return yuvI420Converter.NV21ToNV12Scaled(data, previewWidth, previewHeight, flip, rotate, rc.left, rc.top, rc.width(), rc.height());
            default:
                throw new IllegalStateException("Unsupported color format!");
        }
    }

    // choose the right supported color format
    private int chooseVideoEncoder() {
        // Choose the encoder "video/avc":
        // 1. select default one when type matched.
        // 2. google avc is unusable.
        // 3. choose qcom avc.
        mediaCodecInfo = chooseVideoEncoder(null);
        // mediaCodecInfo = chooseVideoEncoder("google");
        // mediaCodecInfo = chooseVideoEncoder("qcom");

        int matchedColorFormat = 0;
        MediaCodecInfo.CodecCapabilities cc = mediaCodecInfo.getCapabilitiesForType("video/avc");
        for (int i = 0; i < cc.colorFormats.length; i++) {
            int cf = cc.colorFormats[i];
            Log.i(TAG, String.format("vencoder %s supports color fomart 0x%x(%d)", mediaCodecInfo.getName(), cf, cf));

            // choose YUV for h.264, prefer the bigger one.
            // corresponding to the color space transform in onPreviewFrame
            if (cf >= cc.COLOR_FormatYUV420Planar && cf <= cc.COLOR_FormatYUV420SemiPlanar) {
                if (cf > matchedColorFormat) {
                    matchedColorFormat = cf;
                }
            }
        }

        for (int i = 0; i < cc.profileLevels.length; i++) {
            MediaCodecInfo.CodecProfileLevel pl = cc.profileLevels[i];
            Log.i(TAG, String.format("vencoder %s support profile %d, level %d", mediaCodecInfo.getName(), pl.profile, pl.level));
        }

        Log.i(TAG, String.format("vencoder %s choose color format 0x%x(%d)", mediaCodecInfo.getName(), matchedColorFormat, matchedColorFormat));
        return matchedColorFormat;
    }

    private MediaCodecInfo chooseVideoEncoder(String name) {
        int codecCount = MediaCodecList.getCodecCount();
        for (int i = 0; i < codecCount; i++) {
            MediaCodecInfo mci = MediaCodecList.getCodecInfoAt(i);
            if (!mci.isEncoder()) {
                continue;
            }

            String[] types = mci.getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equalsIgnoreCase("video/avc")) {
                    Log.i(TAG, String.format("vencoder %s types: %s", mci.getName(), types[j]));
                    if (name == null) {
                        return mci;
                    }

                    if (mci.getName().contains(name)) {
                        return mci;
                    }
                }
            }
        }

        return null;
    }

    private Rect getOutputRectangle() {
        Rect rc = new Rect();

        int width = (previewWidth < previewHeight) ? previewWidth : previewHeight;
        int height = (previewWidth < previewHeight) ? previewHeight : previewWidth;

        int diff = 0;
        if (aspectRatio != 0)
            diff = width - ((int) (height / aspectRatio));

        int halfDiff = diff / 2;

        int croppedWidth = width - diff;
        int croppedHeight = height;

        if (orientation == ORIENTATION_PORTRAIT) {
            rc.set(halfDiff, 0, croppedWidth + halfDiff, croppedHeight);
        } else {
            rc.set(0, halfDiff, croppedHeight, croppedWidth + halfDiff);
        }

        return rc;
    }

    private Rect getCropRectangle() {
        Rect rc = new Rect();

        int diff = 0;
        if (aspectRatio != 0)
            diff = previewHeight - ((int) (previewWidth / aspectRatio));
        int halfDiff = diff / 2;

        int croppedWidth = previewWidth;
        int croppedHeight = previewHeight - diff;

        rc.set(0, halfDiff, croppedWidth, croppedHeight + halfDiff);
        return rc;
    }
}
