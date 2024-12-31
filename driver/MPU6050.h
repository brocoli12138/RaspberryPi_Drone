// MPU6050.h
#ifndef __MPU6050_H__
#define __MPU6050_H__

#include <fcntl.h>  
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstdint>

// 定义MPU6050硬件地址
#define MPU_ADDR 0X68            // 接地为0X68 接高电平为0X69
#define MPU_SAMPLE_RATE_REG 0X19 // 采样频率分频器
#define MPU_ACCEL_XOUTH_REG 0X3B // 加速度值,X轴高8位寄存器
#define MPU_PWR_MGMT1_REG 0X6B   // 电源管理寄存器1

class MPU6050
{
private:
    int _fd;

public:
    MPU6050(int filediscription);
    void sample(double *result);
    ~MPU6050();
};

MPU6050::MPU6050(int filediscription) : _fd(filediscription)
{
    if (ioctl(_fd, I2C_SLAVE, MPU_ADDR) < 0)
    {
        throw "Failed to connect to the sensor";
    }

    // Initialize the MPU6050
    uint8_t pwr[2] = {MPU_PWR_MGMT1_REG, 0};
    if (write(_fd, pwr, 2) != 2)
    {
        throw "Failed to initialize the MPU6050";
    }
    // Setting the operating parameter of MPU6050
    uint8_t mode[5]{MPU_SAMPLE_RATE_REG, 0x07, 0x06, 0x10, 0x00}; // start from 0x19
    if (write(_fd, mode, 5) != 5)
    {
        throw "Failed to write to the register mode";
    }
}

void MPU6050::sample(double *result)
{
    if (ioctl(_fd, I2C_SLAVE, MPU_ADDR) < 0)
    {
        throw "Failed to connect to the sensor";
    }
    // Read 14 bytes of data (accelerometer, temperature, gyroscope)
    char dataBuf[14];
    uint8_t regAddr = MPU_ACCEL_XOUTH_REG;
    if (write(_fd, &regAddr, 1) != 1)
    {
        throw "Failed to write to the register sample";
    }

    if (read(_fd, dataBuf, 14) != 14)
    {
        throw "Failed to read the sensor data";
    }

    // Convert and output the accelerometer data
    int16_t accel_x = (dataBuf[0] << 8) | dataBuf[1];
    int16_t accel_y = (dataBuf[2] << 8) | dataBuf[3];
    int16_t accel_z = (dataBuf[4] << 8) | dataBuf[5];
    int16_t temp = (dataBuf[6] << 8) | dataBuf[7];
    int16_t gyro_x = (dataBuf[8] << 8) | dataBuf[9];
    int16_t gyro_y = (dataBuf[10] << 8) | dataBuf[11];
    int16_t gyro_z = (dataBuf[12] << 8) | dataBuf[13];

    result[0] = accel_x / 16384.0;
    result[1] = accel_y / 16384.0;
    result[2] = accel_z / 16384.0;
    result[3] = gyro_x / 32.8;
    result[4] = gyro_y / 32.8;
    result[5] = gyro_z / 32.8;
    result[6] = temp / 340 + 36.53;
    return;
}

MPU6050::~MPU6050()
{
}

#endif