#include "H264Source.h"
#include <chrono>
#include <cstring>

H264Source::H264Source(uint32_t frameRate) : frameRate_(frameRate)
{
    payload_ = 96;
    mediaType_ = H264;
    clockRate_ = 90000;
}

H264Source *H264Source::Create(uint32_t frameRate)
{
    return new H264Source(frameRate);
}

H264Source::~H264Source()
{
}

std::string H264Source::GetMediaDescription(uint16_t port)
{
    char buf[100] = {0};
    sprintf(buf, "m=video %hu RTP/AVP 96", port);

    return std::string(buf);
}

std::string H264Source::GetAttribute()
{
    return std::string("a=rtpmap:96 H264/90000");
}

bool H264Source::HandleFrame(MediaFrame frame)
{
    uint8_t *frameBuf = frame.buffer.get();
    uint32_t frameSize = frame.size;

    if (frame.timestamp == 0)
        frame.timestamp = GetTimeStamp();

    if (frameSize <= MAX_RTP_PAYLOAD_SIZE)
    {
        RtpPacket rtpPkt;
        rtpPkt.type = frame.type;
        rtpPkt.timestamp = frame.timestamp;
        rtpPkt.size = frameSize + RTP_HEADER_SIZE;
        rtpPkt.last = 1;
        memcpy(rtpPkt.data.get() + RTP_HEADER_SIZE, frameBuf, frameSize); /* 预留12字节 rtp header */

        if (sendFrameCallback_)
        {
            if (!sendFrameCallback_(rtpPkt))
                return false;
        }
    }
    else
    {
        char FU_A[2] = {0};

        FU_A[0] = (frameBuf[0] & 0xE0) | 28;
        FU_A[1] = 0x80 | (frameBuf[0] & 0x1f);

        frameBuf += 1;
        frameSize -= 1;

        while (frameSize + 2 > MAX_RTP_PAYLOAD_SIZE)
        {
            RtpPacket rtpPkt;
            rtpPkt.type = frame.type;
            rtpPkt.timestamp = frame.timestamp;
            rtpPkt.size = RTP_HEADER_SIZE + MAX_RTP_PAYLOAD_SIZE;
            rtpPkt.last = 0;

            rtpPkt.data.get()[RTP_HEADER_SIZE] = FU_A[0];
            rtpPkt.data.get()[RTP_HEADER_SIZE + 1] = FU_A[1];
            memcpy(rtpPkt.data.get() + RTP_HEADER_SIZE + 2, frameBuf, MAX_RTP_PAYLOAD_SIZE - 2);

            if (sendFrameCallback_)
            {
                if (!sendFrameCallback_(rtpPkt))
                    return false;
            }

            frameBuf += MAX_RTP_PAYLOAD_SIZE - 2;
            frameSize -= MAX_RTP_PAYLOAD_SIZE - 2;

            FU_A[1] &= ~0x80;
        }

        {
            RtpPacket rtpPkt;
            rtpPkt.type = frame.type;
            rtpPkt.timestamp = frame.timestamp;
            rtpPkt.size = RTP_HEADER_SIZE + 2 + frameSize;
            rtpPkt.last = 1;

            FU_A[1] |= 0x40;
            rtpPkt.data.get()[RTP_HEADER_SIZE] = FU_A[0];
            rtpPkt.data.get()[RTP_HEADER_SIZE + 1] = FU_A[1];
            memcpy(rtpPkt.data.get() + RTP_HEADER_SIZE + 2, frameBuf, frameSize);

            if (sendFrameCallback_)
            {
                if (!sendFrameCallback_(rtpPkt))
                    return false;
            }
        }
    }

    return true;
}

uint32_t H264Source::GetTimeStamp()
{
    auto timePoint = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    return (uint32_t)((timePoint.time_since_epoch().count() + 500) / 1000 * 90);
}