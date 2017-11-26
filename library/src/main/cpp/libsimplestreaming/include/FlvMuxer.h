#ifndef FLV_MUXER_H
#define FLV_MUXER_H

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// include c header
#include "rtmp.h"
#include "rtmp_sys.h"
#include "log.h"
#include "android/log.h"
#include "time.h"

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

#define TAG "FLV_MUXER"
#define debug_print(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##__VA_ARGS__)

class FlvMuxer {

private:
    RTMP *rtmp;

public:
    FlvMuxer();

    ~FlvMuxer();

    int initialize(int timeout);

    int start(char *url);
    int stop();

    int isPlaying();

    int writeVideoData(uint8_t *data, int length, long timestamp);
    int writeAudioData(uint8_t *data, int length, long timestamp);

    RTMPPacket *buildH264SpsPpsPacket(uint8_t *sps, int spsLength, uint8_t *pps, int ppsLength);
    RTMPPacket *buildH264VideoPacket(int nalType, uint8_t *data, int length, long timestamp);
    RTMPPacket *buildH264AudioPacket(uint8_t *data, int length, long timestamp);
};

#endif // FLV_MUXER_H
