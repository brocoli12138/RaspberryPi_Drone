#ifndef __MQTT_H__
#define __MQTT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTAsync.h"

#if !defined(_WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif

#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif

class Mqtt
{
private:
    /* data */
public:
    Mqtt(/* args */);
    ~Mqtt();
};

Mqtt::Mqtt(/* args */)
{
}

Mqtt::~Mqtt()
{
}

#endif