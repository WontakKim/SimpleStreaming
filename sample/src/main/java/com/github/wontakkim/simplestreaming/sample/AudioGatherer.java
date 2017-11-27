package com.github.wontakkim.simplestreaming.sample;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.support.annotation.WorkerThread;
import android.util.Log;

import java.util.concurrent.atomic.AtomicBoolean;

public class AudioGatherer {

    private static final String TAG = AudioGatherer.class.getSimpleName();

    public static final int DEFAULT_SAMPLE_RATE = 44100;
    public static final int SAMPLES_PER_FRAME = 2048;       // AAC, bytes/frame/channel

    public interface Callback {

        void onAudioStarted(long milliseconds);

        void onAudioStopped();

        @WorkerThread
        void onAudioData(byte[] data);
    }

    private int sampleRate = DEFAULT_SAMPLE_RATE;
    private int samplesPerFrame = SAMPLES_PER_FRAME;

    private Thread workThread;
    private Callback callback;
    private AtomicBoolean loop = new AtomicBoolean(false);
    private AtomicBoolean audioStarted = new AtomicBoolean(false);
    private long audioStartTime;

    public AudioGatherer() {
        this(DEFAULT_SAMPLE_RATE, DEFAULT_SAMPLE_RATE);
    }

    public AudioGatherer(int sampleRate, int samplesPerFrame) {
        this.sampleRate = sampleRate;
        this.samplesPerFrame = samplesPerFrame;
    }

    public synchronized void start(Callback callback) {
        this.callback = callback;

        if (!audioStarted.get()) {
            if (loop.compareAndSet(false, true)) {
                initializeWorkThread();
                workThread.start();
            }
        } else {
            callback.onAudioStarted(audioStartTime);
        }
    }

    public synchronized void stop() {
        if (loop.get()) {
            releaseWorkThread();
        }
    }

    private void initializeWorkThread() {
        workThread = new Thread(new AudioRecordRunnable());
    }

    private void releaseWorkThread() {
        if (workThread != null && loop.compareAndSet(true, false)) {
            workThread.interrupt();
            workThread = null;
        }
    }

    private class AudioRecordRunnable implements Runnable {

        private AudioRecord audioRecord;

        @Override
        public void run() {
            Log.d(TAG, "Audio gatherer thread name : " + Thread.currentThread().getName());

            android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);

            int bufferReadResult;
            byte[] buffer;

            audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, sampleRate, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT, samplesPerFrame);
            buffer = new byte[samplesPerFrame];

            if (audioRecord.getState() == AudioRecord.STATE_INITIALIZED)
                audioRecord.startRecording();

            Log.d(TAG, "Audio start recording");

            while (loop.get() && !Thread.interrupted() && audioRecord.getRecordingState() != AudioRecord.RECORDSTATE_RECORDING) {

            }

            audioStartTime = System.currentTimeMillis();
            audioStarted.set(true);

            if (callback != null) {
                callback.onAudioStarted(audioStartTime);
            }

            while (loop.get() && !Thread.interrupted()) {
                bufferReadResult = audioRecord.read(buffer, 0, samplesPerFrame);
                if (bufferReadResult > 0) {
                    if (callback != null) {
                        callback.onAudioData(buffer);
                    }
                }
            }

            if (audioRecord != null) {
                if (audioRecord.getRecordingState() == AudioRecord.RECORDSTATE_RECORDING)
                    audioRecord.stop();

                if (audioRecord.getState() == AudioRecord.STATE_INITIALIZED)
                    audioRecord.release();

                audioRecord = null;

                if (callback != null) {
                    callback.onAudioStopped();
                }

                Log.d(TAG, "Audio released");
            }

            audioStarted.set(false);
        }
    }
}
