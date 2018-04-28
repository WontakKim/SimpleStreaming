#include <jni.h>
#include <stdio.h>
#include <FlvMuxer.h>

#define LIBPUBLISHER_ARRAY_ELEMS(a)  (sizeof(a) / sizeof(a[0]))

static JavaVM *jvm;
static JNIEnv *jenv;

static jlong
libsimplestreaming_initialize(JNIEnv *env, jobject thiz) {
    return reinterpret_cast<long> (new FlvMuxer());
}

static jint
libsimplestreaming_release(JNIEnv *env, jobject thiz, jlong pointer) {
    FlvMuxer *flvMuxer = reinterpret_cast<FlvMuxer *> (pointer);
    delete flvMuxer;
    return 0;
}

static jint
libsimplestreaming_start(JNIEnv *env, jobject thiz, jlong pointer, jstring _url, jint timeout) {
    const char *url = env->GetStringUTFChars(_url, 0);
    char* url_copy = (char *) malloc(strlen(url) + 1);
    strcpy(url_copy, url);

    FlvMuxer *flvMuxer = reinterpret_cast<FlvMuxer *> (pointer);
    int ret = flvMuxer->start(url_copy, timeout);

    free(url_copy);
    env->ReleaseStringUTFChars(_url, url);
    return ret;
}

static jint
libsimplestreaming_stop(JNIEnv *env, jobject thiz, jlong pointer) {
    FlvMuxer *flvMuxer = reinterpret_cast<FlvMuxer *> (pointer);
    return flvMuxer->stop();
}

static jint
libsimplestreaming_isPlaying(JNIEnv *env, jobject thiz, jlong pointer) {
    FlvMuxer *flvMuxer = reinterpret_cast<FlvMuxer *> (pointer);
    return flvMuxer->isPlaying();
}

static jint
libsimplestreaming_writeVideoData(JNIEnv *env, jobject thiz, jlong pointer,
                            jbyteArray _data, jint length, jlong timestamp) {
    jbyte *data = env->GetByteArrayElements(_data, NULL);

    FlvMuxer *flvMuxer = reinterpret_cast<FlvMuxer *> (pointer);
    int ret = flvMuxer->writeVideoData((uint8_t *) data, length, timestamp);

    env->ReleaseByteArrayElements(_data, data, 0);
    return ret;
}

static jint
libsimplestreaming_writeAudioData(JNIEnv *env, jobject thiz, jlong pointer,
                            jbyteArray _data, jint length, jlong timestamp) {
    jbyte *data = env->GetByteArrayElements(_data, NULL);

    FlvMuxer *flvMuxer = reinterpret_cast<FlvMuxer *> (pointer);
    int ret = flvMuxer->writeAudioData((uint8_t *) data, length, timestamp);

    env->ReleaseByteArrayElements(_data, data, 0);
    return ret;
}

static JNINativeMethod libsimplestreaming_methods[] = {
        {"initialize",          "()J",                       (void *) libsimplestreaming_initialize    },
        {"release",             "(J)I",                      (void *) libsimplestreaming_release       },
        {"start",               "(JLjava/lang/String;I)I",   (void *) libsimplestreaming_start         },
        {"stop",                "(J)I",                      (void *) libsimplestreaming_stop          },
        {"isPlaying",           "(J)I",                      (void *) libsimplestreaming_isPlaying     },
        {"writeVideoData",      "(J[BIJ)I",                  (void *) libsimplestreaming_writeVideoData},
        {"writeAudioData",      "(J[BIJ)I",                  (void *) libsimplestreaming_writeAudioData},
};

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    jvm = vm;

    if (jvm->GetEnv((void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass clz = jenv->FindClass("com/github/wontakkim/simplestreaming/muxers/FlvMuxer");
    if (clz == NULL) {
        return JNI_ERR;
    }

    if (jenv->RegisterNatives(clz, libsimplestreaming_methods, LIBPUBLISHER_ARRAY_ELEMS(libsimplestreaming_methods))) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}