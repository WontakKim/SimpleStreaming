#include <jni.h>
#include <stdio.h>
#include "AVCPublisher.h"

#define LIBPUBLISHER_ARRAY_ELEMS(a)  (sizeof(a) / sizeof(a[0]))

static JavaVM *jvm;
static JNIEnv *jenv;

static jlong
libpublisher_initialize(JNIEnv *env, jobject thiz, jstring _url, jint timeout) {
    const char *url = env->GetStringUTFChars(_url, 0);
    char* url_copy = (char *) malloc(strlen(url) + 1);
    strcpy(url_copy, url);

    AVCPublisher *rtmp = new AVCPublisher();
    rtmp->initialize(url_copy, timeout);

    free(url_copy);
    env->ReleaseStringUTFChars(_url, url);
    return reinterpret_cast<long> (rtmp);
}

static jint
libpublisher_release(JNIEnv *env, jobject thiz, jlong cptr) {
    AVCPublisher *rtmp = reinterpret_cast<AVCPublisher *> (cptr);
    delete rtmp;
    return 0;
}

static jint
libpublisher_connect(JNIEnv *env, jobject thiz, jlong cptr) {
    AVCPublisher *rtmp = reinterpret_cast<AVCPublisher *> (cptr);
    int ret = rtmp->connect();
    return ret;
}

static jint
libpublisher_sendVideoData(JNIEnv *env, jobject thiz, jlong cptr,
                           jbyteArray _data, jint length, jlong timestamp) {
    jbyte *data = env->GetByteArrayElements(_data, NULL);

    AVCPublisher *rtmp = reinterpret_cast<AVCPublisher *> (cptr);
    int ret = rtmp->sendVideoData((uint8_t *) data, length, timestamp);

    env->ReleaseByteArrayElements(_data, data, 0);
    return ret;
}

static jint
libpublisher_sendAacData(JNIEnv *env, jobject thiz, jlong cptr,
                           jbyteArray _data, jint length, jlong timestamp) {
    jbyte *data = env->GetByteArrayElements(_data, NULL);

    AVCPublisher *rtmp = reinterpret_cast<AVCPublisher *> (cptr);
    int ret = rtmp->sendAacData((uint8_t *) data, length, timestamp);

    env->ReleaseByteArrayElements(_data, data, 0);
    return ret;
}

static JNINativeMethod libyuv_methods[] = {
        {"initialize",          "(java/lang/String;I)J",     (void *) libpublisher_initialize},
        {"release",             "(J)I",                      (void *) libpublisher_release},
        {"connect",             "(J)I",                      (void *) libpublisher_connect},
        {"sendVideoData",       "(J[BIJ)I",                  (void *) libpublisher_sendVideoData},
        {"sendAacData",         "(J[BIJ)I",                  (void *) libpublisher_sendAacData},
};

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    jvm = vm;

    if (jvm->GetEnv((void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass clz = jenv->FindClass("com/github/wontakkim/simplestreaming/AVCPublisher");
    if (clz == NULL) {
        return JNI_ERR;
    }

    if (jenv->RegisterNatives(clz, libyuv_methods, LIBPUBLISHER_ARRAY_ELEMS(libyuv_methods))) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}