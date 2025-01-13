#include "Rpicamera.h"
#include <vector>

std::vector<std::vector<unsigned char>> frameDataList;

void mycallback(std::vector<unsigned char> data)
{
    frameDataList.push_back(data);
    std::cout << "pushbacked!" << std::endl;
}

int main()
{
    Rpicamera camera;
    camera.start(mycallback);
    // Process requests (this would typically be handled in a loop or separate thread)
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}