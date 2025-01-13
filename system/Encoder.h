#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <stdio.h>
#include <vector>
#include <future>
#include <thread>

#define WIDTH 1920
#define HEIGHT 1080
#define FPS 30
#define BITRATE 400000

class Encoder
{
private:
    AVCodec *_codec;
    AVCodecContext *_ctx;
    AVFrame *_frame;
    AVPacket _pkt;
    int _frameNo;

protected:
    std::vector<std::vector<uint8_t>> sync_encode(const std::vector<unsigned char> &data);

public:
    Encoder(/* args */);
    void Async_encode(const std::vector<unsigned char> &data, std::function<void(const std::vector<uint8_t> &)> callback);
    ~Encoder();
};

Encoder::Encoder(/* args */) : _codec(nullptr), _ctx(nullptr), _frame(nullptr), _frameNo(0)
{
    // 1. 初始化 FFmpeg 库
    av_register_all();
    avcodec_register_all();
    // 2. 查找 H.264 编码器（软件编码器）
    _codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!_codec)
    {
        fprintf(stderr, "Codec not found\n");
        return -1;
    }
    // 3. 创建编码上下文
    _ctx = avcodec_alloc_context3(_codec);
    if (!_ctx)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        return -1;
    }

    // 4. 设置编码器参数
    _ctx->bit_rate = BITRATE;
    _ctx->width = WIDTH;
    _ctx->height = HEIGHT;
    _ctx->time_base = (AVRational){1, FPS};
    _ctx->framerate = (AVRational){FPS, 1};
    _ctx->gop_size = 12; // I帧间隔
    _ctx->max_b_frames = 1;
    _ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    // 5. 打开编码器
    if (avcodec_open2(ctx, codec, NULL) < 0)
    {
        fprintf(stderr, "Could not open codec\n");
        return -1;
    }

    // 6. 创建一个 AVFrame 用于编码
    _frame = av_frame_alloc();
    if (!_frame)
    {
        fprintf(stderr, "Could not allocate video frame\n");
        return -1;
    }

    _frame->format = _ctx->pix_fmt;
    _frame->width = _ctx->width;
    _frame->height = _ctx->height;

    if (av_image_alloc(_frame->data, _frame->linesize, _ctx->width, _ctx->height, _ctx->pix_fmt, 32); < 0)
    {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        return -1;
    }

    av_init_packet(&_pkt);
    _pkt.data = nullptr;
    _pkt.size = 0;
}

void Encoder::Async_encode(const std::vector<unsigned char> &data, std::function<void(const std::vector<uint8_t> &)> callback)
{
    // Create a promise to hold the result
    std::promise<std::vector<std::vector<uint8_t>>> promise;
    // Get the future associated with the promise
    std::future<std::vector<std::vector<uint8_t>>> future = promise.get_future();

    // Create a thread to run the computationally intensive task
    std::thread([data, promise = std::move(promise)]() mutable
                {
        // Compute the encoding
        std::vector<std::vector<uint8_t>> result = sync_encode(data);
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
    // 7. 假设 YUV420 数据存储在 std::vector<unsigned char> data 中
    // 将 data 转换为 AVFrame 数据
    size_t y_size = WIDTH * HEIGHT;
    size_t uv_size = y_size / 4;

    // 填充 Y、U、V 分量
    memcpy(_frame->data[0], &data[0], y_size);                 // Y
    memcpy(_frame->data[1], &data[y_size], uv_size);           // U
    memcpy(_frame->data[2], &data[y_size + uv_size], uv_size); // V

    // 8. 编码视频帧并获取编码后的数据

    // 9. 编码并获取数据
    _frame->pts = _frameNo; // 这里设置PTS
    if (avcodec_send_frame(_ctx, _frame) < 0)
    {
        fprintf(stderr, "Error sending frame to encoder\n");
        return -1;
    }

    int ret = 1;
    std::vector<std::vector<uint8_t>> payloadslist;
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(_ctx, &_pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }
        else if (ret < 0)
        {
            fprintf(stderr, "Error encoding frame\n");
            return -1;
        }

        // 10. 将编码后的数据包转换为 std::vector<uint8_t> payload
        std::vector<uint8_t> payload(_pkt.data, _pkt.data + _pkt.size);
        payloadslist.push_back(payload);
        // 11. 调用自定义接口将编码后的数据推送出去
        av_packet_unref(&_pkt);
    }
    return payloadslist;
}

Encoder::~Encoder()
{
    // 12. 释放资源
    avcodec_free_context(&_ctx);
    av_frame_free(&_frame);
}

#endif