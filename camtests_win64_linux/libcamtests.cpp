#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <libcamera/libcamera.h>

using namespace libcamera;
using namespace std::chrono_literals;

static void requestComplete(Request *request);

int main()
{
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();

    // List available cameras
    for (auto const &camera : cm->cameras())
        std::cout << camera->id() << std::endl;

    auto cameras = cm->cameras();
    if (cameras.empty())
    {
        std::cout << "No cameras were identified on the system."
                  << std::endl;
        cm->stop();
        return EXIT_FAILURE;
    }

    std::string cameraId = cameras[0]->id();

    // Get the first available camera
    auto camera = cm->get(cameraId);
    camera->acquire();

    // Generate and configure camera settings
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration({StreamRole::Viewfinder});
    StreamConfiguration &streamConfig = config->at(0);
    std::cout << "Default viewfinder configuration is: " << streamConfig.toString() << std::endl;

    // Set resolution to 640x480
    streamConfig.size.width = 640;
    streamConfig.size.height = 480;
    config->validate();
    std::cout << "Validated viewfinder configuration is: " << streamConfig.toString() << std::endl;
    camera->configure(config.get());

    // Allocate frame buffers
    FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);
    for (StreamConfiguration &cfg : *config)
    {
        int ret = allocator->allocate(cfg.stream());
        if (ret < 0)
        {
            std::cerr << "Can't allocate buffers" << std::endl;
            return -ENOMEM;
        }

        size_t allocated = allocator->buffers(cfg.stream()).size();
        std::cout << "Allocated " << allocated << " buffers for stream" << std::endl;
    }

    Stream *stream = streamConfig.stream();
    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
    std::vector<std::unique_ptr<Request>> requests;
    for (unsigned int i = 0; i < buffers.size(); ++i)
    {
        std::unique_ptr<Request> request = camera->createRequest();
        if (!request)
        {
            std::cerr << "Can't create request" << std::endl;
            return -ENOMEM;
        }

        const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
        int ret = request->addBuffer(stream, buffer.get());
        if (ret < 0)
        {
            std::cerr << "Can't set buffer for request"
                      << std::endl;
            return ret;
        }

        requests.push_back(std::move(request));
    }

    camera->requestCompleted.connect(requestComplete);

    // Start and queue requests
    camera->start();
    for (std::unique_ptr<Request> &request : requests)
        camera->queueRequest(request.get());

    // Process requests (this would typically be handled in a loop or separate thread)
    std::this_thread::sleep_for(5s);

    // Stop the camera and release resources
    camera->stop();
    allocator->free(stream);
    delete allocator;
    camera->release();
    camera.reset();
    cm->stop();

    return 0;
}

void requestComplete(Request *request)
{
    if (request->status() == Request::RequestCancelled)
        return;
    const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();
    for (auto bufferPair : buffers)
    {
        FrameBuffer *buffer = bufferPair.second;
        const FrameMetadata &metadata = buffer->metadata();
        std::cout << " seq: " << std::setw(6) << std::setfill('0') << metadata.sequence << " bytesused: ";

        unsigned int nplane = 0;
        for (const FrameMetadata::Plane &plane : metadata.planes())
        {
            std::cout << plane.bytesused;
            if (++nplane < metadata.planes().size())
                std::cout << "/";
        }
        std::cout << std::endl;
    }
    request->reuse(Request::ReuseBuffers);
    camera->queueRequest(request);
    return;
}