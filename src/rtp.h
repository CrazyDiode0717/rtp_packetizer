#pragma once

#include <memory>

const int RTP_HEADER_SIZE = 12;
const int MAX_RTP_PAYLOAD_SIZE = 1420;

struct RTPFixedHeader
{
    /* byte 0 */
    unsigned char csrc : 4;
    unsigned char extension : 1;
    unsigned char padding : 1;
    unsigned char version : 2;
    /* byte 1 */
    unsigned char payload : 7;
    unsigned char marker : 1;
    /* bytes 2, 3 */
    unsigned short seq;
    /**/ /* bytes 4-7 */
    unsigned int timestamp;
    /* bytes 8-11 */
    unsigned int ssrc;
};

struct RtpPacket
{
    RtpPacket()
        : data(new uint8_t[1600])
    {
        type = 0;
    }

    std::shared_ptr<uint8_t> data;
    uint32_t size;
    uint32_t timestamp;
    uint8_t type;
    uint8_t last;
};