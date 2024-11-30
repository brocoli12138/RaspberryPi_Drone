// FuzzyPIDcontroller.h
#ifndef __FUZZYPIDCONTROLLER_H__
#define __FUZZYPIDCONTROLLER_H__
#include "Fuzzycontroller.h"
#include "PIDcontroller.h"

class FuzzyPIDcontroller
{
private:
    PIDcontroller _pid;
    Fuzzycontroller _kpfuzzy;
    Fuzzycontroller _kifuzzy;
    Fuzzycontroller _kdfuzzy;

public:
    FuzzyPIDcontroller(std::vector<double> universkp,
                       std::vector<double> universki,
                       std::vector<double> universkd,
                       std::vector<std::pair<std::string, double (*)(double)>> e,
                       std::vector<std::pair<std::string, double (*)(double)>> ec,
                       std::vector<std::pair<std::string, double (*)(double)>> kp,
                       std::vector<std::pair<std::string, double (*)(double)>> ki,
                       std::vector<std::pair<std::string, double (*)(double)>> kd,
                       std::map<std::string, std::map<std::string, std::string>> kprules,
                       std::map<std::string, std::map<std::string, std::string>> kirules,
                       std::map<std::string, std::map<std::string, std::string>> kdrules)
        : _pid(), _kpfuzzy(universkp, e, ec, kp, kprules), _kifuzzy(universki, e, ec, ki, kirules), _kdfuzzy(universkd, e, ec, kd, kdrules){};
    double update(double e, double ec, double dt);
    ~FuzzyPIDcontroller(){};
};

double FuzzyPIDcontroller::update(double e, double ec, double dt)
{
    double kp = _kpfuzzy.update(e, ec), ki = _kifuzzy.update(e, ec), kd = _kdfuzzy.update(e, ec);
    _pid.setParameter(kp, ki, kd);
    return _pid.update(e, dt);
}

#endif