#include "RTPSession.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

RTPSession::RTPSession() : port_(-1), udp_socket_(-1), packet_seq_(0)
{
    rtp_header_.version = 2;
    rtp_header_.ssrc = 0;
    rtp_header_.csrc = 0;
    rtp_header_.extension = 0;
}

RTPSession::~RTPSession()
{
    if (-1 != udp_socket_)
    {
        close(udp_socket_);
        udp_socket_ = -1;
    }
}

int RTPSession::InitConnection(const std::string ip, int port)
{
    ip_ = ip;
    port_ = port;

    int ret = 0;
    sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = inet_addr(ip_.c_str());

    udp_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == udp_socket_)
    {
        fprintf(stderr, "UDP create socket error:%s\n", strerror(errno));
        return -(errno);
    }

    ret = connect(udp_socket_, (const sockaddr *)&server_addr, sizeof(sockaddr_in));
    if (-1 == ret)
    {
        printf("UDP connect  error:%s\n", strerror(errno));
        return -(errno);
    }

    return 1;
}

void RTPSession::SetPayloadType(uint32_t payload)
{
    rtp_header_.payload = payload;
}

int RTPSession::SendRTPPacket(RtpPacket &pkt)
{
    rtp_header_.marker = pkt.last;
    rtp_header_.timestamp = htonl(pkt.timestamp);
    rtp_header_.seq = htons(packet_seq_++);
    memcpy(pkt.data.get(), &rtp_header_, RTP_HEADER_SIZE);

    return send(udp_socket_, pkt.data.get(), pkt.size, 0);
}