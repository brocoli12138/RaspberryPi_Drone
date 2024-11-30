#ifndef __FUZZTITEMS_H__
#define __FUZZTITEMS_H__

#include <cmath>
#include <vector>
#include <map>

// exyz membership function
double exyz_positive_big(double input)
{
    if (input >= 0.2 && input <= 1)
    {
        return -9 / 80 / (input - 1.1) - 1 / 8;
    }
    if (input < 0.2 && input >= -1)
    {
        return 0.0;
    }
}

double exyz_positive_small(double input)
{
    if (input >= 0.2 && input <= 1)
    {
        return 9 / 80 / (input - 0.1) - 1 / 8;
    }
    if (input >= 0 && input < 0.2)
    {
        return 5 * input;
    }
    if (input < 0 && input >= -1)
    {
        return 0.0;
    }
}

double exyz_zero(double input)
{
    if (input >= -1 && input <= 1)
    {
        return std::pow(2.7182818281828, -input * input / 2 / 0.09 / 0.09);
    }
    else
    {
        return 0.0;
    }
}

double exyz_negative_small(double input)
{
    if (input < -0.2 && input >= -1)
    {
        return -9 / 80 / (input + 0.1) - 1 / 8;
    }
    if (input < 0 && input >= -0.2)
    {
        return -5 * input;
    }
    if (input >= 0 && input <= 1)
    {
        return 0.0;
    }
}

double exyz_negative_big(double input)
{
    if (input < -0.2 && input >= -1)
    {
        return 9 / 80 / (input + 1.1) - 1 / 8;
    }
    if (input >= -0.2 && input <= 1)
    {
        return 0.0;
    }
}
// ecxyz membership function

double ecxyz_positive_big(double input)
{
    if (3 * input >= 0.2 && 3 * input <= 1)
    {
        return -9 / 80 / (3 * input - 1.1) - 1 / 8;
    }
    if (3 * input < 0.2 && 3 * input >= -1)
    {
        return 0.0;
    }
}

double ecxyz_positive_small(double input)
{
    if (3 * input >= 0.2 && 3 * input <= 1)
    {
        return 9 / 80 / (3 * input - 0.1) - 1 / 8;
    }
    if (3 * input >= 0 && 3 * input < 0.2)
    {
        return 5 * 3 * input;
    }
    if (3 * input < 0 && 3 * input >= -1)
    {
        return 0.0;
    }
}

double ecxyz_zero(double input)
{
    if (3 * input >= -1 && 3 * input <= 1)
    {
        return std::pow(2.7182818281828, -3 * input * 3 * input / 2 / 0.09 / 0.09);
    }
    else
    {
        return 0.0;
    }
}

double ecxyz_negative_small(double input)
{
    if (3 * input < -0.2 && 3 * input >= -1)
    {
        return -9 / 80 / (3 * input + 0.1) - 1 / 8;
    }
    if (3 * input < 0 && 3 * input >= -0.2)
    {
        return -5 * 3 * input;
    }
    if (3 * input >= 0 && 3 * input <= 1)
    {
        return 0.0;
    }
}

double ecxyz_negative_big(double input)
{
    if (3 * input < -0.2 && 3 * input >= -1)
    {
        return 9 / 80 / (3 * input + 1.1) - 1 / 8;
    }
    if (3 * input >= -0.2 && 3 * input <= 1)
    {
        return 0.0;
    }
}

// controlxyz membership function
double controlxyz_positive_big(double input)
{
    if (input >= 26 && input <= 30)
    {
        return 1.0;
    }
    else if (input >= 18 && input < 26)
    {
        return input / 8 + 9 / 4;
    }
    if (input >= -30 && input < 18)
    {
        return 0.0;
    }
}

double controlxyz_positive_small(double input)
{
    if (input >= 12 && input <= 30)
    {
        return -input / 18 + 5 / 3;
    }
    else if (input >= 6 && input < 12)
    {
        return input / 6 - 1;
    }
    if (input < 6 && input >= -30)
    {
        return 0.0;
    }
}

double controlxyz_zero(double input)
{
    if (input < 12 && input >= -12)
    {
        return -input * input / 144 + 1;
    }
    if (input >= 12 && input <= 30 || input < -12 && input >= -30)
    {
        return 0.0;
    }
}

double controlxyz_negative_small(double input)
{
    if (input >= -30 && input < -12)
    {
        return input / 18 + 5 / 3;
    }
    if (input >= -12 && input < -6)
    {
        return -input / 6 - 1;
    }
    if (input >= -6 && input <= 30)
    {
        return 0.0;
    }
}

double controlxyz_negative_big(double input)
{
    if (input < -26 && input >= -30)
    {
        return 1.0;
    }
    else if (input < -18 && input >= -26)
    {
        return -input / 8 + 9 / 4;
    }
    if (input <= 30 && input >= -18)
    {
        return 0.0;
    }
}

std::vector<std::pair<std::__cxx11::string, double (*)(double)>>
    exyz = {{"eNB", exyz_negative_big}, {"eNS", exyz_negative_small}, {"e0", exyz_zero}, {"ePS", exyz_positive_small}, {"ePB", exyz_positive_big}},
    ecxyz = {{"ecNB", ecxyz_negative_big}, {"ecNS", ecxyz_negative_small}, {"ec0", ecxyz_zero}, {"ecPS", ecxyz_positive_small}, {"ecPB", ecxyz_positive_big}},
    controlxyz = {{"ctNB", controlxyz_negative_big}, {"ctNS", controlxyz_negative_small}, {"ct0", controlxyz_zero}, {"ctPS", controlxyz_positive_small}, {"ctPB", controlxyz_positive_big}};

std::map<std::__cxx11::string, std::map<std::__cxx11::string, std::__cxx11::string>> controlxyz_rules =
    {{"eNB", {"ecNB", "ct0"}},
     {"eNB", {"ecNS", "ctNS"}},
     {"eNB", {"ec0", "ctNB"}},
     {"eNB", {"ecPS", "ctNB"}},
     {"eNB", {"ecPB", "ctNB"}},

     {"eNS", {"ecNB", "ctPS"}},
     {"eNS", {"ecNS", "ct0"}},
     {"eNS", {"ec0", "ctNS"}},
     {"eNS", {"ecPS", "ctNB"}},
     {"eNS", {"ecPB", "ctNB"}},

     {"e0", {"ecNB", "ctPB"}},
     {"e0", {"ecNS", "ctPS"}},
     {"e0", {"ec0", "ct0"}},
     {"e0", {"ecPS", "ctNS"}},
     {"e0", {"ecPB", "ctNB"}},

     {"ePS", {"ecNB", "ctPB"}},
     {"ePS", {"ecNS", "ctPB"}},
     {"ePS", {"ec0", "ctPS"}},
     {"ePS", {"ecPS", "ct0"}},
     {"ePS", {"ecPB", "ctNS"}},

     {"ePB", {"ecNB", "ctPB"}},
     {"ePB", {"ecNS", "ctPB"}},
     {"ePB", {"ec0", "ctPS"}},
     {"ePB", {"ecPS", "ctPS"}},
     {"ePB", {"ecPB", "ct0"}}};

double eh_negative_big(double input)
{
    if (input < -7)
    {
        return 1.0;
    }
    if (input >= -7 && input < -5)
    {
        return -0.5 * input - 2.5;
    }
    if (input >= -5)
    {
        return 0.0;
    }
}

double eh_nagative_small(double input)
{
    if (input < -7 || input >= 0)
    {
        return 0.0;
    }
    if (input >= -7 && input < -3)
    {
        return input / 4 + 7 / 4;
    }
    if (input >= -3 && input < 0)
    {
        return -input / 3;
    }
}

double eh_zero(double input)
{
    if (input < -1 || input >= 1)
    {
        return 0.0;
    }
    if (input >= -1 && input < -0.2)
    {
        return 1.25 * input + 1.25;
    }
    if (input >= -0.2 && input < 0.2)

    {
        return 1.0;
    }
    if (input >= 0.2 && input < 1)
    {
        return -1.25 * input + 1.25;
    }
}

double eh_positive_small(double input)
{
    if (input < 0 || input >= 7)
    {
        return 0.0;
    }
    if (input < 7 && input >= 3)
    {
        return -input / 4 + 7 / 4;
    }
    if (input >= 0 && input < 3)
    {
        return input / 3;
    }
}

double eh_positive_big(double input)
{
    if (input >= 7)
    {
        return 1.0;
    }
    if (input >= 5 && input < 7)
    {
        return 0.5 * input - 2.5;
    }
    if (input < 5)
    {
        return 0;
    }
}

double ech_negative_big(double input)
{
    if (input < -4 && input >= -8)
    {
        return -input / 4 - 1.0;
    }
    if (input < -8)
    {
        return 1.0;
    }
    if (input > -4)
    {
        return 0.0;
    }
}

double ech_negative_small(double input)
{
    if (input < -1 && input >= -6)
    {
        return -(input + 6) * (input + 6) / 25 + 1;
    }
    if (input < -6 && input >= -7)
    {
        return 1.0;
    }
    if (input < -7 && input >= -8)
    {
        return input + 8;
    }
    if (input < -8 || input >= -1)
    {
        return 0;
    }
}

double ech_zero(double input)
{
    return std::pow(2.7182818281828, -input * input / 2 / 0.6 / 0.6);
}

double ech_positive_small(double input)
{
    if (input >= 1 && input < 6)
    {
        return -(input - 6) * (input - 6) / 25 + 1;
    }
    if (input >= 6 && input < 7)
    {
        return 1.0;
    }
    if (input >= 7 && input < 8)
    {
        return -input + 8;
    }
    if (input >= 8 || input < 1)
    {
        return 0.0;
    }
}

double ech_positive_big(double input)
{
    if (input >= 4 && input < 8)
    {
        return input / 4 - 1.0;
    }
    if (input >= 8)
    {
        return 1.0;
    }
    if (input < 4)
    {
        return 0.0;
    }
}

double kp_extra_small(double input)
{
    if (input >= 0 && input < 0.25)
    {
        return 1.0;
    }
    if (input >= 0.25 && input < 0.5)
    {
        return -4 * input + 2;
    }
    if (input <= 14 && input >= 0.5)
    {
        return 0.0;
    }
}

double kp_small(double input)
{
    if (input >= 0 && input < 0.25)
    {
        return 0.0;
    }
    if (input >= 0.25 && input < 4)
    {
        return 4 * input / 15 - 1 / 15;
    }
    if (input >= 4 && input < 5)
    {
        return 1.0;
    }
    if (input >= 5 && input < 7)
    {
        return -input / 2 + 7 / 2;
    }
    if (input >= 7 && input <= 14)
    {
        return 0.0;
    }
}

double kp_medium(double input)
{
    if (input >= 0 || input < 5)
    {
        return 0.0;
    }
    if (input >= 5 && input < 7)
    {
        return input / 2 - 5 / 2;
    }
    if (input >= 7 && input < 9)
    {
        return -input / 2 + 9 / 2;
    }
    if (input >= 9 && input <= 14)
    {
        return 0.0;
    }
}

double kp_big(double input)
{
    if (input >= 0 && input < 7)
    {
        return 0.0;
    }
    if (input >= 7 && input < 9)
    {
        return input / 2 - 7 / 2;
    }
    if (input >= 9 && input < 10)
    {
        return 1.0;
    }
    if (input >= 10 && input < 13.75)
    {
        return -4 * input / 15 + 11 / 3;
    }
    if (input >= 13.15 && input <= 14)
    {
        return 0.0;
    }
}

double kp_extra_big(double input)
{
    if (input >= 13.75 && input <= 14)
    {
        return 1.0;
    }
    if (input >= 13.5 && input < 13.75)
    {
        return 4 * input - 54;
    }
    if (input >= 0 && input < 13.5)
    {
        return 0.0;
    }
}

double ki_extra_small(double input)
{
    if (input >= 0 && input < 0.0005)
    {
        return -2000 * input - 1;
    }
    if (input >= 0.0005 && input <= 0.05)
    {
        return 0.0;
    }
}

double ki_small(double input)
{
    if (input >= 0 && input < 0.0005)
    {
        return 0.0;
    }
    if (input >= 0.0005 && input < 0.005)
    {
        return 2000 * input / 9 - 1 / 9;
    }
    if (input >= 0.005 && input < 0.01)
    {
        return 1.0;
    }
    if (input >= 0.01 && input < 0.015)
    {
        return -200 * input + 3;
    }
    if (input >= 0.015 && input <= 0.05)
    {
        return 0.0;
    }
}

double ki_medium(double input)
{
    return std::pow(2.7182818281828, -(input - 0.025) * (input - 0.025) / 2 / 0.004 / 0.004);
}

double ki_big(double input)
{
    if (input >= 0 && input < 0.035)
    {
        return 0.0;
    }
    if (input >= 0.035 && input < 0.04)
    {
        return 200 * input - 7;
    }
    if (input >= 0.04 && input < 0.045)
    {
        return 1.0;
    }
    if (input >= 0.045 && input < 0.0495)
    {
        return -2000 * input / 9 + 11;
    }
    if (input >= 0.0495 && input < 0.05)
    {
        return 0.0;
    }
}

double ki_extra_big(double input)
{
    if (input >= 0 && input < 0.0495)
    {
        return 0.0;
    }
    if (input >= 0.0495 && input <= 0.05)
    {
        return 2000 * input - 99;
    }
}

double kd_extra_small(double input)
{
    if (input >= 0 && input < 0.05)
    {
        return -400 * input * input + 1;
    }
    if (input >= 0.05 && input <= 10)
    {
        return 0.0;
    }
}

double kd_small(double input)
{
    if (input >= 0 && input < 1.5)
    {
        return 2 * input / 3;
    }
    if (input >= 1.5 && input < 2.5)
    {
        return 1.0;
    }
    if (input >= 2.5 && input < 3.5)
    {
        return -input + 3.5;
    }
    if (input >= 3.5 && input <= 10)
    {
        return 0.0;
    }
}

double kd_medium(double input)
{
    if (input >= 0 && input < 2.5)
    {
        return 0.0;
    }
    if (input >= 2.5 && input < 5)
    {
        return 2 * input / 5 - 1;
    }
    if (input >= 5 && input < 7.5)
    {
        return -2 * input / 5 + 3;
    }
    if (input >= 7.5 && input <= 10)
    {
        return 0.0;
    }
}

double kd_big(double input)
{
    if (input >= 0 && input < 6.5)
    {
        return 0.0;
    }
    if (input >= 6.5 && input < 7.5)
    {
        return input - 6.5;
    }
    if (input >= 7.5 && input < 8.5)
    {
        return 1.0;
    }
    if (input >= 8.5 && input <= 10)
    {
        return -2 * input / 3 + 20 / 3;
    }
}

double kd_extra_big(double input)
{
    if (input >= 0 && input < 9.95)
    {
        return 0.0;
    }
    if (input >= 9.95 && input <= 10)
    {
        return -400 * (input - 10) * (input - 10) + 1;
    }
}

std::vector<std::pair<std::__cxx11::string, double (*)(double)>>
    eh = {{"eNB", eh_negative_big}, {"eNS", eh_nagative_small}, {"e0", eh_zero}, {"ePS", eh_positive_small}, {"ePB", eh_positive_big}},
    ech = {{"ecNB", ech_negative_big}, {"ecNS", ech_negative_small}, {"ec0", ech_zero}, {"ecPS", ech_positive_small}, {"ecPB", ech_positive_big}},
    kp = {{"kpES", kp_extra_small}, {"kpS", kp_small}, {"kpM", kp_medium}, {"kpB", kp_big}, {"kpEB", kp_extra_big}},
    ki = {{"kiES", ki_extra_small}, {"kiS", ki_small}, {"kiM", ki_medium}, {"kiB", ki_big}, {"kiEB", ki_extra_big}},
    kd = {{"kdES", kd_extra_small}, {"kdS", kd_small}, {"kdM", kd_medium}, {"kdB", kd_big}, {"kdEB", kd_extra_big}};

std::map<std::__cxx11::string, std::map<std::__cxx11::string, std::__cxx11::string>> kp_rules =
    {{"eNB", {"ecNB", "kpEB"}},
     {"eNB", {"ecNS", "kpB"}},
     {"eNB", {"ec0", "kpM"}},
     {"eNB", {"ecPS", "kpS"}},
     {"eNB", {"ecPB", "kpES"}},

     {"eNS", {"ecNB", "kpB"}},
     {"eNS", {"ecNS", "kpM"}},
     {"eNS", {"ec0", "kpS"}},
     {"eNS", {"ecPS", "kpES"}},
     {"eNS", {"ecPB", "kpES"}},

     {"e0", {"ecNB", "kpM"}},
     {"e0", {"ecNS", "kpS"}},
     {"e0", {"ec0", "kpES"}},
     {"e0", {"ecPS", "kpES"}},
     {"e0", {"ecPB", "kpES"}},

     {"ePS", {"ecNB", "kpB"}},
     {"ePS", {"ecNS", "kpM"}},
     {"ePS", {"ec0", "kpS"}},
     {"ePS", {"ecPS", "kpES"}},
     {"ePS", {"ecPB", "kpES"}},

     {"ePB", {"ecNB", "kpEB"}},
     {"ePB", {"ecNS", "kpB"}},
     {"ePB", {"ec0", "kpM"}},
     {"ePB", {"ecPS", "kpS"}},
     {"ePB", {"ecPB", "kpES"}}};

std::map<std::__cxx11::string, std::map<std::__cxx11::string, std::__cxx11::string>> ki_rules =
    {{"eNB", {"ecNB", "kiES"}},
     {"eNB", {"ecNS", "kiS"}},
     {"eNB", {"ec0", "kiM"}},
     {"eNB", {"ecPS", "kiB"}},
     {"eNB", {"ecPB", "kiEB"}},

     {"eNS", {"ecNB", "kiS"}},
     {"eNS", {"ecNS", "kiM"}},
     {"eNS", {"ec0", "kiS"}},
     {"eNS", {"ecPS", "kiM"}},
     {"eNS", {"ecPB", "kiB"}},

     {"e0", {"ecNB", "kiM"}},
     {"e0", {"ecNS", "kiS"}},
     {"e0", {"ec0", "kiS"}},
     {"e0", {"ecPS", "kiS"}},
     {"e0", {"ecPB", "kiM"}},

     {"ePS", {"ecNB", "kiB"}},
     {"ePS", {"ecNS", "kiM"}},
     {"ePS", {"ec0", "kiS"}},
     {"ePS", {"ecPS", "kiM"}},
     {"ePS", {"ecPB", "kiS"}},

     {"ePB", {"ecNB", "kiEB"}},
     {"ePB", {"ecNS", "kiB"}},
     {"ePB", {"ec0", "kiM"}},
     {"ePB", {"ecPS", "kiS"}},
     {"ePB", {"ecPB", "kiES"}}};

std::map<std::__cxx11::string, std::map<std::__cxx11::string, std::__cxx11::string>> kd_rules =
    {{"eNB", {"ecNB", "kdM"}},
     {"eNB", {"ecNS", "kdS"}},
     {"eNB", {"ec0", "kdES"}},
     {"eNB", {"ecPS", "kdS"}},
     {"eNB", {"ecPB", "kdM"}},

     {"eNS", {"ecNB", "kdB"}},
     {"eNS", {"ecNS", "kdM"}},
     {"eNS", {"ec0", "kdM"}},
     {"eNS", {"ecPS", "kdM"}},
     {"eNS", {"ecPB", "kdB"}},

     {"e0", {"ecNB", "kdEB"}},
     {"e0", {"ecNS", "kdB"}},
     {"e0", {"ec0", "kdEB"}},
     {"e0", {"ecPS", "kdB"}},
     {"e0", {"ecPB", "kdEB"}},

     {"ePS", {"ecNB", "kdB"}},
     {"ePS", {"ecNS", "kdM"}},
     {"ePS", {"ec0", "kdM"}},
     {"ePS", {"ecPS", "kdM"}},
     {"ePS", {"ecPB", "kdB"}},

     {"ePB", {"ecNB", "kdM"}},
     {"ePB", {"ecNS", "kdS"}},
     {"ePB", {"ec0", "kdES"}},
     {"ePB", {"ecPS", "kdS"}},
     {"ePB", {"ecPB", "kdM"}}};
#endif