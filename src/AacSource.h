#pragma once
#include "MediaSource.h"

class AacSource : public MediaSource
{
public:
    static AacSource* Create(uint32_t samplerate=48000, uint32_t channels=2, bool hasAdts=true);
    ~AacSource();

    uint32_t GetSamplerate() const
    { return samplerate_; }

    uint32_t GetChannels() const
    { return channels_; }

    /* SDP媒体描述 m= */
    virtual std::string GetMediaDescription(uint16_t port); 

    /* SDP媒体属性 a= */
    virtual std::string GetAttribute(); 

    bool HandleFrame(MediaFrame frame);

    static uint32_t GetTimeStamp(uint32_t samplerate =48000);
	
private:
    AacSource(uint32_t samplerate, uint32_t channels, bool hasAdts);

    uint32_t samplerate_ = 44100;  
    uint32_t channels_ = 2;         
    bool hasAdts_ = true;

    static const int ADTS_SIZE = 7;
    static const int AU_SIZE   = 4;
};