#include "core/rpicam_encoder.hpp"
#include "output/output.hpp"
#include <chrono>
#include <atomic>
#include <thread>
using namespace std::placeholders;

int argc = 13;
// const char *argv[]{"rpicam-vid", "--inline", "-l", "-o", "tcp://0.0.0.0:1234",
//                    "--width", "1920", "--height", "1080", "--framerate", "120", "-t", "0"};

char *argv[] = {
    const_cast<char *>("rpicam-vid"),
    const_cast<char *>("--inline"),
    const_cast<char *>("-l"),
    const_cast<char *>("-o"),
    const_cast<char *>("tcp://0.0.0.0:1234"),
    const_cast<char *>("--width"),
    const_cast<char *>("1920"),
    const_cast<char *>("--height"),
    const_cast<char *>("1080"),
    const_cast<char *>("--framerate"),
    const_cast<char *>("120"),
    const_cast<char *>("-t"),
    const_cast<char *>("0")};
// rpicam-vid --inline -l -o tcp://0.0.0.0:1234 --width 1920 --height 1080 --framerate 120 -t 0

static int get_colourspace_flags(std::string const &codec)
{
    if (codec == "mjpeg" || codec == "yuv420")
        return RPiCamEncoder::FLAG_VIDEO_JPEG_COLOURSPACE;
    else
        return RPiCamEncoder::FLAG_VIDEO_NONE;
}

class Rpicamera : public RPiCamEncoder
{
private:
    std::atomic<bool> _key;
    VideoOptions *_options;
    std::unique_ptr<Output> _output;
    std::chrono::_V2::system_clock::time_point _start_time;
    std::vector<std::thread> _threads;
    void event_loop();

public:
    Rpicamera(/* args */);
    void start();
    void stop()
    {
        _key = false;
        _threads[0].join();
    };
    ~Rpicamera();
};

Rpicamera::Rpicamera(/* args */) : RPiCamEncoder(), _key(true), _options(GetOptions()),
                                   _output(std::unique_ptr<Output>(Output::Create(_options)))
{
    _options->Parse(argc, argv);
    SetEncodeOutputReadyCallback(std::bind(&Output::OutputReady, _output.get(), _1, _2, _3, _4));
    SetMetadataReadyCallback(std::bind(&Output::MetadataReady, _output.get(), _1));

    OpenCamera();
    ConfigureVideo(get_colourspace_flags(_options->codec));
    StartEncoder();
    StartCamera();
    _start_time = std::chrono::high_resolution_clock::now();
}

void Rpicamera::start()
{
    std::cout << "Camera is running!" << std::endl;
    if (_threads.size() == 0)
    {
        _threads.push_back(std::thread{&Rpicamera::event_loop, this});
    }
}

void Rpicamera::event_loop()
{
    for (unsigned int count = 0;; count++)
    {
        // std::cout << _key << std::endl;
        RPiCamEncoder::Msg msg = Wait();
        if (msg.type == RPiCamApp::MsgType::Timeout)
        {
            LOG_ERROR("ERROR: Device timeout detected, attempting a restart!!!");
            StopCamera();
            StartCamera();
            continue;
        }
        if (msg.type == RPiCamEncoder::MsgType::Quit)
            return;
        else if (msg.type != RPiCamEncoder::MsgType::RequestComplete)
            throw std::runtime_error("unrecognised message!");
        if (_key == false)
            _output->Signal();

        LOG(2, "Viewfinder frame " << count);
        auto now = std::chrono::high_resolution_clock::now();
        bool timeout = !_options->frames && _options->timeout &&
                       ((now - _start_time) > _options->timeout.value);
        bool frameout = _options->frames && count >= _options->frames;
        if (timeout || frameout || _key == false)
        {
            if (timeout)
                LOG(1, "Halting: reached timeout of " << _options->timeout.get<std::chrono::milliseconds>()
                                                      << " milliseconds.");
            StopCamera(); // stop complains if encoder very slow to close
            StopEncoder();
            return;
        }
        CompletedRequestPtr &completed_request = std::get<CompletedRequestPtr>(msg.payload);
        EncodeBuffer(completed_request, VideoStream());
    }
}

Rpicamera::~Rpicamera()
{
}
