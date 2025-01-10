#include <libcamera/libcamera.h>
#include <iostream>
#include <fstream>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>

using namespace libcamera;
using namespace std;

int main()
{
    // 1. 初始化libcamera
    CameraManager cm;
    cm.start();
    shared_ptr<Camera> camera = cm.get("libcamera");
    if (!camera)
    {
        cerr << "无法打开摄像头" << endl;
        return -1;
    }

    // 2. 配置相机
    camera->acquire();
    unique_ptr<CameraConfiguration> config = camera->generateConfiguration({StreamRole::VideoRecording});
    StreamConfiguration &streamConfig = config->at(0);
    streamConfig.size.width = 1920; // 例如 1920x1080 分辨率
    streamConfig.size.height = 1080;
    streamConfig.pixelFormat = PixelFormat::fromString("YUV420"); // YUV格式，适合视频编码

    // 3. 启动相机流
    camera->configure(config.get());
    FrameBufferAllocator allocator(camera);
    for (StreamConfiguration &cfg : *config)
        allocator.allocate(cfg.stream());
    camera->start();

    // 4. 配置H.264硬件编码器 (使用V4L2接口)
    int video_fd = open("/dev/video11", O_RDWR); // 树莓派上V4L2硬件编码器
    if (video_fd == -1)
    {
        cerr << "无法打开视频设备" << endl;
        return -1;
    }

    struct v4l2_format format;
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    format.fmt.pix.width = 1920;
    format.fmt.pix.height = 1080;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_H264; // 设置为H.264格式
    ioctl(video_fd, VIDIOC_S_FMT, &format);

    // 5. 捕获和编码帧
    ofstream outputFile("output.h264", ios::binary); // 保存编码后的视频文件
    for (int i = 0; i < 300; i++)
    { // 假设获取300帧
        std::unique_ptr<libcamera::Request> request = camera->createRequest();
        camera->queueRequest(request);

        // 获取帧数据
        FrameBuffer *buffer = request->buffers().begin()->second;
        const uint8_t *data = static_cast<const uint8_t *>(buffer->planes()[0].data());

        // 编码并写入文件
        write(video_fd, data, buffer->planes()[0].length);
        uint8_t encoded_frame[buffer->planes()[0].length];
        read(video_fd, encoded_frame, sizeof(encoded_frame));
        outputFile.write(reinterpret_cast<const char *>(encoded_frame), sizeof(encoded_frame));
    }

    // 6. 清理资源
    outputFile.close();
    close(video_fd);
    camera->stop();
    camera->release();
    cm.stop();

    return 0;
}
