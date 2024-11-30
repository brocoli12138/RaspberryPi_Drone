// PIDcontroller.h
#ifndef __PIDCONTROLLER_H__
#define __PIDCONTROLLER_H__

class PIDcontroller
{
private:
    // PID的三个控制参数
    double _Kp, _Ki, _Kd;
    double _integral, _last_error;

public:
    PIDcontroller(double Kp = 0, double Ki = 0, double Kd = 0)
        : _Kp(Kp), _Ki(Ki), _Kd(Kd), _integral(0), _last_error(0){};
    void setParameter(const double kp, const double ki, const double kd);
    // 更新输出信号，输入误差和时间差，单位为毫秒
    double update(const double e, const double dt);
    ~PIDcontroller(){};
};

void PIDcontroller::setParameter(const double Kp, const double Ki, const double Kd)
{
    _Kp = Kp;
    _Ki = Ki;
    _Kd = Kd;
}

double PIDcontroller::update(const double e, const double dt)
{
    // 计算误差积分，使用离散的加法近似
    _integral = _integral + (e + _last_error) * dt / 2;
    // 计算误差的微分，使用梯形法近似
    double ec = (e - _last_error) / dt;
    // 计算控制信号
    double control = _Kp * e + _Ki * _integral + _Kd * ec;
    // 维护类对象实例内的相关变量
    _last_error = e;
    // 返回结果
    return control;
}

#endif