/* #include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h> */
#include <iostream>
#include <cstring>
#include "HMC5883L.h"

int main()
{
    int file;
    const char *device = "/dev/i2c-1";
    int addr = 0x1E; // HMC5883L I2C address

    // 打开I2C设备
    if ((file = open(device, O_RDWR)) < 0)
    {
        std::cerr << "Failed to open the i2c bus" << std::endl;
        return 1;
    }
    try
    {
        HMC5883L campass{file};
        double result[3];
        while (1)
        {
            campass.sample(result);
            std::cout << "X: " << result[0] << ", Y: " << result[1] << ", Z: " << result[2] << std::endl;
            sleep(0.1);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    // 关闭设备
    close(file);
    return 0;
}
