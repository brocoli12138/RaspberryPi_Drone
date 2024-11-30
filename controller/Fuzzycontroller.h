// Fuzzycontroller.h
#ifndef __FUZZYCONTROLLER_H__
#define __FUZZYCONTROLLER_H__

#include <map>
#include <string>
#include <vector>

class LinguisticVariable : protected std::pair<double (*)(double), double>
{
private:
    /* data */

public:
    LinguisticVariable(double (*membershipFunc)(double)) : std::pair<double (*)(double), double>(membershipFunc, 0){};
    // convert a crisp input into a fuzzy linguistic variable with a membership value
    double linguistify(double crispInput)
    {
        double temp = this->first(crispInput);
        this->second = temp <= 1 ? temp : 1;
        return this->second;
    };
    // return a reference to membership value of linguistic varable
    double &value() { return this->second; };
    ~LinguisticVariable(){};

    // calculate membership value of this liguistic variable under different crisp input
    friend double calcMembership(const LinguisticVariable &var, const double crispInput, const double limit = 1) { return var.first(crispInput) > limit ? limit : var.first(crispInput); }
};

class FuzzySet : public std::map<std::string, LinguisticVariable>
{
private:
    double _universeleft, _universeright;

protected:
    // calculate the maximum membership among the fuzzy linguitic variable in the fuzzy set according to crisp input
    double calcMembershipValue(const double crispInput);

public:
    FuzzySet(double universeleft, double universeright) : std::map<std::string, LinguisticVariable>(),
                                                          _universeleft(universeleft), _universeright(universeright){};
    void insert(const std::string name, double (*membershipFunc)(double)) { this->emplace(name, LinguisticVariable{membershipFunc}); };
    // using centriod method
    double defuzzify();
    void fuzzify(double crispInput);
    ~FuzzySet(){};
};

double FuzzySet::defuzzify()
{
    double numeratorSum = 0, denominatorSum = 0, dx = 0.0001, temp; // 分子 分母 微分变量
    for (double x = _universeleft; (x + dx) <= _universeright; x += dx)
    {
        temp = 0.5 * (calcMembershipValue(x) + calcMembershipValue(x + dx)) * dx;
        numeratorSum = numeratorSum + x * temp;
        denominatorSum = denominatorSum + temp;
    }
    return numeratorSum / denominatorSum;
}

void FuzzySet::fuzzify(double crispInput)
{
    for (auto &x : *this)
    {
        x.second.linguistify(crispInput);
    }
}

double FuzzySet::calcMembershipValue(const double crispInput)
{
    double result = 0, temp = 0;
    for (auto iter = this->begin(); iter != this->end(); iter++)
    {
        temp = calcMembership(iter->second, crispInput, iter->second.value());
        result = temp > result ? temp : result;
    }
    return result;
}

class Fuzzycontroller : private std::map<std::string, std::map<std::string, std::string>>
{
private:
    FuzzySet _e, _ec;
    FuzzySet _output;

protected:
    void crispInput(double e_crispInput, double ec_crispInput);
    double getCrispOutput();

public:
    Fuzzycontroller(std::vector<double> univers, std::vector<std::pair<std::string, double (*)(double)>>, std::vector<std::pair<std::string, double (*)(double)>>, std::vector<std::pair<std::string, double (*)(double)>>, std::map<std::string, std::map<std::string, std::string>>);
    double update(double e_crispInput, double ec_crispInput)
    {
        crispInput(e_crispInput, ec_crispInput);
        return getCrispOutput();
    };
    ~Fuzzycontroller(){};
};

Fuzzycontroller::Fuzzycontroller(std::vector<double> univers, std::vector<std::pair<std::string, double (*)(double)>> e, std::vector<std::pair<std::string, double (*)(double)>> ec, std::vector<std::pair<std::string, double (*)(double)>> output, std::map<std::string, std::map<std::string, std::string>> rules)
    : std::map<std::string, std::map<std::string, std::string>>(rules), _e(univers[0], univers[1]), _ec(univers[2], univers[3]), _output(univers[4], univers[5])
{
    for (auto x : e)
    {
        _e.insert(x.first, x.second);
    }
    for (auto x : ec)
    {
        _ec.insert(x.first, x.second);
    }
    for (auto x : output)
    {
        _output.insert(x.first, x.second);
    }
}

void Fuzzycontroller::crispInput(double e_crispInput, double ec_crispInput)
{
    _e.fuzzify(e_crispInput);
    _ec.fuzzify(ec_crispInput);
}

double Fuzzycontroller::getCrispOutput()
{
    std::vector<std::string> e_notZero, ec_notZero;
    // 遍历获取隶属度不为零的输入模糊语言变量
    for (auto iter = _e.begin(); iter != _e.end(); iter++)
    {
        if (iter->second.value() != 0)
        {
            e_notZero.push_back(iter->first);
        }
    }
    for (auto iter = _ec.begin(); iter != _ec.end(); iter++)
    {
        if (iter->second.value() != 0)
        {
            ec_notZero.push_back(iter->first);
        }
    }
    // 初始化输出模糊集合
    for (auto iter = _output.begin(); iter != _output.end(); iter++)
    {
        iter->second.value() = 0;
    }
    // 应用模糊逻辑 将计算完的输出隶属度放入对应的模糊变量中
    for (auto x : e_notZero)
    {
        for (auto y : ec_notZero)
        {
            // _output[(*this)[x][y]].value() = _output[(*this)[x][y]].value() > std::min(_e[x].value(), _ec[y].value()) ? _output[(*this)[x][y]].value() : std::min(_e[x].value(), _ec[y].value());
            // Calculate the influence value once to clean up the logic
            auto leftiter = _e.find(x);
            auto rightiter = _ec.find(y);
            double influenceValue = std::min(leftiter->second.value(), rightiter->second.value());

            // Access or insert the output LinguisticVariable for the rule result
            auto ruleResult = (*this)[x][y];
            auto iter = _output.find(ruleResult);

            // Now safely update the value with the computed influence
            iter->second.value() = std::max(iter->second.value(), influenceValue);
        }
    }
    // 逆模糊化
    return _output.defuzzify();
}

#endif //
