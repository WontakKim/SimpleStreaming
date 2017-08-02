#ifndef RTMP_WRAPPER_H
#define RTMP_WRAPPER_H

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

#define BYTE uint8_t

#define RTMP_HEAD_SIZE (sizeof(RTMPPacket) + RTMP_MAX_HEADER_SIZE)

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

class RTMPWrapper
{
    private:
        RTMP *rtmp;

    public:
        ~RTMPWrapper();

        int initialize(char *url, int w, int h, int timeOut);

        int sendSpsAndPps(BYTE *sps, int spsLen, BYTE *pps, int ppsLen, long timestamp);

        int sendVideoData(BYTE *data, int len, long timestamp);

        int sendAacSpec(BYTE *data, int len);

        int sendAacData(BYTE *data, int len,long timestamp);

        int stop() const;
};

#endif // RTMP_WRAPPER_H
