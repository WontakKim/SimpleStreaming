#include <malloc.h>
#include "RTMPWrapper.h"

int RTMPWrapper::initialize(char *url, int timeOut)
{
    RTMP_LogSetLevel(RTMP_LOGDEBUG);

    rtmp = RTMP_Alloc();
    RTMP_Init(rtmp);
    rtmp->Link.timeout = timeOut;
    RTMP_SetupURL(rtmp, url);
    RTMP_EnableWrite(rtmp);

    debug_print("Initialized RTMP !!!");

    return 0;
}

int RTMPWrapper::connect()
{
    if (!RTMP_Connect(rtmp, NULL) ) {
        debug_print("Connect failure !!!");
        return -1;
    }

    debug_print("Connected with server !!!");

    if (!RTMP_ConnectStream(rtmp, 0)) {
        debug_print("Connect stream failure !!!");
        return -1;
    }

    debug_print("Connected stream !!!");

    return 0;
}

int RTMPWrapper::sendSPSAndPPS(BYTE *sps, int spsLength, BYTE *pps, int ppsLength)
{
    RTMPPacket *packet = (RTMPPacket *) malloc(RTMP_HEAD_SIZE + 1024);
    memset(packet, 0, RTMP_HEAD_SIZE);
    packet->m_body = (char *) packet + RTMP_HEAD_SIZE;
    BYTE *body = (BYTE *) packet->m_body;

    int i = 0;
    body[i++] = 0x17; // 1:keyframe 7:AVC
    body[i++] = 0x00; // AVC sequence header

    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00; //fill in 0

    /*AVCDecoderConfigurationRecord*/
    body[i++] = 0x01;   // version
    body[i++] = sps[1]; // AVCProfileIndecation
    body[i++] = sps[2]; // profile_compatibilty
    body[i++] = sps[3]; // AVCLevelIndication
    body[i++] = 0xff;   // lengthSizeMinusOne

    /*SPS*/
    body[i++] = 0xe1;
    body[i++] = (spsLength >> 8) & 0xff;
    body[i++] = spsLength & 0xff;
    /*sps data*/
    memcpy(&body[i], sps, spsLength);

    i += spsLength;

    /*PPS*/
    body[i++] = 0x01;
    /*sps data length*/
    body[i++] = (ppsLength >> 8) & 0xff;
    body[i++] = ppsLength & 0xff;
    memcpy(&body[i], pps, ppsLength);
    i += ppsLength;

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = i;
    packet->m_nChannel = STREAM_CHANNEL_VIDEO;
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet->m_nInfoField2 = rtmp->m_stream_id;

    /*发送*/
    if (RTMP_IsConnected(rtmp)) {
        RTMP_SendPacket(rtmp, packet, TRUE);
    }
    free(packet);
    return 0;
}

int RTMPWrapper::sendVideoData(BYTE *buf, int len, long timestamp) {
    int type;

    /*去掉帧界定符*/
    if (buf[2] == 0x00) {/*00 00 00 01*/
        buf += 4;
        len -= 4;
    } else if (buf[2] == 0x01) {
        buf += 3;
        len - 3;
    }

    type = buf[0] & 0x1f;

    RTMPPacket *packet = (RTMPPacket *) malloc(RTMP_HEAD_SIZE + len + 9);
    memset(packet, 0, RTMP_HEAD_SIZE);
    packet->m_body = (char *) packet + RTMP_HEAD_SIZE;
    packet->m_nBodySize = len + 9;


    /* send video packet*/
    BYTE *body = (BYTE *) packet->m_body;
    memset(body, 0, len + 9);

    /*key frame*/
    body[0] = 0x27;
    if (type == NAL_SLICE_IDR) {
        body[0] = 0x17; //关键帧
    }

    body[1] = 0x01;/*nal unit*/
    body[2] = 0x00;
    body[3] = 0x00;
    body[4] = 0x00;

    body[5] = (len >> 24) & 0xff;
    body[6] = (len >> 16) & 0xff;
    body[7] = (len >> 8) & 0xff;
    body[8] = (len) & 0xff;

    /*copy data*/
    memcpy(&body[9], buf, len);

    packet->m_hasAbsTimestamp = 0;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nInfoField2 = rtmp->m_stream_id;
    packet->m_nChannel = STREAM_CHANNEL_VIDEO;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_nTimeStamp = timestamp;

    if (RTMP_IsConnected(rtmp)) {
        RTMP_SendPacket(rtmp, packet, TRUE);
    }
    free(packet);

    return 0;
}

int RTMPWrapper::sendAacSpec(BYTE *data, int spec_len) {
    RTMPPacket *packet;
    BYTE *body;
    int len = spec_len;//spec len 是2
    packet = (RTMPPacket *) malloc(RTMP_HEAD_SIZE + len + 2);
    memset(packet, 0, RTMP_HEAD_SIZE);
    packet->m_body = (char *) packet + RTMP_HEAD_SIZE;
    body = (BYTE *) packet->m_body;

    /*AF 00 +AAC RAW data*/
    body[0] = 0xAF;
    body[1] = 0x00;
    memcpy(&body[2], data, len);/*data 是AAC sequeuece header数据*/

    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = len + 2;
    packet->m_nChannel = STREAM_CHANNEL_AUDIO;
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_nInfoField2 = rtmp->m_stream_id;

    if (RTMP_IsConnected(rtmp)) {
        RTMP_SendPacket(rtmp, packet, TRUE);
    }
    free(packet);

    return 0;
}

int RTMPWrapper::sendAacData(BYTE *data, int len, long timeOffset) {
//    data += 5;
//    len += 5;
    if (len > 0) {
        RTMPPacket *packet;
        BYTE *body;
        packet = (RTMPPacket *) malloc(RTMP_HEAD_SIZE + len + 2);
        memset(packet, 0, RTMP_HEAD_SIZE);
        packet->m_body = (char *) packet + RTMP_HEAD_SIZE;
        body = (BYTE *) packet->m_body;

        /*AF 00 +AAC Raw data*/
        body[0] = 0xAF;
        body[1] = 0x01;
        memcpy(&body[2], data, len);

        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nBodySize = len + 2;
        packet->m_nChannel = STREAM_CHANNEL_AUDIO;
        packet->m_nTimeStamp = timeOffset;
        packet->m_hasAbsTimestamp = 0;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
        packet->m_nInfoField2 = rtmp->m_stream_id;
        if (RTMP_IsConnected(rtmp)) {
            RTMP_SendPacket(rtmp, packet, TRUE);
        }
        free(packet);

    }
    return 0;
}

int RTMPWrapper::stop() const {
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
    return 0;
}

RTMPWrapper::~RTMPWrapper() { stop(); }
