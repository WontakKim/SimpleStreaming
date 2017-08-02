LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES += librtmp

LOCAL_SRC_FILES := \
		source/RTMPWrapper.cpp

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_LDLIBS += -llog
LOCAL_MODULE := libpublisher

include $(BUILD_SHARED_LIBRARY)
