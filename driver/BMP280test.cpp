#include "BMP280.h"
#include <iostream>

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
        BMP280 barometer{file};
        int i = 0;
        double result[2];
        while (1)
        {

            barometer.sample(result);

            std::cout << i << " temperature: " << result[0] << " pressure: " << result[1] << "\n";

            sleep(0.1);
            i++;
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
