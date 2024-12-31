#ifndef __RPI_cameraH__
#define __RPI_cameraH__

#include <libcamera/libcamera.h>
#include <libcamera/_cameramanager.h>
#include <libcamera/camera.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>
#include <fstream>
#include <iostream>

class Rpicamera : private CameraManager
{
private:
    std::shared_ptr<libcamera::Camera> _camera;
    std::unique_ptr<libcamera::FrameBufferAllocator> _allocator;
    std::unique_ptr<CameraConfiguration> _cameraConfig
        std::atomic<bool>
            _running;
    std::thread _captureThread;

    void captureLoop();

public:
    Rpicamera(/* args */);
    std::ostream &captureVideoStream(std::ostream &output);
    ~Rpicamera();
};

Rpicamera::Rpicamera(/* args */) : CameraManager(), _camera(nullptr), _allocator(nullptr), _cameraConfig(nullptr), _running(false)
{
    // Initialize Camera Manager
    if (start())
    {
        throw std::runtime_error("Failed to start CameraManager.");
    }

    // Get the first available camera
    if (cameras().empty())
    {
        throw std::runtime_error("No cameras available.");
    }
    _camera = cameras()[0];
    if (_camera->acquire())
    {
        throw std::runtime_error("Failed to acquire the camera.");
    }

    // Configure the camera for video recording
    _cameraConfig = _camera->generateConfiguration({libcamera::StreamRole::VideoRecording});
    if (!_cameraConfig || _cameraConfig->validate() != libcamera::CameraConfiguration::Valid)
    {
        throw std::runtime_error("Failed to generate camera configuration.");
    }

    libcamera::StreamConfiguration &streamConfig = _cameraConfig->at(0);
    streamConfig.pixelFormat = formats::H264;

    if (_camera->configure(config.get()) < 0)
    {
        throw std::runtime_error("Failed to configure the camera");
    }

    // Allocate buffers
    _allocator = std::make_unique<libcamera::FrameBufferAllocator>(camera);
    for (libcamera::StreamConfiguration &cfg : *_cameraConfig)
    {
        if (_allocator->allocate(cfg.stream()) < 0)
        {
            throw std::runtime_error("Failed to allocate buffers.");
        }
    }
}

std::ostream &Rpicamera::captureVideoStream(std::ostream &output)
{
    // Start the camera
    if (_camera->start())
    {
        throw std::runtime_error("Failed to start the camera");
    }

    // Start the capture loop
    _running = true;
    _captureThread = std::thread(&VideoStream::captureLoop, this);
}

void Rpicamera::captureLoop()
{
}

Rpicamera::~Rpicamera()
{
}

#endif