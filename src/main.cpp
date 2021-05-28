#include <cstdio>
#include <memory>
#include <thread>
#include <chrono>
#include <fstream>
#include "H264Source.h"
#include "RTPSession.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}


std::ofstream h264Writer;

static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, MediaSource *source)
{
    int ret;

    /* send the frame to the encoder */
    if (frame)
        printf("Send frame %3" PRId64 "\n", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0)
    {
        fprintf(stderr, "Error sending a frame for encoding\n");
        return;
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
        {
            fprintf(stderr, "Error during encoding\n");
            return;
        }

        printf("Write packet %3" PRId64 " (size=%5d)\n", pkt->pts, pkt->size);

        MediaFrame vidoeFrame;
        vidoeFrame.size = 0;

        h264Writer.write((char*)pkt->data, pkt->size);
        h264Writer.flush();

        vidoeFrame.type = VIDEO_FRAME_P;

        if (pkt->data[4] == 0x67 || pkt->data[4] == 0x65 || pkt->data[4] == 0x68)
        {
            vidoeFrame.type = VIDEO_FRAME_I;
        }

        //去掉00000001
        vidoeFrame.size = pkt->size - 4;
        vidoeFrame.buffer.reset(new uint8_t[vidoeFrame.size]);
        memcpy(vidoeFrame.buffer.get(), pkt->data + 4, vidoeFrame.size);

        source->HandleFrame(vidoeFrame);

        av_packet_unref(pkt);
    }
}

int Process(MediaSource *source)
{
    const AVCodec *codec;
    AVCodecContext *c = NULL;
    int i, ret, x, y;
    AVFrame *frame;
    AVPacket *pkt;
    uint8_t endcode[] = {0, 0, 1, 0xb7};

    avcodec_register_all();

    codec = avcodec_find_encoder_by_name("libx264");
    if (!codec)
    {
        fprintf(stderr, "Codec libx264 not found\n");
        return 0;
    }

    c = avcodec_alloc_context3(codec);
    if (!c)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        return 0;
    }

    pkt = av_packet_alloc();
    if (!pkt)
        return 0;

    /* put sample parameters */
    c->bit_rate = 500000;
    /* resolution must be a multiple of two */
    c->width = 640;
    c->height = 480;
    /* frames per second */
    c->time_base = (AVRational){1, 25};
    c->framerate = (AVRational){25, 1};

    c->gop_size = 12;
    c->max_b_frames = 0;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "fast", 0);

    ret = avcodec_open2(c, codec, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open codec\n");
        return 0;
    }

    frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, "Could not allocate video frame\n");
        return 0;
    }
    frame->format = c->pix_fmt;
    frame->width = c->width;
    frame->height = c->height;

    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Could not allocate the video frame data\n");
        return 0;
    }

    /* encode 1 second of video */
    for (i = 0; i < 1000; i++)
    {
        /* make sure the frame data is writable */
        ret = av_frame_make_writable(frame);
        if (ret < 0)
            return 0;

        /* prepare a dummy image */
        /* Y */
        for (y = 0; y < c->height; y++)
        {
            for (x = 0; x < c->width; x++)
            {
                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
            }
        }

        /* Cb and Cr */
        for (y = 0; y < c->height / 2; y++)
        {
            for (x = 0; x < c->width / 2; x++)
            {
                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
            }
        }

        frame->pts = i;

        /* encode the image */
        encode(c, frame, pkt, source);

        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }

    /* flush the encoder */
    encode(c, NULL, pkt, source);

    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);
}

int main()
{
    int nRet = 0;

    h264Writer.open("/home/cent/media/test.h264", std::ios::binary);

    RTPSession rtpSession;
    nRet = rtpSession.InitConnection("172.16.169.15", 8899);
    if (nRet <= 0)
    {
        printf("InitConnection failed\n");
        return 0;
    }

    std::unique_ptr<MediaSource> prtH264Source(H264Source::Create());
    if (!prtH264Source)
    {
        printf("create h264 source failed\n");
        return 0;
    }

    rtpSession.SetPayloadType(prtH264Source->GetPayloadType());

    prtH264Source->SetSendFrameCallback(std::bind(&RTPSession::SendRTPPacket, &rtpSession, std::placeholders::_1));

    Process(prtH264Source.get());

    printf("end\n");
    return 0;
}