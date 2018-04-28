#include <jni.h>
#include <libyuv.h>

#define LIBYUV_ARRAY_ELEMS(a)  (sizeof(a) / sizeof(a[0]))

using namespace libyuv;

struct YuvFrame {
    int width;
    int height;
    uint8_t *data;
    uint8_t *y;
    uint8_t *u;
    uint8_t *v;
};

static JavaVM *jvm;
static JNIEnv *jenv;

static const int SRC_COLOR_FMT = FOURCC_RGBA;
static const int DST_COLOR_FMT = FOURCC_NV12;

static struct YuvFrame i420_rotated_frame;
static struct YuvFrame i420_scaled_frame;
static struct YuvFrame nv12_frame;

static bool convert_to_i420(uint8_t *src_frame, jint src_width, jint src_height,
                            jboolean need_flip, jint rotate_degree, int format) {
    int y_size = src_width * src_height;

    if (rotate_degree % 180 == 0) {
        if (i420_rotated_frame.width != src_width || i420_rotated_frame.height != src_height) {
            free(i420_rotated_frame.data);
            i420_rotated_frame.width = src_width;
            i420_rotated_frame.height = src_height;
            i420_rotated_frame.data = (uint8_t *) malloc(y_size * 4 / 2);
            i420_rotated_frame.y = i420_rotated_frame.data;
            i420_rotated_frame.u = i420_rotated_frame.y + y_size;
            i420_rotated_frame.v = i420_rotated_frame.u + y_size / 4;
        }
    } else {
        if (i420_rotated_frame.width != src_height || i420_rotated_frame.height != src_width) {
            free(i420_rotated_frame.data);
            i420_rotated_frame.width = src_height;
            i420_rotated_frame.height = src_width;
            i420_rotated_frame.data = (uint8_t *) malloc(y_size * 4 / 2);
            i420_rotated_frame.y = i420_rotated_frame.data;
            i420_rotated_frame.u = i420_rotated_frame.y + y_size;
            i420_rotated_frame.v = i420_rotated_frame.u + y_size / 4;
        }
    }

    jint ret = ConvertToI420(src_frame, y_size,
                             i420_rotated_frame.y, i420_rotated_frame.width,
                             i420_rotated_frame.u, i420_rotated_frame.width / 2,
                             i420_rotated_frame.v, i420_rotated_frame.width / 2,
                             0, 0,
                             src_width, src_height,
                             src_width, src_height,
                             (RotationMode) rotate_degree, format);
    if (ret < 0) {
        return false;
    }

    ret = I420Scale(i420_rotated_frame.y, i420_rotated_frame.width,
                    i420_rotated_frame.u, i420_rotated_frame.width / 2,
                    i420_rotated_frame.v, i420_rotated_frame.width / 2,
                    need_flip ? -i420_rotated_frame.width : i420_rotated_frame.width,
                    i420_rotated_frame.height,
                    i420_scaled_frame.y, i420_scaled_frame.width,
                    i420_scaled_frame.u, i420_scaled_frame.width / 2,
                    i420_scaled_frame.v, i420_scaled_frame.width / 2,
                    i420_scaled_frame.width, i420_scaled_frame.height,
                    kFilterNone);

    if (ret < 0) {
        return false;
    }

    return true;
}

static bool convert_to_i420_with_crop_scale(uint8_t *src_frame, jint src_width, jint src_height,
                                            jint crop_x, jint crop_y, jint crop_width,
                                            jint crop_height,
                                            jboolean need_flip, jint rotate_degree, int format) {
    int y_size = src_width * src_height;

    if (rotate_degree % 180 == 0) {
        if (i420_rotated_frame.width != src_width || i420_rotated_frame.height != src_height) {
            free(i420_rotated_frame.data);
            i420_rotated_frame.data = (uint8_t *) malloc(y_size * 3 / 2);
            i420_rotated_frame.y = i420_rotated_frame.data;
            i420_rotated_frame.u = i420_rotated_frame.y + y_size;
            i420_rotated_frame.v = i420_rotated_frame.u + y_size / 4;
        }

        i420_rotated_frame.width = crop_width;
        i420_rotated_frame.height = crop_height;

    } else {
        if (i420_rotated_frame.width != src_height || i420_rotated_frame.height != src_width) {
            free(i420_rotated_frame.data);
            i420_rotated_frame.data = (uint8_t *) malloc(y_size * 3 / 2);
            i420_rotated_frame.y = i420_rotated_frame.data;
            i420_rotated_frame.u = i420_rotated_frame.y + y_size;
            i420_rotated_frame.v = i420_rotated_frame.u + y_size / 4;
        }

        i420_rotated_frame.width = crop_height;
        i420_rotated_frame.height = crop_width;
    }

    jint ret = ConvertToI420(src_frame, y_size,
                             i420_rotated_frame.y, i420_rotated_frame.width,
                             i420_rotated_frame.u, i420_rotated_frame.width / 2,
                             i420_rotated_frame.v, i420_rotated_frame.width / 2,
                             crop_x, crop_y,
                             src_width, need_flip ? -src_height : src_height,
                             crop_width, crop_height,
                             (RotationMode) rotate_degree, format);
    if (ret < 0) {
        return false;
    }
//need_flip ? -i420_rotated_frame.width : i420_rotated_frame.width
    ret = I420Scale(i420_rotated_frame.y, i420_rotated_frame.width,
                    i420_rotated_frame.u, i420_rotated_frame.width / 2,
                    i420_rotated_frame.v, i420_rotated_frame.width / 2,
                    i420_rotated_frame.width, i420_rotated_frame.height,
                    i420_scaled_frame.y, i420_scaled_frame.width,
                    i420_scaled_frame.u, i420_scaled_frame.width / 2,
                    i420_scaled_frame.v, i420_scaled_frame.width / 2,
                    i420_scaled_frame.width, i420_scaled_frame.height,
                    kFilterNone);

    if (ret < 0) {
        return false;
    }

    return true;
}

static void
libyuv_setResolution(JNIEnv *env, jobject thiz, jint out_width, jint out_height) {
    int y_size = out_width * out_height;

    if (i420_scaled_frame.width != out_width || i420_scaled_frame.height != out_height) {
        free(i420_scaled_frame.data);
        i420_scaled_frame.width = out_width;
        i420_scaled_frame.height = out_height;
        i420_scaled_frame.data = (uint8_t *) malloc(y_size * 3 / 2);
        i420_scaled_frame.y = i420_scaled_frame.data;
        i420_scaled_frame.u = i420_scaled_frame.y + y_size;
        i420_scaled_frame.v = i420_scaled_frame.u + y_size / 4;
    }

    if (nv12_frame.width != out_width || nv12_frame.height != out_height) {
        free(nv12_frame.data);
        nv12_frame.width = out_width;
        nv12_frame.height = out_height;
        nv12_frame.data = (uint8_t *) malloc(y_size * 3 / 2);
        nv12_frame.y = nv12_frame.data;
        nv12_frame.u = nv12_frame.y + y_size;
        nv12_frame.v = nv12_frame.u + y_size / 4;
    }
}

static void
libyuv_release(JNIEnv *env, jobject thiz) {
    i420_rotated_frame.width = 0;
    i420_rotated_frame.height = 0;
    free(i420_rotated_frame.data);

    i420_scaled_frame.width = 0;
    i420_scaled_frame.height = 0;
    free(i420_scaled_frame.data);

    nv12_frame.width = 0;
    nv12_frame.height = 0;
    free(nv12_frame.data);
}

// For COLOR_FormatYUV420Planar
static jbyteArray libyuv_RGBAToI420(JNIEnv *env, jobject thiz, jbyteArray frame, jint src_width,
                                    jint src_height, jboolean need_flip, jint rotate_degree) {
    jbyte *rgba_frame = env->GetByteArrayElements(frame, NULL);

    if (!convert_to_i420((uint8_t *) rgba_frame, src_width, src_height, need_flip, rotate_degree,
                         FOURCC_RGBA)) {
        return NULL;
    }

    int y_size = i420_scaled_frame.width * i420_scaled_frame.height;
    jbyteArray i420Frame = env->NewByteArray(y_size * 3 / 2);
    env->SetByteArrayRegion(i420Frame, 0, y_size * 3 / 2, (jbyte *) i420_scaled_frame.data);

    env->ReleaseByteArrayElements(frame, rgba_frame, JNI_ABORT);
    return i420Frame;
}

static jbyteArray
libyuv_NV21ToNV12Scaled(JNIEnv *env, jobject thiz, jbyteArray frame, jint src_width,
                        jint src_height, jboolean need_flip, jint rotate_degree,
                        jint crop_x, jint crop_y, jint crop_width, jint crop_height) {
    jbyte *rgba_frame = env->GetByteArrayElements(frame, NULL);

    if (!convert_to_i420_with_crop_scale((uint8_t *) rgba_frame, src_width, src_height,
                                         crop_x, crop_y, crop_width, crop_height,
                                         need_flip, rotate_degree, FOURCC_NV21)) {
        return NULL;
    }

    int ret = ConvertFromI420(i420_scaled_frame.y, i420_scaled_frame.width,
                              i420_scaled_frame.u, i420_scaled_frame.width / 2,
                              i420_scaled_frame.v, i420_scaled_frame.width / 2,
                              nv12_frame.data, nv12_frame.width,
                              nv12_frame.width, nv12_frame.height,
                              FOURCC_NV12);
    if (ret < 0) {
        return NULL;
    }

    int y_size = nv12_frame.width * nv12_frame.height;
    jbyteArray nv12Frame = env->NewByteArray(y_size * 3 / 2);
    env->SetByteArrayRegion(nv12Frame, 0, y_size * 3 / 2, (jbyte *) nv12_frame.data);

    env->ReleaseByteArrayElements(frame, rgba_frame, JNI_ABORT);
    return nv12Frame;
}

static jbyteArray
libyuv_NV21ToI420Scaled(JNIEnv *env, jobject thiz, jbyteArray frame, jint src_width,
                        jint src_height, jboolean need_flip, jint rotate_degree,
                        jint crop_x, jint crop_y, jint crop_width, jint crop_height) {
    jbyte *argb_frame = env->GetByteArrayElements(frame, NULL);

    if (!convert_to_i420_with_crop_scale((uint8_t *) argb_frame, src_width, src_height,
                                         crop_x, crop_y, crop_width, crop_height,
                                         need_flip, rotate_degree, FOURCC_NV21)) {
        return NULL;
    }

    int y_size = i420_scaled_frame.width * i420_scaled_frame.height;
    jbyteArray i420Frame = env->NewByteArray(y_size * 3 / 2);
    env->SetByteArrayRegion(i420Frame, 0, y_size * 3 / 2, (jbyte *) i420_scaled_frame.data);

    env->ReleaseByteArrayElements(frame, argb_frame, JNI_ABORT);
    return i420Frame;
}

// For Bitmap.getPixels() ARGB_8888
static jbyteArray libyuv_ARGBToI420(JNIEnv *env, jobject thiz, jintArray frame, jint src_width,
                                    jint src_height, jboolean need_flip, jint rotate_degree) {
    jint *argb_frame = env->GetIntArrayElements(frame, NULL);

    if (!convert_to_i420((uint8_t *) argb_frame, src_width, src_height, need_flip, rotate_degree,
                         FOURCC_ARGB)) {
        return NULL;
    }

    int y_size = i420_scaled_frame.width * i420_scaled_frame.height;
    jbyteArray i420Frame = env->NewByteArray(y_size * 3 / 2);
    env->SetByteArrayRegion(i420Frame, 0, y_size * 3 / 2, (jbyte *) i420_scaled_frame.data);

    env->ReleaseIntArrayElements(frame, argb_frame, JNI_ABORT);
    return i420Frame;
}

// For Bitmap.getPixels() ARGB_8888
static jbyteArray
libyuv_ARGBToI420Scaled(JNIEnv *env, jobject thiz, jintArray frame, jint src_width,
                        jint src_height, jboolean need_flip, jint rotate_degree,
                        jint crop_x, jint crop_y, jint crop_width, jint crop_height) {
    jint *argb_frame = env->GetIntArrayElements(frame, NULL);

    if (!convert_to_i420_with_crop_scale((uint8_t *) argb_frame, src_width, src_height,
                                         crop_x, crop_y, crop_width, crop_height,
                                         need_flip, rotate_degree, FOURCC_ARGB)) {
        return NULL;
    }

    int y_size = i420_scaled_frame.width * i420_scaled_frame.height;
    jbyteArray i420Frame = env->NewByteArray(y_size * 3 / 2);
    env->SetByteArrayRegion(i420Frame, 0, y_size * 3 / 2, (jbyte *) i420_scaled_frame.data);

    env->ReleaseIntArrayElements(frame, argb_frame, JNI_ABORT);
    return i420Frame;
}

// For COLOR_FormatYUV420SemiPlanar
static jbyteArray libyuv_RGBAToNV12(JNIEnv *env, jobject thiz, jbyteArray frame, jint src_width,
                                    jint src_height, jboolean need_flip, jint rotate_degree) {
    jbyte *rgba_frame = env->GetByteArrayElements(frame, NULL);

    if (!convert_to_i420((uint8_t *) rgba_frame, src_width, src_height, need_flip, rotate_degree,
                         FOURCC_RGBA)) {
        return NULL;
    }

    int ret = ConvertFromI420(i420_scaled_frame.y, i420_scaled_frame.width,
                              i420_scaled_frame.u, i420_scaled_frame.width / 2,
                              i420_scaled_frame.v, i420_scaled_frame.width / 2,
                              nv12_frame.data, nv12_frame.width,
                              nv12_frame.width, nv12_frame.height,
                              DST_COLOR_FMT);
    if (ret < 0) {
        return NULL;
    }

    int y_size = nv12_frame.width * nv12_frame.height;
    jbyteArray nv12Frame = env->NewByteArray(y_size * 3 / 2);
    env->SetByteArrayRegion(nv12Frame, 0, y_size * 3 / 2, (jbyte *) nv12_frame.data);

    env->ReleaseByteArrayElements(frame, rgba_frame, JNI_ABORT);
    return nv12Frame;
}

// For Bitmap.getPixels() ARGB_8888
static jbyteArray libyuv_ARGBToNV12(JNIEnv *env, jobject thiz, jintArray frame, jint src_width,
                                    jint src_height, jboolean need_flip, jint rotate_degree) {
    jint *argb_frame = env->GetIntArrayElements(frame, NULL);

    if (!convert_to_i420((uint8_t *) argb_frame, src_width, src_height, need_flip, rotate_degree,
                         FOURCC_ARGB)) {
        return NULL;
    }

    int ret = ConvertFromI420(i420_scaled_frame.y, i420_scaled_frame.width,
                              i420_scaled_frame.u, i420_scaled_frame.width / 2,
                              i420_scaled_frame.v, i420_scaled_frame.width / 2,
                              nv12_frame.data, nv12_frame.width,
                              nv12_frame.width, nv12_frame.height,
                              DST_COLOR_FMT);
    if (ret < 0) {
        return NULL;
    }

    int y_size = nv12_frame.width * nv12_frame.height;
    jbyteArray nv12Frame = env->NewByteArray(y_size * 3 / 2);
    env->SetByteArrayRegion(nv12Frame, 0, y_size * 3 / 2, (jbyte *) nv12_frame.data);

    env->ReleaseIntArrayElements(frame, argb_frame, JNI_ABORT);
    return nv12Frame;
}

// For Bitmap.getPixels() ARGB_8888
static jbyteArray
libyuv_ARGBToNV12Scaled(JNIEnv *env, jobject thiz, jintArray frame, jint src_width,
                        jint src_height, jboolean need_flip, jint rotate_degree,
                        jint crop_x, jint crop_y, jint crop_width, jint crop_height) {
    jint *argb_frame = env->GetIntArrayElements(frame, NULL);

    if (!convert_to_i420_with_crop_scale((uint8_t *) argb_frame, src_width, src_height,
                                         crop_x, crop_y, crop_width, crop_height,
                                         need_flip, rotate_degree, FOURCC_ARGB)) {
        return NULL;
    }

    int ret = ConvertFromI420(i420_scaled_frame.y, i420_scaled_frame.width,
                              i420_scaled_frame.u, i420_scaled_frame.width / 2,
                              i420_scaled_frame.v, i420_scaled_frame.width / 2,
                              nv12_frame.data, nv12_frame.width,
                              nv12_frame.width, nv12_frame.height,
                              DST_COLOR_FMT);
    if (ret < 0) {
        return NULL;
    }

    int y_size = nv12_frame.width * nv12_frame.height;
    jbyteArray nv12Frame = env->NewByteArray(y_size * 3 / 2);
    env->SetByteArrayRegion(nv12Frame, 0, y_size * 3 / 2, (jbyte *) nv12_frame.data);

    env->ReleaseIntArrayElements(frame, argb_frame, JNI_ABORT);
    return nv12Frame;
}

static JNINativeMethod libyuv_methods[] = {
        {"setResolution",        "(II)V",                 (void *) libyuv_setResolution},
        {"release",              "()V",                   (void *) libyuv_release},
        {"RGBAToI420",           "([BIIZI)[B",            (void *) libyuv_RGBAToI420},
        {"RGBAToNV12",           "([BIIZI)[B",            (void *) libyuv_RGBAToNV12},
        {"ARGBToI420Scaled",     "([IIIZIIIII)[B",        (void *) libyuv_ARGBToI420Scaled},
        {"ARGBToNV12Scaled",     "([IIIZIIIII)[B",        (void *) libyuv_ARGBToNV12Scaled},
        {"ARGBToI420",           "([IIIZI)[B",            (void *) libyuv_ARGBToI420},
        {"ARGBToNV12",           "([IIIZI)[B",            (void *) libyuv_ARGBToNV12},
        {"NV21ToNV12Scaled",     "([BIIZIIIII)[B",        (void *) libyuv_NV21ToNV12Scaled},
        {"NV21ToI420Scaled",     "([BIIZIIIII)[B",        (void *) libyuv_NV21ToI420Scaled},
};

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    jvm = vm;

    if (jvm->GetEnv((void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass clz = jenv->FindClass("com/github/wontakkim/simplestreaming/vuv/YuvI420Converter");
    if (clz == NULL) {
        return JNI_ERR;
    }

    if (jenv->RegisterNatives(clz, libyuv_methods, LIBYUV_ARRAY_ELEMS(libyuv_methods))) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}