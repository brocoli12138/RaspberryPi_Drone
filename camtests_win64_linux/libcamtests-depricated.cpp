#include <iomanip>
#include <iostream>
#include <sys/mman.h>
#include <memory>
#include <thread>
#include <vector>
#include <libcamera/libcamera.h>
#include <cstring>

volatile bool isStopped = false;

using namespace libcamera;
static std::shared_ptr<Camera> camera;
std::vector<unsigned char> frameDataBuff;

static void requestComplete(Request *request)
{
	if (request->status() == Request::RequestCancelled)
		return;
	/* 	std::cout << std::endl
				  << "Request completed: " << request->toString() << std::endl;

		const ControlList &requestMetadata = request->metadata();
		for (const auto &ctrl : requestMetadata)
		{
			const ControlId *id = controls::controls.at(ctrl.first);
			const ControlValue &value = ctrl.second;

			std::cout << "\t" << id->name() << " = " << value.toString()
					  << std::endl;
		}

		const Request::BufferMap &buffers = request->buffers();
		for (auto bufferPair : buffers)
		{
			// (Unused) Stream *stream = bufferPair.first;
			FrameBuffer *buffer = bufferPair.second;
			const FrameMetadata &metadata = buffer->metadata();

			// Print some information about the buffer which has completed.
			std::cout << " seq: " << std::setw(6) << std::setfill('0') << metadata.sequence
					  << " timestamp: " << metadata.timestamp
					  << " bytesused: ";

			unsigned int nplane = 0;
			for (const FrameMetadata::Plane &plane : metadata.planes())
			{
				std::cout << plane.bytesused;
				if (++nplane < metadata.planes().size())
					std::cout << "/";
			}

			std::cout << std::endl;


			// * Image data can be accessed here, but the FrameBuffer	must be mapped by the application

		} */

	// Process completed request
	const Request::BufferMap &buffers = request->buffers();
	for (auto bufferPair : buffers)
	{
		// (Unused) Stream *stream = bufferPair.first;
		FrameBuffer *buffer = bufferPair.second;
		if (!buffer)
		{
			continue;
		}

		std::vector<unsigned char> frameDataBuff;
		// Access the buffer data
		for (const FrameBuffer::Plane &plane : buffer->planes())
		{
			void *data = mmap(nullptr, plane.length, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), 0);
			size_t size = plane.length;
			if (data == MAP_FAILED)
			{
				std::cerr << "Failed to map buffer memory" << std::endl;
				continue;
			}

			// Example: Process frame data (e.g., save to file, analyze, etc.)
			// std::cout << "Frame captured, size: " << plane.length << " bytes" << std::endl
			std::vector<unsigned char> planedata{size};
			memcpy(planedata.data(), data, size);
			frameDataBuff.insert(frameDataBuff.end(), planedata.begin(), planedata.end());
			munmap(data, plane.length);
		}
	}

	/* Re-queue the Request to the camera. */
	request->reuse(Request::ReuseBuffers);
	if (!isStopped)
	{
		camera->queueRequest(request);
	}
}

std::string cameraName(Camera *camera)
{
	const ControlList &props = camera->properties();
	std::string name;

	const auto &location = props.get(properties::Location);
	if (location)
	{
		switch (*location)
		{
		case properties::CameraLocationFront:
			name = "Internal front camera";
			break;
		case properties::CameraLocationBack:
			name = "Internal back camera";
			break;
		case properties::CameraLocationExternal:
			name = "External camera";
			const auto &model = props.get(properties::Model);
			if (model)
				name = " '" + *model + "'";
			break;
		}
	}

	name += " (" + camera->id() + ")";

	return name;
}

int main()
{

	std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
	cm->start();

	for (auto const &camera : cm->cameras())
		std::cout << " - " << cameraName(camera.get()) << std::endl;

	if (cm->cameras().empty())
	{
		std::cout << "No cameras were identified on the system."
				  << std::endl;
		cm->stop();
		return EXIT_FAILURE;
	}

	std::string cameraId = cm->cameras()[0]->id();
	camera = cm->get(cameraId);
	camera->acquire();

	std::unique_ptr<CameraConfiguration> config =
		camera->generateConfiguration({StreamRole::Viewfinder});

	StreamConfiguration &streamConfig = config->at(0);
	std::cout << "Default viewfinder configuration is: "
			  << streamConfig.toString() << std::endl;

#if 0
	streamConfig.size.width = 0; //4096
	streamConfig.size.height = 0; //2560

	int ret = camera->configure(config.get());
	if (ret) {
		std::cout << "CONFIGURATION FAILED!" << std::endl;
		return EXIT_FAILURE;
	}
#endif

	config->validate();
	std::cout << "Validated viewfinder configuration is: "
			  << streamConfig.toString() << std::endl;

	camera->configure(config.get());

	FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);

	for (StreamConfiguration &cfg : *config)
	{
		int ret = allocator->allocate(cfg.stream());
		if (ret < 0)
		{
			std::cerr << "Can't allocate buffers" << std::endl;
			return EXIT_FAILURE;
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
			return EXIT_FAILURE;
		}

		const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
		int ret = request->addBuffer(stream, buffer.get());
		if (ret < 0)
		{
			std::cerr << "Can't set buffer for request"
					  << std::endl;
			return EXIT_FAILURE;
		}

		ControlList &controls = request->controls();
		controls.set(controls::Brightness, 0.5);

		requests.push_back(std::move(request));
	}

	camera->requestCompleted.connect(requestComplete);

	camera->start();
	for (std::unique_ptr<Request> &request : requests)
		camera->queueRequest(request.get());

	// Process requests (this would typically be handled in a loop or separate thread)
	std::this_thread::sleep_for(std::chrono::seconds(5));

	isStopped = true;
	camera->stop();
	allocator->free(stream);
	delete allocator;
	camera->release();
	camera.reset();
	cm->stop();

	return EXIT_SUCCESS;
}
