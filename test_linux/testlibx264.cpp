#include <iostream>
#include <x264.h>
#include <fstream>
#include <vector>
#include <cstring> // for memcpy

using namespace std;

// 读取 YUV 数据（假设是 YUV420 格式）
bool readYUV(const string &filename, vector<uint8_t> &data, int width, int height) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Failed to open YUV file!" << endl;
        return false;
    }

    // YUV420 格式：Y 宽高为 (width, height)，U 和 V 宽高为 (width/2, height/2)
    int frame_size = width * height * 3 / 2;  // Y + U + V
    data.resize(frame_size);
    file.read(reinterpret_cast<char*>(data.data()), frame_size);
    return file.gcount() == frame_size;
}

int main() {
    // 视频参数
    int width = 1920;   // 视频宽度
    int height = 1080;  // 视频高度
    string input_file = "input.yuv"; // 输入的 YUV 文件

    // 初始化 x264 编码器
    x264_t *encoder = nullptr;
    x264_param_t param;
    x264_param_default(&param);
    param.i_width = width;
    param.i_height = height;
    param.i_csp = X264_CSP_I420;  // YUV420 格式
    param.i_fps_num = 30;         // 帧率为 30fps
    param.i_fps_den = 1;
    param.i_bitdepth = 8;         // 8-bit 色深
    param.b_vfr_input = 0;        // 固定帧率输入

    // 设置编码质量（可以根据需求调整）
    param.i_level_idc = 40;  // H.264 Level 4.0

    // 创建编码器
    encoder = x264_encoder_open(&param);
    if (!encoder) {
        cerr << "Failed to open x264 encoder!" << endl;
        return -1;
    }

    // 读取输入的 YUV 数据
    vector<uint8_t> yuv_data;
    if (!readYUV(input_file, yuv_data, width, height)) {
        return -1;
    }

    // 创建并设置 x264_picture_t
    x264_picture_t pic_in, pic_out;
    x264_picture_alloc(&pic_in, X264_CSP_I420, width, height);
    memcpy(pic_in.img.plane[0], yuv_data.data(), width * height);  // Y 分量
    memcpy(pic_in.img.plane[1], yuv_data.data() + width * height, width * height / 4);  // U 分量
    memcpy(pic_in.img.plane[2], yuv_data.data() + width * height + width * height / 4, width * height / 4);  // V 分量

    // 打开输出文件
    ofstream output("output.h264", ios::binary);

    // 编码并写入 H.264 数据
    for (int i = 0; i < 100; ++i) {  // 假设我们处理100帧
        x264_nal_t* nals = nullptr;
        int i_nal = 0;
        int result = x264_encoder_encode(encoder, &nals, &i_nal, &pic_in, &pic_out);
        if (result < 0) {
            cerr << "Failed to encode frame!" << endl;
            break;
        }

        // 将编码后的数据写入文件
        for (int j = 0; j < i_nal; ++j) {
            output.write(reinterpret_cast<char*>(nals[j].p_payload), nals[j].i_payload);
        }
    }

    // 清理
    x264_encoder_close(encoder);
    x264_picture_clean(&pic_in);
    output.close();

    cout << "Encoding completed!" << endl;
    return 0;
}
