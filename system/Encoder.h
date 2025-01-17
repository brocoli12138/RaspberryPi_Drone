#include <x264.h>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <future>
#include <thread>
#include <iostream>

#define WIDTH 640
#define HEIGTH 480

class Encoder
{
private:
    x264_param_t _param;
    x264_t *_encoder;
    std::function<void(const std::vector<uint8_t> &)> _callback;
    x264_picture_t _current_raw, _current_output;

protected:
    std::vector<uint8_t> sync_encode(const std::vector<uint8_t> &data);

public:
    Encoder(std::function<void(const std::vector<uint8_t> &)> callback);
    void Async_encode(const std::vector<unsigned char> &data);
    ~Encoder();
};

Encoder::Encoder(std::function<void(const std::vector<uint8_t> &)> callback) : _param(), _encoder(nullptr), _callback(callback)
{
    x264_param_default(&_param);
    _param.i_width = WIDTH;
    _param.i_height = HEIGTH;
    _param.i_csp = X264_CSP_I420; // YUV420 格式
    _param.i_fps_num = 30;        // 帧率为 30fps
    _param.i_fps_den = 1;
    _param.i_bitdepth = 8;                // 8-bit 色深
    _param.b_vfr_input = 0;               // 固定帧率输入
    _param.i_log_level = X264_LOG_NONE;   // 去除日志信息
    _param.b_sliced_threads = 0;          // 禁用 SEI 生成
    _param.i_nal_hrd = X264_NAL_HRD_NONE; // 不生成 HRD（Hypothetical Reference Decoder）信息
    _param.b_annexb = 1;                  // 使用 Annex-B 格式（可自动添加 NAL 单元头）

    // 设置编码质量（可以根据需求调整）
    _param.i_level_idc = 40; // H.264 Level 4.0

    // 创建编码器
    _encoder = x264_encoder_open(&_param);
    if (!_encoder)
    {
        throw std::runtime_error("Failed to open x264 encoder!");
    }

    x264_picture_alloc(&_current_raw, X264_CSP_I420, WIDTH, HEIGTH);
    memset(&_current_output, 0, sizeof(_current_output)); // 初始化 _current_output
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

std::vector<uint8_t> Encoder::sync_encode(const std::vector<uint8_t> &data)
{
    std::vector<uint8_t> encodedData;
    int i_nal = 0;
    x264_nal_t *nals = nullptr;
    int result = 0;
    // 编码当前帧
    if (data.size() != 0)
    {                                                                                                             // 填充 YUV 数据
        memcpy(_current_raw.img.plane[0], data.data(), WIDTH * HEIGTH);                                           // Y 分量
        memcpy(_current_raw.img.plane[1], data.data() + WIDTH * HEIGTH, WIDTH * HEIGTH / 4);                      // U 分量
        memcpy(_current_raw.img.plane[2], data.data() + WIDTH * HEIGTH + WIDTH * HEIGTH / 4, WIDTH * HEIGTH / 4); // V 分量
        result = x264_encoder_encode(_encoder, &nals, &i_nal, &_current_raw, &_current_output);
    }
    else
    {
        std::cout << "the end of stream!" << std::endl;
        result = x264_encoder_encode(_encoder, &nals, &i_nal, nullptr, &_current_output);
    }
    if (result < 0)
    {
        return {};
    }
    // 将编码后的 NAL 单元数据追加到 std::vector 中
    for (int j = 0; j < i_nal; ++j)
    {
        if (nals[j].i_type == NAL_SEI)
        {
            continue; // 跳过 SEI 和 AUD 单元
        }
        encodedData.insert(encodedData.end(), nals[j].p_payload, nals[j].p_payload + nals[j].i_payload);
    }
    return encodedData;
}

Encoder::~Encoder()
{
    // 刷新缓冲区，输出所有帧
    Async_encode({});
    // 清理
    if (_encoder)
    {
        x264_encoder_close(_encoder);
    }
    x264_picture_clean(&_current_raw);
}
