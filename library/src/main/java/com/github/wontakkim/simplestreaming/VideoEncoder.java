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

    private final String TAG = "VIDEO_ENCODER";

    public final int DEFAULT_FRAME_RATE = 30;
    public final int DEFAULT_BIT_RATE = 1000000;

    public interface Callback {

        void onEncodedVideoFrame(ByteBuffer buffer, MediaCodec.BufferInfo info);
    }

    private int width, height;
    private int frameRate = DEFAULT_FRAME_RATE;
    private int bitrate = DEFAULT_BIT_RATE;

    private Callback callback;

    public VideoEncoder(int width, int height) {
        this.width = width;
        this.height = height;
    }

    public void setFrameRate(int frameRate) {
        this.frameRate = frameRate;
    }

    public void setBitrate(int bitrate) {
        this.bitrate = bitrate;
    }

    @Override
    protected MediaFormat buildMediaFormat() {
        MediaFormat format = MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC, width, height);
        format.setInteger(KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
        format.setInteger(KEY_I_FRAME_INTERVAL, 1);
        format.setInteger(KEY_FRAME_RATE, frameRate);
        format.setInteger(KEY_BIT_RATE, bitrate);
        format.setInteger(KEY_MAX_INPUT_SIZE, 0);
        return format;
    }

    @Override
    protected MediaCodec buildMediaCodec() throws IOException {
        return MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_VIDEO_AVC);
    }

    @Override
    protected void onEncodedFrame(ByteBuffer buffer, MediaCodec.BufferInfo bufferInfo) {
        if (callback != null)
            callback.onEncodedVideoFrame(buffer, bufferInfo);
    }

    public void setCallback(Callback callback) {
        this.callback = callback;
    }
}
