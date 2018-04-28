package com.github.wontakkim.simplestreaming.yuv;

public class YuvI420Converter {

    public YuvI420Converter(int outWidth, int outHeight) {
        setResolution(outWidth, outHeight);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        release();
    }

    private native void setResolution(int outWidth, int outHeight);
    private native void release();

    public native byte[] RGBAToI420(byte[] frame, int width, int height, boolean flip, int rotate);
    public native byte[] RGBAToNV12(byte[] frame, int width, int height, boolean flip, int rotate);
    public native byte[] ARGBToI420Scaled(int[] frame, int width, int height, boolean flip, int rotate, int crop_x, int crop_y, int crop_width, int crop_height);
    public native byte[] ARGBToNV12Scaled(int[] frame, int width, int height, boolean flip, int rotate, int crop_x, int crop_y, int crop_width, int crop_height);
    public native byte[] ARGBToI420(int[] frame, int width, int height, boolean flip, int rotate);
    public native byte[] ARGBToNV12(int[] frame, int width, int height, boolean flip, int rotate);
    public native byte[] NV21ToNV12Scaled(byte[] frame, int width, int height, boolean flip, int rotate, int crop_x, int crop_y, int crop_width, int crop_height);
    public native byte[] NV21ToI420Scaled(byte[] frame, int width, int height, boolean flip, int rotate, int crop_x, int crop_y, int crop_width, int crop_height);

    static {
        System.loadLibrary("yuv");
    }
}
