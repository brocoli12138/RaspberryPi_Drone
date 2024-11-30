#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "DMP.h"
#include "Quaternion.h"
#include "Fuzzycontroller.h"
#include "FuzzyPIDcontroller.h"
#include "Fuzzyitems.h"
#include <vector>
#include <chrono>

class Controller
{
private:
    DMP _dmp;
    Fuzzycontroller _angle_controller;
    FuzzyPIDcontroller _height_controller;

    std::chrono::system_clock::time_point _last_time_point;
    double _last_error_x, _last_error_y, _last_error_z, _last_error_h;

public:
    Controller();
    std::vector<double> output(std::vector<double> target);
    ~Controller(){};
};

Controller::Controller() : _dmp(0.001, 0.5, 0.0004, 0.001),
                 _angle_controller({-1, 1, -1, 1, -30, 30}, exyz, ecxyz, controlxyz, controlxyz_rules),
                 _height_controller({-9999, 9999, -9999, 9999, 0, 14}, {-9999, 9999, -9999, 9999, 0, 0.05}, {-9999, 9999, -9999, 9999, 0, 10}, eh, ech, kp, ki, kd, kp_rules, ki_rules, kd_rules),
                 _last_time_point(std::chrono::high_resolution_clock::now())
{
    // need a initial error
    std::vector<double> solve_result = _dmp.solve(2.0);
    double height = solve_result[4];
    Quaternion poseglobal{solve_result[0], solve_result[1], solve_result[2], solve_result[3]};
    // calculate errorlocal
    double target_height = 0;
    Quaternion target_pose_local{1, 0, 0, 0};
    Quaternion errorlocal = target_pose_local * conjugate(poseglobal);
    // calculate control signal
    std::vector<double> errorlocal_vector = errorlocal.tovector();
    _last_error_x = errorlocal_vector[1];
    _last_error_y = errorlocal_vector[2];
    _last_error_z = errorlocal_vector[3];
    _last_error_h = target_height - height;
};

std::vector<double> Controller::output(std::vector<double> target)
{
    /* targetposeglobal = poseglobal * targetposelocal * poseglobal_conj
    errorglobal = targetposeglobal * poseglobal_conj
    errorlocal = poseglobal_conj * targetposeglobal * poseglobal = targetposelocal * poseglobal_conj */

    // setp 1: get now time point and calculate duration from last time point
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = now - _last_time_point;
    double dt = duration.count();
    // step 2: get pose from _dmp
    std::vector<double> solve_result = _dmp.solve(dt);
    double height = solve_result[4];
    Quaternion poseglobal{solve_result[0], solve_result[1], solve_result[2], solve_result[3]};
    // step 3: calculate errorlocal
    double target_height = target[4];
    Quaternion target_pose_local{target[0], target[1], target[2], target[3]};
    Quaternion errorlocal = target_pose_local * conjugate(poseglobal);
    // step 4: calculate control signal
    std::vector<double> errorlocal_vector = errorlocal.tovector();
    double ex = errorlocal_vector[1], ey = errorlocal_vector[2], ez = errorlocal_vector[3], eh = target_height - height,
           ecx = (ex - _last_error_x) / dt, ecy = (ey - _last_error_y) / dt, ecz = (ez - _last_error_z) / dt, ech = (eh - _last_error_h) / dt;
    double controlx = _angle_controller.update(ex, ecx), controly = _angle_controller.update(ey, ecy),
           controlz = _angle_controller.update(ez, ecz), controlh = _height_controller.update(eh, ech, dt);
    std::vector<double> result{0.0, 0.0, 0.0, 0.0};
    result[0] = controlh * (100 - controlx - controly - controlz) / 100;
    result[1] = controlh * (100 - controlx + controly + controlz) / 100;
    result[2] = controlh * (100 + controlx - controly + controlz) / 100;
    result[3] = controlh * (100 + controlx + controly - controlz) / 100;
    // step 5: renew _last
    _last_time_point = now;
    _last_error_h = eh;
    _last_error_x = ex;
    _last_error_y = ey;
    _last_error_z = ez;
    // step 6: return result
    return result;
}

#endif