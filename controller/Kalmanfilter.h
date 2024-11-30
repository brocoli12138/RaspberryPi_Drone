// KalmanFilter.h
#ifndef __KalmanFilter_H__
#define __KalmanFilter_H__

#include <vector>
double defaultPredictFunciton(double, double, double);

class KalmanFilter
{
private:
    double _measure_pric;
    double _last_estimate_pric;

    double _last_estimate_result;

public:
    KalmanFilter(double measure_pric, double estimate_pric);
    /* output estimate result and maintain inner status */
    double update(double measure_for_result, double measure_for_predict);
    ~KalmanFilter();
};

KalmanFilter::KalmanFilter(double measure_pric, double estimate_pric)
    : _measure_pric(measure_pric), _last_estimate_pric(estimate_pric),
      _last_estimate_result(0)
{
}

double KalmanFilter::update(double measure_for_result, double measure_for_predict)
{
    // step 1 get measure value
    // has gotten in measure_for_result
    // step 2 get last estimate result
    // has gotten in member _last_estimate_result
    // step 4 calculate predict pricision
    double predict_pricision = _last_estimate_pric;
    // step 5 calculate estimate pricision
    double estimate_pricision = (predict_pricision * _measure_pric) / (predict_pricision + _measure_pric);
    // step 6 calculate kalman gain
    double kalman_gain = predict_pricision / (predict_pricision + _measure_pric);
    // step 7 output estimate result
    double estimate_result = (1 - kalman_gain) * measure_for_result + kalman_gain * measure_for_predict;;
    // setp 9 renew last estimate result and last estimate pricision these are angle instead of angularv
    _last_estimate_result = estimate_result;
    _last_estimate_pric = estimate_pricision;
    return estimate_result;
}

KalmanFilter::~KalmanFilter()
{
}

#endif