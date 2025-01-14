#ifndef __RPI_CAMERA_H__
#define __RPI_CAMERA_H__

#include <iomanip>
#include <iostream>
#include <sys/mman.h>
#include <memory>
#include <thread>
#include <vector>
#include <libcamera/libcamera.h>
#include <cstring>

using namespace libcamera;
static unsigned int DFT_BUFFER_COUNT = 6;
class Rpicamera
{
private:
    std::unique_ptr<CameraManager> _cameraManager;
    std::shared_ptr<Camera> _camera;
    std::unique_ptr<CameraConfiguration> _cameraConfiguration;
    FrameBufferAllocator *_allocator;
    std::vector<std::unique_ptr<Request>> _requests;
    volatile bool _isStopped;
    void (*_callback)(std::vector<unsigned char> &);
    void requestComplete(Request *request);

public:
    Rpicamera(const int n = 0);
    void start(void (*callback)(std::vector<unsigned char> &));
    void stop() { _isStopped = true; };
    ~Rpicamera();
};

Rpicamera::Rpicamera(const int n /* = 0 */)
{
    _callback = nullptr;
    _isStopped = false;
    _cameraManager = std::make_unique<CameraManager>();
    _cameraManager->start();

    if (_cameraManager->cameras().empty())
    {
        std::cout << "No cameras were identified on the system."
                  << std::endl;
        _cameraManager->stop();
        return;
    }

    _camera = _cameraManager->get(_cameraManager->cameras()[n]->id());
    if (_camera->acquire() < 0)
    {
        _cameraManager->stop();
        return;
    }
    // configure the camera
    _cameraConfiguration = _camera->generateConfiguration({StreamRole::Viewfinder});

    StreamConfiguration &streamConfig = _cameraConfiguration->at(0);
    streamConfig.size.width = 800;  // 4096
    streamConfig.size.height = 600; // 2560
    streamConfig.bufferCount = DFT_BUFFER_COUNT;
    streamConfig.pixelFormat = PixelFormat::fromString("YUYV");

    // check if all stream configuration is valid
    _cameraConfiguration->validate();
    // bind the configuration to camera
    if (_camera->configure(_cameraConfiguration.get()))
    {
        std::cout << "CONFIGURATION FAILED!" << std::endl;
        return;
    }
    _allocator = new FrameBufferAllocator(_camera);
    // get a stream object
    Stream *stream = streamConfig.stream();
    if (_allocator->allocate(stream) < 0)
    {
        std::cerr << "Can't allocate buffers" << std::endl;
        return;
    }

    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = _allocator->buffers(stream);
    for (unsigned int i = 0; i < buffers.size(); ++i)
    {
        std::unique_ptr<Request> request = _camera->createRequest();
        if (!request)
        {
            std::cerr << "Can't create request" << std::endl;
            return;
        }

        const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
        int ret = request->addBuffer(stream, buffer.get());
        if (ret < 0)
        {
            std::cerr << "Can't set buffer for request"
                      << std::endl;
            return;
        }

        _requests.push_back(std::move(request));
    }

    _camera->requestCompleted.connect(this, &Rpicamera::requestComplete);
    _camera->start();
    return;
}

void Rpicamera::requestComplete(Request *request)
{
    if (request->status() == Request::RequestCancelled)
        return;

    const Request::BufferMap &buffers = request->buffers();
    const FrameBuffer *buffer = buffers.begin()->second;
    size_t length = 0;
    auto &planes = buffer->planes();
    void *data = nullptr;
    std::vector<unsigned char> aFrameDataBuff;
    for (auto plane : planes)
    {
        data = mmap(nullptr, plane.length, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), 0);
        aFrameDataBuff.resize(length + plane.length);
        memcpy(aFrameDataBuff.data() + length, data, plane.length);
        munmap(data, plane.length);
        length += plane.length;
    }
    _callback(aFrameDataBuff);
    std::cout << "Framedatalength in RPI:" << length << std::endl;

    /* Re-queue the Request to the camera. */
    request->reuse(Request::ReuseBuffers);
    if (!_isStopped)
    {
        _camera->queueRequest(request);
    }
}
void Rpicamera::start(void (*callback)(std::vector<unsigned char> &))
{
    _callback = callback;
    for (std::unique_ptr<Request> &request : _requests)
        _camera->queueRequest(request.get());
}

Rpicamera::~Rpicamera()
{
    _isStopped = true;
    _camera->stop();
    _allocator->free((_cameraConfiguration->at(0)).stream());
    delete _allocator;
    _camera->release();
    _camera.reset();
    _cameraManager->stop();
}

#endif