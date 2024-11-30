#include "../include/PIDcontroller.h"
#include <iostream>
#include <windows.h>
int main()
{
    PIDcontroller controller{0.5, 0.2, 0.02};
    double data = 1000, aim = 1500, last_data = 990, controlsig = 0;
    double duration = 0.1;
    for (int i = 0; i < 200; i++)
    {
        controlsig = controller.update(aim - data, duration);
        data += controlsig;
        std::cout << "control: " << controlsig << " data:" << data << '\n';
        last_data = data;
        Sleep(1);
    }
    return 0;
}