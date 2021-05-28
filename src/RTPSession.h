#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <memory>

#include "rtp.h"

class RTPSession
{
public:
    RTPSession();
    virtual ~RTPSession();

    int InitConnection(const std::string ip, int port);

    void SetPayloadType(uint32_t payload);

    int SendRTPPacket(RtpPacket& pkt);

private:
private:
    std::string ip_;
    int port_;
    int udp_socket_;
    int packet_seq_;
    RTPFixedHeader rtp_header_;
};