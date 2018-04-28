package com.github.wontakkim.simplestreaming.sample;

import android.hardware.Camera;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.design.widget.FloatingActionButton;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.github.wontakkim.simplestreaming.rtmp.RtmpPublisher;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class CameraFragment extends Fragment implements CameraPreview.PreviewCallback, AudioGatherer.Callback {

    @BindView(R.id.camera_preview)
    CameraPreview preview;

    @BindView(R.id.fab_play)
    FloatingActionButton playFab;

    private Unbinder unbinder;

    private RtmpPublisher streaming;
    private AudioGatherer audioGatherer;
    private boolean isPlaying = false;

    public static CameraFragment newInstance() {
        CameraFragment fragment = new CameraFragment();

        Bundle args = new Bundle();
        fragment.setArguments(args);

        return fragment;
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_camera, container, false);
        unbinder = ButterKnife.bind(this, view);

        preview.setPreviewResolution(640, 480);
        preview.setPreviewCallback(this);

        streaming = new RtmpPublisher(640, 480);
        streaming.setUrl("rtmp://127.0.0.1:1935/live/livestream");
        streaming.prepare();

        audioGatherer = new AudioGatherer();

        return view;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }

    @OnClick(R.id.fab_play)
    public void onPlayClick() {
        streaming.start();
        audioGatherer.start(this);
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        streaming.putVideoData(data);
    }

    @Override
    public void onAudioStarted(long milliseconds) {

    }

    @Override
    public void onAudioStopped() {

    }

    @Override
    public void onAudioData(byte[] data) {
        streaming.putAudioData(data);
    }
}
