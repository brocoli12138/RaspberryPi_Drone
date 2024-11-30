#ifndef __HMC5883L_H__
#define __HMC5883L_H__

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstdint>

#define HMC_ADDR 0x1E
#define HMC_CONFIG_REG_A 0x00
#define HMC_MEG_XOUTH_REG 0x03

class HMC5883L
{
private:
    int _fd;

public:
    HMC5883L(int filediscription);
    void sample(double *result);
    ~HMC5883L();
};

HMC5883L::HMC5883L(int filediscription) : _fd(filediscription)
{
    if (ioctl(_fd, I2C_SLAVE, HMC_ADDR) < 0)
    {
        throw "Failed to connect to the sensor";
    }
    // Initialize the HMC5883L
    uint8_t cfg[4] = {HMC_CONFIG_REG_A, 0x70, 0x00, 0x00};
    if (write(_fd, cfg, 4) != 4)
    {
        throw "Failed to write to the register mode";
    }
}

void HMC5883L::sample(double *result)
{
    if (ioctl(_fd, I2C_SLAVE, HMC_ADDR) < 0)
    {
        throw "Failed to connect to the sensor";
    }
    // Read 6 bytes of data
    char dataBuf[6];
    uint8_t regAddr = HMC_MEG_XOUTH_REG;
    if (write(_fd, &regAddr, 1) != 1)
    {
        throw "Failed to locate the sensor data reg";
    }

    if (read(_fd, dataBuf, 6) != 6)
    {
        throw "Failed to read the sensor data";
    }

    // Convert and output the accelerometer data
    int16_t megn_x = (dataBuf[0] << 8) | dataBuf[1];
    int16_t megn_y = (dataBuf[2] << 8) | dataBuf[3];
    int16_t megn_z = (dataBuf[4] << 8) | dataBuf[5];

    result[0] = megn_x;
    result[1] = megn_y;
    result[2] = megn_z;
    return;
}

HMC5883L::~HMC5883L()
{
}

#endif