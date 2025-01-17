#ifndef __ENCODER_H__
#define __ENCODER_H__

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <stdio.h>
#include <vector>
#include <future>
#include <thread>

#define WIDTH 640
#define HEIGHT 480
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
    std::vector<uint8_t> sync_encode(const std::vector<unsigned char> &data);
    std::function<void(const std::vector<uint8_t> &)> _callback;

public:
    Encoder(std::function<void(const std::vector<uint8_t> &)> callback);
    void Async_encode(const std::vector<unsigned char> &data);
    ~Encoder();
};

Encoder::Encoder(std::function<void(const std::vector<uint8_t> &)> callback) : _codec(nullptr), _ctx(nullptr), _frame(nullptr), _pkt(nullptr), _frameNo(0), _callback(callback)
{
    // Find the H.264 encoder that supports RGB input
    _codec = avcodec_find_encoder_by_name("libx264rgb");
    if (!_codec)
    {
        throw std::runtime_error("Codec libx264rgb not found");
    }

    // Allocate the codec context
    _ctx = avcodec_alloc_context3(_codec);
    if (!_ctx)
    {
        throw std::runtime_error("Failed to allocate codec context");
    }

    // Set codec parameters: resolution, pixel format, and frame rate
    _ctx->bit_rate = BITRATE;
    _ctx->width = WIDTH;
    _ctx->height = HEIGHT;
    _ctx->time_base = (AVRational){1, FPS};
    _ctx->framerate = (AVRational){FPS, 1};
    _ctx->gop_size = 12;
    _ctx->max_b_frames = 1;
    _ctx->pix_fmt = AV_PIX_FMT_RGB24;

    // Open the codec
    if (avcodec_open2(_ctx, _codec, nullptr) < 0)
    {
        throw std::runtime_error("Failed to open codec");
    }

    // Allocate an AVFrame for the raw RGB data
    _frame = av_frame_alloc();
    if (!_frame)
    {
        throw std::runtime_error("Failed to allocate frame");
    }

    // Set frame properties
    _frame->format = _ctx->pix_fmt;
    _frame->width = _ctx->width;
    _frame->height = _ctx->height;

    // Allocate memory for the frame buffer
    if (av_frame_get_buffer(_frame, 32) < 0)
    {
        throw std::runtime_error("Failed to allocate frame buffer");
    }

    // Allocate an AVPacket for encoded data
    _pkt = av_packet_alloc();
    if (!_pkt)
    {
        throw std::runtime_error("Failed to allocate packet");
    }
}

void Encoder::Async_encode(const std::vector<unsigned char> &data)
{
    // Create a promise to hold the result
    std::promise<std::vector<uint8_t>> promise;
    // Get the future associated with the promise
    std::future<std::vector<uint8_t>> future = promise.get_future();

    // Create a thread to run the computationally intensive task
    std::thread([this, data, promise = std::move(promise)]() mutable
                {
        // Compute the encoding
        std::vector<uint8_t> result = this->sync_encode(data);
        // Set the result in the promise
        promise.set_value(result); })
        .detach();

    // Create a thread to wait for the result and call the callback
    std::thread([this, future = std::move(future)]() mutable
                {
        // Wait for the result
        std::vector<uint8_t> result = future.get();
        // Call the callback with the result
            _callback(result); })
        .detach();
}

std::vector<uint8_t> Encoder::sync_encode(const std::vector<unsigned char> &data)
{
    _frameNo++;

    // Check if the input data matches the expected frame size
    if (data.size() != WIDTH * HEIGHT * 3)
    {
        throw std::invalid_argument("Frame size does not match encoder dimensions");
    }

    // Copy the RGB data into the frame buffer
    memcpy(_frame->data[0], data.data(), data.size());

    // Set PTS
    _frame->pts = _frameNo;

    // Send the frame to the encoder
    if (avcodec_send_frame(_ctx, _frame) < 0)
    {
        throw std::runtime_error("Failed to send frame to encoder");
    }

    // Retrieve encoded packets from the encoder
    std::vector<uint8_t> encodedData;
    while (true)
    {
        int ret = avcodec_receive_packet(_ctx, _pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            // No more packets available or end of stream
            break;
        }
        else if (ret < 0)
        {
            throw std::runtime_error("Failed to receive packet from encoder");
        }

        // Append the packet data to the encoded data vector
        encodedData.insert(encodedData.end(), _pkt->data, _pkt->data + _pkt->size);
        av_packet_unref(_pkt); // Unreference the packet to reuse it
    }
    return encodedData; // Return the encoded frame data
}

Encoder::~Encoder()
{
    // Free codec context if allocated
    if (_ctx)
    {
        avcodec_free_context(&_ctx);
    }
    // Free frame if allocated
    if (_frame)
    {
        av_frame_free(&_frame);
    }
    // Free packet if allocated
    if (_pkt)
    {
        av_packet_free(&_pkt);
    }
}

#endif