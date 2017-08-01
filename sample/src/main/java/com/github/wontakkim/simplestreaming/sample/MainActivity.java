package com.github.wontakkim.simplestreaming.sample;

import android.Manifest;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.WindowManager;

import com.tbruyelle.rxpermissions2.RxPermissions;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MainActivity extends AppCompatActivity {

    @BindView(R.id.camera_preview)
    CameraPreview preview;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);

        ButterKnife.bind(this);

        startCamera();
    }

    private void startCamera() {
        preview.setPreviewResolution(640, 360);

        RxPermissions permissions = new RxPermissions(this);
        permissions.request(Manifest.permission.CAMERA)
                .subscribe(isGranted -> {
                    if (isGranted) {
                        preview.startCamera();
                    }
                });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        preview.stopCamera();
    }
}
