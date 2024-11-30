// BMP280.h
#ifndef __BMP280_H__
#define __BMP280_H__

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstdint>

#define BMP280_ADDR 0x76
#define BMP280_PRESS_MSB 0xF7
#define BMP280_CTRL_MEAS 0xF4
#define BMP280_DIG_T1 0x88

class BMP280
{
private:
    int _fd;
    int32_t _tfine;
    uint16_t _dig_T1, _dig_P1;
    int16_t _dig_T2, _dig_T3, _dig_P2, _dig_P3, _dig_P4, _dig_P5, _dig_P6, _dig_P7, _dig_P8, _dig_P9;

public:
    BMP280(int filediscription);
    void sample(double *result);
    ~BMP280();
};

BMP280::BMP280(int filediscription) : _fd(filediscription)
{
    if (ioctl(_fd, I2C_SLAVE, BMP280_ADDR) < 0)
    {
        throw "Failed to connect to the sensor";
    }

    // Initialize the BMP280
    uint8_t pwr[3] = {BMP280_CTRL_MEAS, 0x57, 0x10};
    if (write(_fd, pwr, 3) != 3)
    {
        throw "Failed to initialize the BMP280";
    }

    // 读取校准数据
    char dataBuf[24];
    uint8_t regAddr = BMP280_DIG_T1;
    if (write(_fd, &regAddr, 1) != 1)
    {
        throw "Failed to write to the register sample";
    }

    if (read(_fd, dataBuf, 24) != 24)
    {
        throw "Failed to read the sensor data";
    }

    _dig_T1 = (dataBuf[1] << 8) | dataBuf[0];
    _dig_T2 = (dataBuf[3] << 8) | dataBuf[2];
    _dig_T3 = (dataBuf[5] << 8) | dataBuf[4];

    _dig_P1 = (dataBuf[7] << 8) | dataBuf[6];
    _dig_P2 = (dataBuf[9] << 8) | dataBuf[8];
    _dig_P3 = (dataBuf[11] << 8) | dataBuf[10];

    _dig_P4 = (dataBuf[13] << 8) | dataBuf[12];
    _dig_P5 = (dataBuf[15] << 8) | dataBuf[14];
    _dig_P6 = (dataBuf[17] << 8) | dataBuf[16];

    _dig_P7 = (dataBuf[19] << 8) | dataBuf[18];
    _dig_P8 = (dataBuf[21] << 8) | dataBuf[20];
    _dig_P9 = (dataBuf[23] << 8) | dataBuf[22];
}

void BMP280::sample(double *result)
{
    if (ioctl(_fd, I2C_SLAVE, BMP280_ADDR) < 0)
    {
        throw "Failed to connect to the sensor";
    }
    // 读取原始气压温度数据
    char dataBuf[8];
    uint8_t regAddr = BMP280_PRESS_MSB;
    if (write(_fd, &regAddr, 1) != 1)
    {
        throw "Failed to write to the register sample";
    }
    if (read(_fd, dataBuf, 8) != 8)
    {
        throw "Failed to read the sensor data";
    }

    int adc_T = (dataBuf[3] << 12) | (dataBuf[4] << 4) | (dataBuf[5] >> 4);
    int adc_P = (dataBuf[0] << 12) | (dataBuf[1] << 4) | (dataBuf[2] >> 4);

    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)_dig_T1 << 1))) * ((int32_t)_dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)_dig_T1)) * ((adc_T >> 4) - ((int32_t)_dig_T1))) >> 12) * ((int32_t)_dig_T3)) >> 14;
    _tfine = var1 + var2;
    T = (_tfine * 5 + 128) >> 8;
    result[0] = (float)T / 100;

    // 计算补偿后的气压
    int64_t p;
    var1 = ((int64_t)_tfine) - 128000;
    var2 = var1 * var1 * (int64_t)_dig_P6;
    var2 = var2 + ((var1 * (int64_t)_dig_P5) << 17);
    var2 = var2 + (((int64_t)_dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)_dig_P3) >> 8) + ((var1 * (int64_t)_dig_P5) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)_dig_P1) >> 33;
    if (var1 == 0)
    {
        result[1] = 0; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)_dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)_dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)_dig_P7) << 4);

    // 输出气压，单位为帕斯卡
    result[1] = (uint32_t)(p / 256);
}

BMP280::~BMP280()
{
}

#endif