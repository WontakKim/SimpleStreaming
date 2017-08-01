LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_STATIC_LIBRARIES += libpolarssl

LOCAL_SRC_FILES := \
		source/amf.c			\
		source/hashswf.c	\
		source/log.c			\
		source/parseurl.c	\
		source/rtmp.c			\
		source/thread.c		\

LOCAL_CFLAGS += -DCRYPTO=POLARSSL -DUSE_POLARSSL
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_MODULE := librtmp

include $(BUILD_SHARED_LIBRARY)
