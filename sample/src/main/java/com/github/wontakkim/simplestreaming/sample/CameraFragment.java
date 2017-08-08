package com.github.wontakkim.simplestreaming.sample;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class CameraFragment extends Fragment {

    @BindView(R.id.camera_preview)
    CameraPreview preview;

    private Unbinder unbinder;

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
        preview.setPreviewResolution(640, 360);
        return view;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }
}
