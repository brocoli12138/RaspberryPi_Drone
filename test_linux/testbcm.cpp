#include <fcntl.h>
#include <iostream>
#include <linux/videodev2.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

enum class Platform
{
    VC4,
    UNKNOWN
};

Platform get_platform()
{
    for (unsigned int device_num = 0; device_num < 256; device_num++)
    {
        char device_name[16];
        snprintf(device_name, sizeof(device_name), "/dev/video%u", device_num);
        int fd = open(device_name, O_RDWR, 0);
        if (fd < 0)
            continue;

        v4l2_capability caps;
        if (ioctl(fd, VIDIOC_QUERYCAP, &caps) == 0)
        {
            std::cout << (char *)caps.card << std::endl;
            if (!strncmp((char *)caps.card, "bcm2835-isp", sizeof(caps.card)))
            {
                close(fd);
                return Platform::VC4;
            }
        }
        close(fd);
    }
    return Platform::UNKNOWN;
}

int main()
{
    Platform platform = get_platform();
    if (platform == Platform::VC4)
    {
        std::cout << "Platform is VC4" << std::endl;
    }
    else
    {
        std::cout << "Platform is UNKNOWN" << std::endl;
    }
    return 0;
}