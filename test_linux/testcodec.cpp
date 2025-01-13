// 假设每次从摄像头获取单帧YUV420数据存储在 std::vector<unsigned char> data 中
// data 保存的是单帧的 YUV420 数据（1帧）

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <stdio.h>
#include <vector>

extern void send_payload(const std::vector<uint8_t>& payload);  // 你自己实现的数据发送接口

#define WIDTH 1920
#define HEIGHT 1080
#define FPS 30
#define BITRATE 400000

extern std::vector<unsigned char> data; // 存储YUV420数据的vector

int main() {
    AVCodec *codec;
    AVCodecContext *ctx = NULL;
    AVFrame *frame = NULL;
    AVPacket pkt;
    int ret;

    // 1. 初始化 FFmpeg 库
    av_register_all();
    avcodec_register_all();

    // 2. 查找 H.264 编码器（软件编码器）
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        return -1;
    }

    // 3. 创建编码上下文
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return -1;
    }

    // 4. 设置编码器参数
    ctx->bit_rate = BITRATE;
    ctx->width = WIDTH;
    ctx->height = HEIGHT;
    ctx->time_base = (AVRational){1, FPS};
    ctx->framerate = (AVRational){FPS, 1};
    ctx->gop_size = 12; // I帧间隔
    ctx->max_b_frames = 1;
    ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    // 5. 打开编码器
    ret = avcodec_open2(ctx, codec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open codec\n");
        return -1;
    }

    // 6. 创建一个 AVFrame 用于编码
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        return -1;
    }

    frame->format = ctx->pix_fmt;
    frame->width = ctx->width;
    frame->height = ctx->height;

    ret = av_image_alloc(frame->data, frame->linesize, ctx->width, ctx->height, ctx->pix_fmt, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        return -1;
    }

    // 7. 假设 YUV420 数据存储在 std::vector<unsigned char> data 中
    // 将 data 转换为 AVFrame 数据
    size_t y_size = WIDTH * HEIGHT;
    size_t uv_size = y_size / 4;

    // 填充 Y、U、V 分量
    memcpy(frame->data[0], &data[0], y_size);          // Y
    memcpy(frame->data[1], &data[y_size], uv_size);     // U
    memcpy(frame->data[2], &data[y_size + uv_size], uv_size); // V

    // 8. 编码视频帧并获取编码后的数据
    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;

    // 9. 编码并获取数据
    frame->pts = 0;  // 这里设置PTS，可以根据需要递增
    ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending frame to encoder\n");
        return -1;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(ctx, &pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            return -1;
        }

        // 10. 将编码后的数据包转换为 std::vector<uint8_t> payload
        std::vector<uint8_t> payload(pkt.data, pkt.data + pkt.size);
        
        // 11. 调用自定义接口将编码后的数据推送出去
        send_payload(payload);

        av_packet_unref(&pkt);
    }

    // 12. 释放资源
    avcodec_free_context(&ctx);
    av_frame_free(&frame);

    return 0;
}
