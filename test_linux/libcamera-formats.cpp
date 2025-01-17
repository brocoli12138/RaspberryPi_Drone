#include <iostream>
#include <libcamera/libcamera.h>

using namespace libcamera;

void listSupportedFormats(std::shared_ptr<Camera> camera) {
    if (!camera) {
        std::cerr << "Invalid camera object!" << std::endl;
        return;
    }

    // 定义要查询的角色
    StreamRole roles[] = { StreamRole::Viewfinder, StreamRole::StillCapture, StreamRole::VideoRecording };

    for (StreamRole role : roles) {
        // 为每个角色生成配置
        std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration({role});
        if (!config) {
            std::cerr << "Failed to generate configuration for role: " << role << std::endl;
            continue;
        }

        // 获取格式信息
        const StreamFormats &formats = config->at(0).formats();

        // 打印角色信息
        std::string roleName;
        switch (role) {
            case StreamRole::Viewfinder:
                roleName = "Viewfinder";
                break;
            case StreamRole::StillCapture:
                roleName = "StillCapture";
                break;
            case StreamRole::VideoRecording:
                roleName = "VideoRecording";
                break;
            default:
                roleName = "Unknown";
                break;
        }
        std::cout << "Supported formats for role: " << roleName << std::endl;

        // 遍历支持的格式
        for (const auto &format : formats.pixelformats()) {
            std::cout << "  Format: " << format.toString() << std::endl;

            // 获取支持的分辨率列表
            const std::vector<Size> &sizes = formats.sizes(format);
            if (sizes.empty()) {
                std::cout << "    No supported resolutions." << std::endl;
                continue;
            }

            for (const Size &size : sizes) {
                std::cout << "    Resolution: " << size.toString() << std::endl;
            }
        }
    }
}

int main() {
    // 创建 CameraManager
    CameraManager manager;
    if (manager.start() != 0) {
        std::cerr << "Failed to start CameraManager!" << std::endl;
        return -1;
    }

    // 获取第一个可用摄像头
    if (manager.cameras().empty()) {
        std::cerr << "No cameras available!" << std::endl;
        manager.stop();
        return -1;
    }

    std::shared_ptr<Camera> camera = manager.cameras()[0];
    if (!camera) {
        std::cerr << "Failed to get camera!" << std::endl;
        manager.stop();
        return -1;
    }

    // 打印支持的格式
    listSupportedFormats(camera);

    // 停止 CameraManager
    manager.stop();
    return 0;
}
