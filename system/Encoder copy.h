#ifndef __ENCODER_H__
#define __ENCODER_H__

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

#include <stdio.h>
#include <vector>
#include <future>
#include <thread>

#define WIDTH 800
#define HEIGHT 600
#define FPS 30
#define BITRATE 400000

class Encoder
{
private:
    const AVCodec *_codec;
    AVCodecContext *_ctx;
    AVFrame *_frame;
    AVPacket *_pkt;
    int _frameNo;

protected:
    std::vector<std::vector<uint8_t>> sync_encode(const std::vector<unsigned char> &data);

public:
    Encoder(/* args */);
    void Async_encode(const std::vector<unsigned char> &data, std::function<void(const std::vector<uint8_t> &)> callback);
    ~Encoder();
};

Encoder::Encoder(/* args */) : _codec(nullptr), _ctx(nullptr), _frame(nullptr), _pkt(nullptr), _frameNo(0)
{
    // Find H.264 encoder
    _codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!_codec)
    {
        fprintf(stderr, "Codec not found\n");
        return; // Exit constructor if codec not found
    }

    // Allocate codec context
    _ctx = avcodec_alloc_context3(_codec);
    if (!_ctx)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        return; // Exit constructor if context allocation fails
    }

    // Set codec parameters
    _ctx->bit_rate = BITRATE;
    _ctx->width = WIDTH;
    _ctx->height = HEIGHT;
    _ctx->time_base = (AVRational){1, FPS};
    _ctx->framerate = (AVRational){FPS, 1};
    _ctx->gop_size = 12; // I-frame interval
    _ctx->max_b_frames = 1;
    _ctx->pix_fmt = AV_PIX_FMT_YUYV422; // Set to YUYV422 pixel format

    // Open codec
    if (avcodec_open2(_ctx, _codec, NULL) < 0)
    {
        fprintf(stderr, "Could not open codec\n");
        return; // Exit constructor if codec opening fails
    }

    // Allocate frame
    _frame = av_frame_alloc();
    if (!_frame)
    {
        fprintf(stderr, "Could not allocate video frame\n");
        return; // Exit constructor if frame allocation fails
    }

    _frame->format = _ctx->pix_fmt;
    _frame->width = _ctx->width;
    _frame->height = _ctx->height;

    if (av_frame_get_buffer(_frame, 32) < 0)
    {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        return; // Exit constructor if image allocation fails
    }

    _pkt = av_packet_alloc();
    if (!_pkt)
    {
        fprintf(stderr, "Could not allocate AVPacket\n");
        return; // Exit constructor if packet allocation fails
    }
}

void Encoder::Async_encode(const std::vector<unsigned char> &data, std::function<void(const std::vector<uint8_t> &)> callback)
{
    // Create a promise to hold the result
    std::promise<std::vector<std::vector<uint8_t>>> promise;
    // Get the future associated with the promise
    std::future<std::vector<std::vector<uint8_t>>> future = promise.get_future();

    // Create a thread to run the computationally intensive task
    std::thread([this, data, promise = std::move(promise)]() mutable
                {
        // Compute the encoding
        std::vector<std::vector<uint8_t>> result = this->sync_encode(data);
        // Set the result in the promise
        promise.set_value(result); })
        .detach();

    // Create a thread to wait for the result and call the callback
    std::thread([callback = std::move(callback), future = std::move(future)]() mutable
                {
        // Wait for the result
        std::vector<std::vector<uint8_t>> result = future.get();
        // Call the callback with the result
        for (const auto& res : result)
        {
            callback(res);
        } })
        .detach();
}

std::vector<std::vector<uint8_t>> Encoder::sync_encode(const std::vector<unsigned char> &data)
{
    _frameNo++;
    // Convert data to AVFrame data
    // Copy the YUYV data into the frame's data
    memcpy(_frame->data[0], data.data(), data.size()); // Y

    // Encode video frame and get encoded data
    _frame->pts = _frameNo; // Set PTS
    if (avcodec_send_frame(_ctx, _frame) < 0)
    {
        fprintf(stderr, "Error sending frame to encoder\n");
        return {};
    }

    int ret = 1;
    std::vector<std::vector<uint8_t>> payloadslist;
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(_ctx, _pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }
        else if (ret < 0)
        {
            fprintf(stderr, "Error encoding frame\n");
            return {};
        }

        // Convert encoded packet to std::vector<uint8_t> payload
        std::vector<uint8_t> payload(_pkt->data, _pkt->data + _pkt->size);
        payloadslist.push_back(payload);
        // Unreference the packet
        av_packet_unref(_pkt);
    }
    return payloadslist;
}

Encoder::~Encoder()
{
    // Free resources
    if (_pkt)
    {
        av_packet_free(&_pkt);
    }
    if (_ctx)
    {
        avcodec_free_context(&_ctx);
    }
    if (_frame)
    {
        av_frame_free(&_frame);
    }
}

#endif