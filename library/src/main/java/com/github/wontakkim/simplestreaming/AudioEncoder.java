package com.github.wontakkim.simplestreaming;

import android.media.MediaCodec;
import android.media.MediaFormat;

import java.io.IOException;
import java.nio.ByteBuffer;

import static android.media.AudioFormat.CHANNEL_IN_DEFAULT;
import static android.media.MediaFormat.KEY_BIT_RATE;
import static android.media.MediaFormat.KEY_MAX_INPUT_SIZE;

public class AudioEncoder extends MediaEncoder {

    public interface Callback {

        void onEncodedAudioFrame(ByteBuffer buffer, MediaCodec.BufferInfo bufferInfo);
    }

    public static class Builder {

        public final int DEFAULT_SAMPLE_RATE = 44100;
        public final int DEFAULT_BIT_RATE = 96000;

        private int channelCount = CHANNEL_IN_DEFAULT;
        private int sampleRate = DEFAULT_SAMPLE_RATE;
        private int bitrate = DEFAULT_BIT_RATE;

        public Builder setChannelCount(int channelCount) {
            this.channelCount = channelCount;
            return this;
        }

        public Builder setSampleRate(int sampleRate) {
            this.sampleRate = sampleRate;
            return this;
        }

        public Builder setBitrate(int bitrate) {
            this.bitrate = bitrate;
            return this;
        }

        public AudioEncoder build() {
            return new AudioEncoder(channelCount, sampleRate, bitrate);
        }
    }

    private int channelCount;
    private int sampleRate;
    private int bitrate;

    private Callback callback;

    private AudioEncoder(int channelCount, int sampleRate, int bitrate) {
        this.channelCount = channelCount;
        this.sampleRate = sampleRate;
        this.bitrate = bitrate;
    }

    @Override
    protected MediaFormat buildMediaFormat() {
        MediaFormat format = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, sampleRate, channelCount);
        format.setInteger(KEY_MAX_INPUT_SIZE, 0);
        format.setInteger(KEY_BIT_RATE, bitrate);
        return format;
    }

    @Override
    protected MediaCodec buildMediaCodec() throws IOException {
        return MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
    }

    @Override
    protected void onEncodedFrame(ByteBuffer buffer, MediaCodec.BufferInfo bufferInfo) {
        if (callback != null) {
            callback.onEncodedAudioFrame(buffer, bufferInfo);
        }
    }


    public void setCallback(Callback callback) {
        this.callback = callback;
    }
}
