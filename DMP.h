// DMP.h
#ifndef __DMP_H__
#define __DMP_H__

#define PI 3.141592653589793

#include <vector>
#include <cmath>
#include "Kalmanfilter.h"
#include "MPU6050.h"
#include "HMC5883L.h"
#include "BMP280.h"
#include "Quaternion.h"

// digital motion processor
class DMP
{
private:
    int _fd;
    MPU6050 _mpu;
    HMC5883L _compass;
    BMP280 _barometer;
    KalmanFilter _rollfilter, _pitchfilter, _yawfilter, _velocityfilter, _heightfilter;
    Quaternion _last_pose;
    double _last_roll, _last_pitch, _last_yaw, _last_velocity, _last_height, _initial_g;

protected:
    double calcHeight(double pressure, double temperature);
    Quaternion project2geometic(Quaternion &angulur_quaternion, Quaternion &pose_quanternion);
    // 使用弧度表示欧拉角

protected:
    std::vector<double> sample();

public:
    DMP(double accelerometer_pricision, double gryoscope_pricise, double megnetometer_pricise, double barometer_pricision);
    std::vector<double> solve(double dt);
    ~DMP();
};

DMP::DMP(double accelerometer_pricision, double gryoscope_pricise, double megnetometer_pricise, double barometer_pricision)
    : _fd(open("/dev/i2c-1", O_RDWR)),
      _mpu(_fd), _compass(_fd), _barometer(_fd),
      _last_pose(0, 0, 0, 0), _last_velocity(0),
      _rollfilter(accelerometer_pricision, gryoscope_pricise),
      _pitchfilter(accelerometer_pricision, gryoscope_pricise),
      _yawfilter(megnetometer_pricise, gryoscope_pricise),
      _velocityfilter(barometer_pricision, accelerometer_pricision),
      _heightfilter(barometer_pricision, accelerometer_pricision)
{
    if (_fd < 0)
    {
        throw "Failed to open the i2c bus";
    }
    // waste two frames of data to initialize sensors
    sample();
    sample();
    std::vector<double> initdata = sample();
    // get data from MPU6050 to form a initial pose
    double pitch = std::atan(initdata[0] / std::pow(initdata[1] * initdata[1] + initdata[2] * initdata[2], 0.5)),
           roll = std::atan(initdata[1] / std::pow(initdata[0] * initdata[0] + initdata[2] * initdata[2], 0.5));
    Quaternion qz{1, 0, 0, 0}, qy{std::cos(pitch / 2), 0, std::sin(pitch / 2), 0}, qx{std::cos(roll / 2), std::sin(roll / 2), 0, 0};
    _last_pose = (qz * qy * qx).unitize();
    // get data from HMC5883L to remove zero error of yaw
    _last_pitch = pitch;
    _last_roll = roll;
    _last_yaw = std::atan2(initdata[8], initdata[7]);
    // get data from BMP280 to remove zero error of height
    _last_height = calcHeight(initdata[11], (initdata[6] + initdata[10]) / 2);
    _initial_g = std::pow(initdata[0] * initdata[0] + initdata[1] * initdata[1] + initdata[2] * initdata[2], 0.5);
}

std::vector<double> DMP::solve(double dt)
{
    // setp 1: get data from MPU6050
    // setp 2: get data from HMC5883L (turn degree into radis)
    // setp 3: get data from BMP280
    std::vector<double> initdata = sample();
    // setp 4: calculate delta roll(accelerometer radis), delta pitch(accelerometer radis) and delta yaw(megnetometer radis) as well as delta height(barometer). These are all measured value.
    double deltapitch = std::atan(initdata[0] / std::pow(initdata[1] * initdata[1] + initdata[2] * initdata[2], 0.5)) - _last_pitch,
           deltaroll = std::atan(initdata[1] / std::pow(initdata[0] * initdata[0] + initdata[2] * initdata[2], 0.5)) - _last_roll,
           deltayaw = std::atan2(initdata[8], initdata[7]) - _last_yaw,
           deltaheight = calcHeight(initdata[11], (initdata[6] + initdata[10]) / 2) - _last_height;
    // step 5: use kalmanfilter to form filterd angles
    double deltaroll_filtered = _rollfilter.update(deltaroll, initdata[3] * PI / 180 / 1000 * dt),
           deltapitch_filtered = _pitchfilter.update(deltapitch, initdata[4] * PI / 180 / 1000 * dt),
           deltayaw_filtered = _yawfilter.update(deltayaw, initdata[5] * PI / 180 / 1000 * dt);
    // setp 6: transform angular velocity into quaternion
    Quaternion w_local{0, deltaroll_filtered / dt, deltapitch_filtered / dt, deltayaw_filtered / dt};
    // step 7: project the angular velocity into ground coordinate
    Quaternion w_global = _last_pose * w_local * conjugate(_last_pose);
    // strp 8: renew pose(this is the result of pose)
    Quaternion newpose = (_last_pose + 0.5 * w_global * _last_pose * dt).unitize();
    // setp 9: calculate velocity from velocityfilter
    double a = std::pow(initdata[0] * initdata[0] + initdata[1] * initdata[1] + initdata[2] * initdata[2], 0.5) - _initial_g;
    double velocity = deltaheight / dt,
           velocity_filtered = _velocityfilter.update(velocity, _last_velocity + a * dt / 1000);
    // setp 10: calculate height from heightfilter (form result align with renewed pose)
    double height = _last_height + deltaheight,
           height_filtered = _heightfilter.update(height, _last_height + 0.5 * a * dt * dt / 1000000);
    // setp 11: renew all _last_time data
    _last_roll = _last_roll + deltaroll_filtered;
    _last_pitch = _last_pitch + deltapitch_filtered;
    _last_yaw = _last_yaw + deltayaw_filtered;
    _last_velocity = velocity_filtered;
    _last_height = height_filtered;
    _last_pose = newpose;
    // setp 12: return the result
    std::vector<double> result = newpose.tovector();
    result.push_back(calcHeight(initdata[11], (initdata[6] + initdata[10]) / 2));
    return result;
}

std::vector<double> DMP::sample()
{
    std::vector<double> result;
    double samples[7];
    _mpu.sample(samples);
    for (int i = 0; i < 7; i++)
    {
        result.push_back(samples[i]);
    }
    _compass.sample(samples);
    for (int i = 0; i < 3; i++)
    {
        result.push_back(samples[i]);
    }
    _barometer.sample(samples);
    for (int i = 0; i < 2; i++)
    {
        result.push_back(samples[i]);
    }
    return result;
}

DMP::~DMP()
{
    close(_fd);
}

double DMP::calcHeight(double pressure, double temperature)
{
    // 计算高度的函数 米
    // return (1.0 - std::pow(101325 / pressure, -6.560731)) * 288.15 / 0.0065;
    // return 44330.0 * (1.0 - std::pow(pressure / 101325, 1.0 / 5.255));
    double hp = (288.15 / 0.0065) * (1 - std::pow(pressure / 101325, 8.3144598 * 0.0065 / 9.80665 / 0.0289644)),
           hc = (temperature + 273.15 + 0.0065 * hp) * hp / 288.15;
    return hc;
}

Quaternion DMP::project2geometic(Quaternion &anguler_quaternion, Quaternion &pose_quanternion)
{
    return pose_quanternion * anguler_quaternion * conjugate(pose_quanternion);
}

#endif
