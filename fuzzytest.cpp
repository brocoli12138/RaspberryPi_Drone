#include "../include/Fuzzycontroller.h"
#include <iostream>

double e_big_triangle(double input)
{
    if (std::abs(input) >= 50000)
    {
        return 1.0;
    }
    else if (std::abs(input) < 50000 && std::abs(input) >= 25000)
    {
        return 1.0 / 25000 * std::abs(input) - 1;
    }
    else
    {
        return 0.0;
    }
}

double e_medium_triangle(double input)
{
    if (std::abs(input) >= 25000 && std::abs(input) < 50000)
    {
        return -1.0 / 25000 * std::abs(input) + 2;
    }
    else if (std::abs(input) < 25000)
    {
        return 1.0 / 25000 * std::abs(input);
    }
    else
    {
        return 0.0;
    }
}

double e_small_triangle(double input)
{
    if (std::abs(input) < 25000)
    {
        return -1.0 / 25000 * std::abs(input) + 1;
    }
    else
    {
        return 0.0;
    }
}

double ec_big_triangle(double input)
{
    if (std::abs(input) >= 200)
    {
        return 1.0;
    }
    else if (std::abs(input) < 200 && std::abs(input) >= 100)
    {
        return 1.0 / 100 * input - 1;
    }
    else
    {
        return 0.0;
    }
}

double ec_medium_triangle(double input)
{
    if (std::abs(input) >= 100 && std::abs(input) < 200)
    {
        return -1.0 / 100 * std::abs(input) + 2;
    }
    else if (std::abs(input) < 100)
    {
        return 1.0 / 100 * std::abs(input);
    }
    else
    {
        return 0.0;
    }
}

double ec_small_triangle(double input)
{
    if (std::abs(input) < 10)
    {
        return -1.0 / 10 * std::abs(input) + 1;
    }
    else
    {
        return 0.0;
    }
}

double output_big_triangle(double input)
{
    if (input >= 100)
    {
        return 1.0;
    }
    else if (input < 100 && input >= 50)
    {
        return 1.0 / 50 * input - 1;
    }
    else
    {
        return 0.0;
    }
}

double output_medium_triangle(double input)
{
    if (input < 100 && input >= 50)
    {
        return -1.0 / 50 * input + 2;
    }
    else if (input < 50 && input >= 0)
    {
        return 1.0 / 50 * input;
    }
    else
    {
        return 0.0;
    }
}

double output_small_triangle(double input)
{
    if (input < 50 && input >= 0)
    {
        return -1.0 / 50 * input + 1;
    }
    else
    {
        return 0.0;
    }
}

int main()
{
    std::vector<std::pair<std::string, double (*)(double)>> e, ec, output;
    e.emplace_back(std::pair<std::string, double (*)(double)>{std::string{"ebig"}, (&e_big_triangle)});
    e.emplace_back(std::pair<std::string, double (*)(double)>{std::string{"emedium"}, (&e_medium_triangle)});
    e.emplace_back(std::pair<std::string, double (*)(double)>{std::string{"esamll"}, (&e_small_triangle)});

    ec.emplace_back(std::pair<std::string, double (*)(double)>{std::string{"ecbig"}, (&ec_big_triangle)});
    ec.emplace_back(std::pair<std::string, double (*)(double)>{std::string{"ecmedium"}, (&ec_medium_triangle)});
    ec.emplace_back(std::pair<std::string, double (*)(double)>{std::string{"ecsamll"}, (&ec_small_triangle)});

    output.emplace_back(std::pair<std::string, double (*)(double)>{std::string{"outputbig"}, (&output_big_triangle)});
    output.emplace_back(std::pair<std::string, double (*)(double)>{std::string{"outputmedium"}, (&output_medium_triangle)});
    output.emplace_back(std::pair<std::string, double (*)(double)>{std::string{"outputsamll"}, (&output_small_triangle)});

    std::map<std::string, std::map<std::string, std::string>> rules;
    rules["ebig"]["ecbig"] = "outputbig";
    rules["ebig"]["ecmedium"] = "outputbig";
    rules["ebig"]["ecsamll"] = "outputbig";
    rules["emedium"]["ecbig"] = "outputbig";
    rules["emedium"]["ecmedium"] = "outputmedium";
    rules["emedium"]["ecsamll"] = "outputmedium";
    rules["esamll"]["ecbig"] = "outputbig";
    rules["esamll"]["ecmedium"] = "outputmedium";
    rules["esamll"]["ecsamll"] = "outputsamll";

    Fuzzycontroller controller{{-100000, 100000, -100000, 100000, 0, 100}, e, ec, output, rules};

    std::cout << controller.update(99999, 9999) << std::endl;
    return 0;
}