#include "Rpicamera.h"
#include <vector>


int main()
{
    Rpicamera camera;
    camera.start();
    // Process requests (this would typically be handled in a loop or separate thread)
    std::this_thread::sleep_for(std::chrono::seconds(30));
    camera.stop();

    return 0;
}