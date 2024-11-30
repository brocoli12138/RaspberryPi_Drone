#ifndef __Quaternion_H__
#define __Quaternion_H__
#include <vector>
#include <cmath>

class Quaternion
{
private:
    double _w, _i, _j, _k;

public:
    Quaternion(double, double, double, double);
    Quaternion unitize() const
    {
        double norm = std::pow(_w * _w + _i * _i + _j * _j + _k * _k, 0.5);
        return Quaternion{_w / norm, _i / norm, _j / norm, _k / norm};
    }
    std::vector<double> tovector() { return std::vector<double>{_w, _i, _j, _k}; };
    Quaternion operator*(const Quaternion &q) const
    {
        double resw, resi, resj, resk;
        resw = _w * q._w - _i * q._i - _j * q._j - _k * q._k;
        resi = _w * q._i + _i * q._w + _j * q._k - _k * q._j;
        resj = _w * q._j - _i * q._k + _j * q._w + _k * q._i;
        resk = _w * q._k + _i * q._j - _j * q._i + _k * q._w;
        return Quaternion{resw, resi, resj, resk};
    };
    Quaternion operator+(const Quaternion &q) const
    {
        return Quaternion{_w + q._w, _i + q._i, _j + q._j, _k + q._k};
    };

    friend Quaternion conjugate(const Quaternion &q)
    {
        return Quaternion{q._w, -q._i, -q._j, -q._k};
    };
    
    friend Quaternion operator*(double num, const Quaternion &q)
    {
        return Quaternion{num * q._w, num * q._i, num * q._j, num * q._k};
    }
    friend Quaternion operator*(const Quaternion &q, double num)
    {
        return Quaternion{num * q._w, num * q._i, num * q._j, num * q._k};
    }
    ~Quaternion();
};

Quaternion::Quaternion(double w, double i, double j, double k) : _w(w), _i(i), _j(j), _k(k)
{
}

Quaternion::~Quaternion()
{
}

#endif