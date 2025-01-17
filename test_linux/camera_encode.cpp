#include "Rpicamera.h"
#include "Encoder.h"

#include <iostream>
#include <fstream>
std::ofstream file("video.h264", std::ios::binary);

void encodercallback(const std::vector<uint8_t> &data)
{
    std::cout << "encodedlength: " << data.size() << std::endl;
    //  打开文件以写入二进制数据

    // 写入数据到文件
    file.write(reinterpret_cast<const char *>(data.data()), data.size());

    // 检查写入是否成功
    /* if (!file)
    {
        std::cerr << "写入文件失败!" << std::endl;
    }
    else
    {
        std::cout << "数据成功写入 " << "video.h264" << std::endl;
    } */
}

Encoder *encoder;

void cameracallback(std::vector<unsigned char> &data)
{
    int bufflength = data.size();
    // std::cout << "bufflength: " << bufflength << std::endl;
    encoder->Async_encode(data);
}

int main()
{
    Rpicamera camera;
    encoder = new Encoder{encodercallback};
    camera.start(cameracallback);
    std::this_thread::sleep_for(std::chrono::seconds(15));
    camera.stop();
    std::cout << "camera stopped!" << std::endl;
    delete encoder;
    file.close();
    return 0;
}