#pragma once
#include "MediaSource.h"

class H264Source : public MediaSource
{
public:
    static H264Source* Create(uint32_t frameRate=25);
    ~H264Source();

    void SetFrameRate(uint32_t frameRate)
    { frameRate_ = frameRate; }

    uint32_t GetFrameRate() const 
    { return frameRate_; }

    /* SDP媒体描述 m= */
    virtual std::string GetMediaDescription(uint16_t port); 

    /* SDP媒体属性 a= */
    virtual std::string GetAttribute(); 

    bool HandleFrame(MediaFrame frame);

    static uint32_t GetTimeStamp();
	
private:
    H264Source(uint32_t frameRate);

    uint32_t frameRate_ = 25;
};