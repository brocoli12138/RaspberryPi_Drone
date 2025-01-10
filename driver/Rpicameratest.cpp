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
    return 0;
}