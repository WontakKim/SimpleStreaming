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
    if (!RTMP_Connect(rtmp, NULL))
    {
        debug_print("Connect failure !!!");
        return -1;
    }

    debug_print("Connected with server !!!");

    if (!RTMP_ConnectStream(rtmp, 0))
    {
        debug_print("Connect stream failure !!!");
        return -1;
    }

    debug_print("Connected stream !!!");

    return 0;
}

int RTMPWrapper::sendVideoSpsAndPps(uint8_t *sps, int spsLength, uint8_t *pps, int ppsLength)
{
    RTMPPacket *packet = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    memset(packet, 0, sizeof(RTMPPacket));

    /* Remove SPS NAL start prefix code */
    int spsNALStartPrefixBytes = 4; // 00 00 00 01 : 4 bytes
    if (sps[2] == 0x01) // 00 00 01 : 3 bytes
    {
        spsNALStartPrefixBytes = 3;
    }

    sps += spsNALStartPrefixBytes;
    spsLength -= spsNALStartPrefixBytes;

    /* Remove PPS NAL start prefix code */
    int ppsNALStartPrefixBytes = 4; // 00 00 00 01 : 4 bytes
    if (pps[2] == 0x01) // 00 00 01 : 3 bytes
    {
        ppsNALStartPrefixBytes = 3;
    }

    pps += ppsNALStartPrefixBytes;
    ppsLength -= ppsNALStartPrefixBytes;

    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_hasAbsTimestamp = 0;
    packet->m_nChannel = STREAM_CHANNEL_VIDEO;
    packet->m_nTimeStamp = 0;
    packet->m_nInfoField2 = rtmp->m_stream_id;
    packet->m_nBodySize = spsLength + ppsLength + 16;
    packet->m_body = (char *) malloc(packet->m_nBodySize);
    memset(packet->m_body, 0, packet->m_nBodySize);

    int index = 0;
    uint8_t *body = (uint8_t *) packet->m_body;

    body[index++] = 0x17; // 1:Key Frame 7:AVC (H.264)
    body[index++] = 0x00; // AVC sequence header

    /* Start Time */
    body[index++] = 0x00;
    body[index++] = 0x00;
    body[index++] = 0x00;

    /* AVC Decoder Configuration Record */
    body[index++] = 0x01;   // Configuration Version
    body[index++] = sps[1]; // AVC Profile Indication
    body[index++] = sps[2]; // Profile Compatibility
    body[index++] = sps[3]; // AVC Level Indication
    body[index++] = 0xFF;   // Length Size Minus One

    /* Sequence Parameter Sets */
    body[index++] = 0xE1; // Num Of Sequence Parameter Sets
    /* SPS Length */
    body[index++] = (spsLength >> 8) & 0xFF;
    body[index++] = spsLength & 0xFF;
    /* SPS Data*/
    memcpy(&body[index], sps, spsLength);
    index += spsLength;

    /* Picture Parameter Sets */
    body[index++] = 0x01; // Num Of Picture Parameter Sets
    /* PPS Length */
    body[index++] = (ppsLength >> 8) & 0xFF;
    body[index++] = ppsLength & 0xFF;
    /* PPS Data */
    memcpy(&body[index], pps, ppsLength);

    if (RTMP_IsConnected(rtmp))
    {
        RTMP_SendPacket(rtmp, packet, TRUE);
    }

    free(packet->m_body);
    free(packet);

    return 0;
}

int RTMPWrapper::sendVideoData(uint8_t *data, int length, long timestamp)
{
    RTMPPacket *packet = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    memset(packet, 0, sizeof(RTMPPacket));

    /* Remove NAL start prefix code */
    int nalStartPrefixBytes = 4; // 00 00 00 01 : 4 bytes
    if (data[2] == 0x01) // 00 00 01 : 3 bytes
    {
        nalStartPrefixBytes = 3;
    }

    data += nalStartPrefixBytes;
    length -= nalStartPrefixBytes;

    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_hasAbsTimestamp = 0;
    packet->m_nChannel = STREAM_CHANNEL_VIDEO;
    packet->m_nTimeStamp = timestamp;
    packet->m_nInfoField2 = rtmp->m_stream_id;
    packet->m_nBodySize = length + 9;
    packet->m_body = (char *) malloc(packet->m_nBodySize);
    memset(packet->m_body, 0, packet->m_nBodySize);

    uint8_t *body = (uint8_t *) packet->m_body;

    int type = data[0] & 0x1F;

    // Key Frame
    body[0] = 0x27;
    if (type == NAL_SLICE_IDR)
    {
        body[0] = 0x17;
    }
    else if (type == NAL_SEI)
    {
        free(body);
        free(packet);
        return 0;
    }

    body[1] = 0x01; // NAL_UNIT
    body[2] = 0x00;
    body[3] = 0x00;
    body[4] = 0x00;

    body[5] = (length >> 24) & 0xff;
    body[6] = (length >> 16) & 0xff;
    body[7] = (length >> 8) & 0xff;
    body[8] = (length) & 0xff;

    /* Video data */
    memcpy(&body[9], data, length);

    if (RTMP_IsConnected(rtmp))
    {
        RTMP_SendPacket(rtmp, packet, TRUE);
    }

    free(packet->m_body);
    free(packet);

    return 0;
}

int RTMPWrapper::sendAacSpec(uint8_t *data, int length)
{
    RTMPPacket *packet = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    memset(packet, 0, sizeof(RTMPPacket));

    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_hasAbsTimestamp = 0;
    packet->m_nChannel = STREAM_CHANNEL_AUDIO;
    packet->m_nTimeStamp = 0;
    packet->m_nInfoField2 = rtmp->m_stream_id;
    packet->m_nBodySize = length + 2;
    packet->m_body = (char *) malloc(packet->m_nBodySize);
    memset(packet->m_body, 0, packet->m_nBodySize);

    uint8_t *body = (uint8_t *) packet->m_body;

    /* AAC RAW data */
    body[0] = 0xAF;
    body[1] = 0x00;

    /* AAC Spec data */
    memcpy(&body[2], data, length);

    if (RTMP_IsConnected(rtmp))
    {
        RTMP_SendPacket(rtmp, packet, TRUE);
    }

    free(packet->m_body);
    free(packet);

    return 0;
}

int RTMPWrapper::sendAacData(uint8_t *data, int len, long timeOffset) {
//    data += 5;
//    len += 5;
    if (len > 0) {
        RTMPPacket *packet;
        uint8_t *body;
        packet = (RTMPPacket *) malloc(sizeof(RTMPPacket) + len + 2);
        memset(packet, 0, sizeof(RTMPPacket));
        packet->m_body = (char *) packet + sizeof(RTMPPacket);
        body = (uint8_t *) packet->m_body;

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

RTMPWrapper::~RTMPWrapper() {
    stop();
}
