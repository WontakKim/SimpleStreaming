#ifndef AVC_PUBLISHER_H
#define AVC_PUBLISHER_H

#include <stdlib.h>
#include <string.h>
#include "Rtmp.h"

#ifdef __cplusplus
extern "C" {
#endif

// include c header
#include "rtmp.h"
#include "rtmp_sys.h"
#include "log.h"
#include "android/log.h"
#include "time.h"

#define DEBUG

#ifdef DEBUG
#define TAG "PUBLISHER"
#define debug_print(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##__VA_ARGS__)
#else
#define TAG "PUBLISHER"
#define debug_print(fmt, ...)
#endif

#ifdef __cplusplus
}
#endif

#define NAL_SLICE       1
#define NAL_SLICE_DPA   2
#define NAL_SLICE_DPB   3
#define NAL_SLICE_DPC   4
#define NAL_SLICE_IDR   5
#define NAL_SEI         6
#define NAL_SPS         7
#define NAL_PPS         8
#define NAL_AUD         9
#define NAL_FILLER      12

#define STREAM_CHANNEL_METADATA  0x03
#define STREAM_CHANNEL_VIDEO     0x04
#define STREAM_CHANNEL_AUDIO     0x05

class AVCPublisher {

    private:
        RTMP *rtmp;

    public:
        ~AVCPublisher();

        int initialize(char *url, int timeOut);

        int release() const;

        int connect();

        int sendVideoData(uint8_t *data, int length, long timestamp);

        int sendAacData(uint8_t *data, int length, long timestamp);
};

#endif // AVC_PUBLISHER_H
