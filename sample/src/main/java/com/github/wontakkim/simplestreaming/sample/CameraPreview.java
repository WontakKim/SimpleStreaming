package com.github.wontakkim.simplestreaming.sample;

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.List;

import static android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT;

public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {

    private static final String TAG = "CAMERA_PREVIEW";

    private Camera camera;
    private int cameraType = CAMERA_FACING_FRONT;
    private int cameraId = -1;

    private int previewWidth;
    private int previewHeight;

    private int previewOrientation = Configuration.ORIENTATION_PORTRAIT;
    private int previewRotation = 90;

    public CameraPreview(Context context) {
        this(context, null);
    }

    public CameraPreview(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void surfaceCreated(SurfaceHolder holder) {
        try {
            camera.setPreviewDisplay(holder);
            camera.startPreview();
        } catch (IOException e) {
            Log.d(TAG, "Error setting camera preview: " + e.getMessage());
        }
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        camera.stopPreview();
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (holder.getSurface() == null){
            return;
        }

        try {
            camera.stopPreview();
        } catch (Exception e){

        }

        try {
            camera.setPreviewDisplay(holder);
            camera.startPreview();
        } catch (Exception e){
            Log.d(TAG, "Error starting camera preview: " + e.getMessage());
        }
    }

    public int[] setPreviewResolution(int width, int height) {
        camera = openCamera();

        previewWidth = width;
        previewHeight = height;

        Camera.Size rs = adaptPreviewResolution(camera.new Size(width, height));
        if (rs != null) {
            previewWidth = rs.width;
            previewHeight = rs.height;
        }

        camera.getParameters().setPreviewSize(previewWidth, previewHeight);
        return new int[] { previewWidth, previewHeight };
    }

    public void setPreviewOrientation(int orientation) {
        previewOrientation = orientation;
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        if (orientation == Configuration.ORIENTATION_PORTRAIT) {
            if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                previewRotation = info.orientation % 360;
                previewRotation = (360 - previewRotation) % 360;  // compensate the mirror
            } else {
                previewRotation = (info.orientation + 360) % 360;
            }
        } else if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
            if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                previewRotation = (info.orientation + 90) % 360;
                previewRotation = (360 - previewRotation) % 360;  // compensate the mirror
            } else {
                previewRotation = (info.orientation + 270) % 360;
            }
        }
    }

    private Camera.Size adaptPreviewResolution(Camera.Size resolution) {
        float diff = 100f;
        float xdy = (float) resolution.width / (float) resolution.height;
        Camera.Size best = null;

        for (Camera.Size size : camera.getParameters().getSupportedPreviewSizes()) {
            if (size.equals(resolution)) {
                return size;
            }

            float tmp = Math.abs(((float) size.width / (float) size.height) - xdy);
            if (tmp < diff) {
                diff = tmp;
                best = size;
            }
        }

        return best;
    }

    public boolean startCamera() {
        if (camera == null) {
            camera = openCamera();
            if (camera == null) {
                return false;
            }
        }

        Camera.Parameters params = camera.getParameters();
        params.setPictureSize(previewWidth, previewHeight);
        params.setPreviewSize(previewWidth, previewHeight);
        int[] range = adaptFpsRange(30, params.getSupportedPreviewFpsRange());
        params.setPreviewFpsRange(range[0], range[1]);
        params.setPreviewFormat(ImageFormat.NV21);
        params.setFlashMode(Camera.Parameters.FLASH_MODE_OFF);
        params.setWhiteBalance(Camera.Parameters.WHITE_BALANCE_AUTO);
        params.setSceneMode(Camera.Parameters.SCENE_MODE_AUTO);

        List<String> supportedFocusModes = params.getSupportedFocusModes();
        if (supportedFocusModes != null && !supportedFocusModes.isEmpty()) {
            if (supportedFocusModes.contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE)) {
                params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
            } else if (supportedFocusModes.contains(Camera.Parameters.FOCUS_MODE_AUTO)) {
                params.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
                camera.autoFocus(null);
            } else {
                params.setFocusMode(supportedFocusModes.get(0));
            }
        }

        camera.setParameters(params);
        camera.setDisplayOrientation(previewRotation);

        getHolder().addCallback(this);

        return true;
    }

    public void stopCamera() {
        if (camera != null) {
            camera.stopPreview();
            camera.release();
            camera = null;
        }
    }

    private int[] adaptFpsRange(int expectedFps, List<int[]> fpsRanges) {
        expectedFps *= 1000;
        int[] closestRange = fpsRanges.get(0);
        int measure = Math.abs(closestRange[0] - expectedFps) + Math.abs(closestRange[1] - expectedFps);
        for (int[] range : fpsRanges) {
            if (range[0] <= expectedFps && range[1] >= expectedFps) {
                int curMeasure = Math.abs(range[0] - expectedFps) + Math.abs(range[1] - expectedFps);
                if (curMeasure < measure) {
                    closestRange = range;
                    measure = curMeasure;
                }
            }
        }
        return closestRange;
    }

    private Camera openCamera() {
        Camera camera;

        if (cameraId < 0) {
            Camera.CameraInfo info = new Camera.CameraInfo();
            int cameraCount = Camera.getNumberOfCameras();

            for (int i=0; i<cameraCount; i++) {
                Camera.getCameraInfo(i, info);
                cameraId = i;

                if (info.facing == cameraType) {
                    break;
                }
            }
        }

        camera = Camera.open(cameraId);
        return camera;
    }
}