#include "H265Source.h"
#include <chrono>
#include <cstring>

H265Source::H265Source(uint32_t frameRate) : frameRate_(frameRate)
{
    payload_ = 96;
    mediaType_ = H265;
    clockRate_ = 90000;
}

H265Source *H265Source::Create(uint32_t frameRate)
{
    return new H265Source(frameRate);
}

H265Source::~H265Source()
{
}

std::string H265Source::GetMediaDescription(uint16_t port)
{
    char buf[100] = {0};
    sprintf(buf, "m=video %hu RTP/AVP 96", port);

    return std::string(buf);
}

std::string H265Source::GetAttribute()
{
    return std::string("a=rtpmap:96 H265/90000");
}

bool H265Source::HandleFrame(MediaFrame frame)
{
    uint8_t *frameBuf = frame.buffer.get();
    uint32_t frameSize = frame.size;

    if (frame.timestamp == 0)
        frame.timestamp = GetTimeStamp();

    if (frameSize <= MAX_RTP_PAYLOAD_SIZE) 
    {
		RtpPacket rtp_pkt;
		rtp_pkt.type = frame.type;
		rtp_pkt.timestamp = frame.timestamp;
		rtp_pkt.size = frameSize + RTP_HEADER_SIZE;
		rtp_pkt.last = 1;

		memcpy(rtp_pkt.data.get()+4+RTP_HEADER_SIZE, frameBuf, frameSize);
        
		if (sendFrameCallback_) 
        {
			if (!sendFrameCallback_(rtp_pkt)) 
            {
				return false;
			}          
		}
	}	
	else 
    {	
		char FU[3] = {0};	
		char nalUnitType = (frameBuf[0] & 0x7E) >> 1; 
		FU[0] = (frameBuf[0] & 0x81) | (49<<1); 
		FU[1] = frameBuf[1]; 
		FU[2] = (0x80 | nalUnitType); 
        
		frameBuf  += 2;
		frameSize -= 2;
        
		while (frameSize + 3 > MAX_RTP_PAYLOAD_SIZE) 
        {
			RtpPacket rtp_pkt;
			rtp_pkt.type = frame.type;
			rtp_pkt.timestamp = frame.timestamp;
			rtp_pkt.size = 4 + RTP_HEADER_SIZE + MAX_RTP_PAYLOAD_SIZE;
			rtp_pkt.last = 0;

			rtp_pkt.data.get()[RTP_HEADER_SIZE+4] = FU[0];
			rtp_pkt.data.get()[RTP_HEADER_SIZE+5] = FU[1];
			rtp_pkt.data.get()[RTP_HEADER_SIZE+6] = FU[2];
			memcpy(rtp_pkt.data.get() + RTP_HEADER_SIZE + 3, frameBuf, MAX_RTP_PAYLOAD_SIZE - 3);
            
			if (sendFrameCallback_) 
            {
				if (!sendFrameCallback_(rtp_pkt)) 
                {
					return false;
				}                
			}
            
			frameBuf  += (MAX_RTP_PAYLOAD_SIZE - 3);
			frameSize -= (MAX_RTP_PAYLOAD_SIZE - 3);
        
			FU[2] &= ~0x80;						
		}
        
		{
			RtpPacket rtp_pkt;
			rtp_pkt.type = frame.type;
			rtp_pkt.timestamp = frame.timestamp;
			rtp_pkt.size = 4 + RTP_HEADER_SIZE + 3 + frameSize;
			rtp_pkt.last = 1;

			FU[2] |= 0x40;
			rtp_pkt.data.get()[RTP_HEADER_SIZE] = FU[0];
			rtp_pkt.data.get()[RTP_HEADER_SIZE + 1] = FU[1];
			rtp_pkt.data.get()[RTP_HEADER_SIZE + 2] = FU[2];
			memcpy(rtp_pkt.data.get() + RTP_HEADER_SIZE + 3, frameBuf, frameSize);
            
			if (sendFrameCallback_) 
            {
				if (!sendFrameCallback_(rtp_pkt)) 
                {
					return false;
				}               
			}
		}            
	}

    return true;
}

uint32_t H265Source::GetTimeStamp()
{
    auto timePoint = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    return (uint32_t)((timePoint.time_since_epoch().count() + 500) / 1000 * 90);
}