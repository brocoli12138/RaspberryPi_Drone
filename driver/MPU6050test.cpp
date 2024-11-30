#include "MPU6050.h"
#include <iostream>
#include <cmath>

int main()
{
    int file;
    const char *device = "/dev/i2c-1";
    int addr = 0x1E; // HMC5883L I2C address
    try
    {
        // 打开I2C设备
        if ((file = open(device, O_RDWR)) < 0)
        {
            std::cerr << "Failed to open the i2c bus" << std::endl;
            return 1;
        }

        MPU6050 mpu{file};
        double result[7];

        while (1)
        {
            mpu.sample(result);
            std::cout << result[0] << ' ' << result[1] << ' ' << result[2] << '\n';
            std::cout << std::pow(result[0] * result[0] + result[1] * result[1] + result[2] * result[2], 0.5) << std::endl;
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
