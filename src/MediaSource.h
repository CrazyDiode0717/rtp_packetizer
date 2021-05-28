#pragma once

#include <functional>
#include <string>
#include "rtp.h"
#include "media.h"

class MediaSource
{
public:
    typedef std::function<int(RtpPacket& pkt)> SendFrameCallback;

    MediaSource() {}
    virtual ~MediaSource() {}

    virtual MediaType GetMediaType() const
    {
        return mediaType_;
    }

    /* SDP媒体描述 m= */
    virtual std::string GetMediaDescription(uint16_t port = 0) = 0;

    /* SDP媒体属性 a= */
    virtual std::string GetAttribute() = 0;

    virtual bool HandleFrame(MediaFrame frame) = 0;

    virtual void SetSendFrameCallback(const SendFrameCallback cb)
    {
        sendFrameCallback_ = cb;
    }

    virtual uint32_t GetPayloadType() const
    {
        return payload_;
    }

protected:
    MediaType mediaType_ = NONE;
    uint32_t payload_ = 0;
    uint32_t clockRate_ = 0;
    SendFrameCallback sendFrameCallback_;
};