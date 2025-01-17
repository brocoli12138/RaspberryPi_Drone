#include <libcamera/libcamera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <sys/mman.h>
#include <memory>
#include <chrono>
#include <thread>

using namespace libcamera;
#define WIDTH 640
#define HEIGHT 480


void write_bmp(const char *filename, const void *buffer, int width, int height)
{
    // Create BMP file header
    unsigned char bmpfileheader[14] = {
        'B', 'M',   // Magic number for file
        0, 0, 0, 0, // Size of the BMP file
        0, 0,       // Reserved
        0, 0,       // Reserved
        54, 0, 0, 0 // Start position of pixel data (54 bytes)
    };

    // Create BMP info header
    unsigned char bmpinfoheader[40] = {
        40, 0, 0, 0, // Size of the info header
        0, 0, 0, 0,  // Width of the image
        0, 0, 0, 0,  // Height of the image
        1, 0,        // Number of color planes
        24, 0,       // Bits per pixel (24 for RGB24)
        0, 0, 0, 0,  // Compression (no compression)
        0, 0, 0, 0,  // Size of the raw bitmap data
        0, 0, 0, 0,  // Print resolution horizontal
        0, 0, 0, 0,  // Print resolution vertical
        0, 0, 0, 0,  // Number of colors in the palette
        0, 0, 0, 0   // Important colors
    };

    int filesize = 54 + 3 * width * height;
    bmpfileheader[2] = (unsigned char)(filesize);
    bmpfileheader[3] = (unsigned char)(filesize >> 8);
    bmpfileheader[4] = (unsigned char)(filesize >> 16);
    bmpfileheader[5] = (unsigned char)(filesize >> 24);

    bmpinfoheader[4] = (unsigned char)(width);
    bmpinfoheader[5] = (unsigned char)(width >> 8);
    bmpinfoheader[6] = (unsigned char)(width >> 16);
    bmpinfoheader[7] = (unsigned char)(width >> 24);
    bmpinfoheader[8] = (unsigned char)(height);
    bmpinfoheader[9] = (unsigned char)(height >> 8);
    bmpinfoheader[10] = (unsigned char)(height >> 16);
    bmpinfoheader[11] = (unsigned char)(height >> 24);

    std::ofstream file(filename, std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<char *>(bmpfileheader), 14);
    file.write(reinterpret_cast<char *>(bmpinfoheader), 40);
    file.write(reinterpret_cast<const char *>(buffer), 3 * width * height);
    file.close();
}

void requestComplete(Request *request)
{
    if (request->status() == Request::RequestComplete)
    {
        const FrameBuffer *buffer = request->buffers().begin()->second;
        const FrameMetadata &metadata = buffer->metadata();
        const FrameBuffer::Plane &plane = buffer->planes()[0];

        // Access the buffer data directly
        void *data = mmap(nullptr, plane.length, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), 0);
        ;
        write_bmp("output.bmp", data, WIDTH, HEIGHT);
        munmap(data, plane.length);
    }
}

int main()
{
    // Initialize libcamera
    std::unique_ptr<CameraManager> camera_manager = std::make_unique<CameraManager>();
    camera_manager->start();

    if (camera_manager->cameras().empty())
    {
        std::cerr << "No cameras available" << std::endl;
        return -1;
    }

    std::shared_ptr<Camera> camera = camera_manager->cameras()[0];
    camera->acquire();

    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration({StreamRole::StillCapture});
    config->at(0).pixelFormat = PixelFormat::fromString("RGB888");
    config->at(0).size.width = WIDTH;
    config->at(0).size.height = HEIGHT;
    config->validate();
    camera->configure(config.get());

    std::unique_ptr<FrameBufferAllocator> allocator = std::make_unique<FrameBufferAllocator>(camera);
    allocator->allocate(config->at(0).stream());

    std::vector<std::unique_ptr<Request>> requests;
    for (unsigned int i = 0; i < config->at(0).bufferCount; ++i)
    {
        std::unique_ptr<Request> request = camera->createRequest();
        request->addBuffer(config->at(0).stream(), allocator->buffers(config->at(0).stream())[i].get());
        requests.push_back(std::move(request));
    }
    camera->requestCompleted.connect(requestComplete);
    camera->start();

    for (auto &request : requests)
    {
        camera->queueRequest(request.get());
    }

    // Wait for request completion
    std::this_thread::sleep_for(std::chrono::seconds(2));

    camera->stop();
    camera->release();
    camera_manager->stop();

    return 0;
}