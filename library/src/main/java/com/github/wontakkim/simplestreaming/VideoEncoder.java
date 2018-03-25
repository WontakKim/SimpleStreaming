package com.github.wontakkim.simplestreaming;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;

import java.io.IOException;
import java.nio.ByteBuffer;

import static android.media.MediaFormat.KEY_BIT_RATE;
import static android.media.MediaFormat.KEY_COLOR_FORMAT;
import static android.media.MediaFormat.KEY_FRAME_RATE;
import static android.media.MediaFormat.KEY_I_FRAME_INTERVAL;
import static android.media.MediaFormat.KEY_MAX_INPUT_SIZE;

public class VideoEncoder extends MediaEncoder {

    public static final int DEFAULT_FRAME_RATE = 30;
    public static final int DEFAULT_BIT_RATE = 1000000;

    public interface Callback {

        void onEncodedVideoFrame(ByteBuffer buffer, MediaCodec.BufferInfo info);
    }

    private int width, height;
    private int frameRate;
    private int bitRate;

    private Callback callback;

    public VideoEncoder(int width, int height, int frameRate, int bitRate) {
        this.width = width;
        this.height = height;
        this.frameRate = frameRate;
        this.bitRate = bitRate;
    }

    @Override
    protected MediaFormat buildMediaFormat() {
        MediaFormat format = MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC, width, height);
        format.setInteger(KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
        format.setInteger(KEY_I_FRAME_INTERVAL, 1);
        format.setInteger(KEY_FRAME_RATE, frameRate);
        format.setInteger(KEY_BIT_RATE, bitRate);
        format.setInteger(KEY_MAX_INPUT_SIZE, 0);
        return format;
    }

    @Override
    protected MediaCodec buildMediaCodec() throws IOException {
        return MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_VIDEO_AVC);
    }

    @Override
    protected void onEncodedFrame(ByteBuffer buffer, MediaCodec.BufferInfo bufferInfo) {
        if (callback != null) {
            callback.onEncodedVideoFrame(buffer, bufferInfo);
        }
    }

    public void setCallback(Callback callback) {
        this.callback = callback;
    }
}
