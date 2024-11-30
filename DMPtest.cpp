#include "DMP.h"
#include <iostream>
#include <vector>

int main()
{
    DMP dmp{0.001, 0.5, 0.0004, 0.001};
    std::vector<double> res;
    try
    {
        while (1)
        {
            res = dmp.solve(10);
            for (auto iter = res.begin(); iter < res.end(); iter++)
            {
                std::cout << *iter << ' ';
            }
            std::cout << '\n';
        }
    }
    catch (char const* e)
    {
        std::cerr << e << '\n';
    }

    return 0;
}