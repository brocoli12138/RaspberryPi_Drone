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

#define TIMEOUT 10000L
class Mqtt
{
private:
    MQTTAsync _client;
    MQTTAsync_connectOptions _conn_opts;
    MQTTAsync_disconnectOptions _disc_opts;
    MQTTAsync_message _pub_msg;
    MQTTAsync_responseOptions _resp_opts;

public:
    Mqtt(const char *,const char *);
    ~Mqtt();
};

Mqtt::Mqtt(const char * brokerAddress,const char *clientID) : _client(), _conn_opts(MQTTAsync_connectOptions_initializer),
               _disc_opts(MQTTAsync_disconnectOptions_initializer),
               _pub_msg(MQTTAsync_message_initializer),
               _resp_opts(MQTTAsync_responseOptions_initializer)
{
    int rc;
    if ((rc = MQTTAsync_create(&_client, brokerAddress, clientID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to create client, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }

    if ((rc = MQTTAsync_setCallbacks(_client, _client, connlost, msgarrvd, NULL)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }

    _conn_opts.keepAliveInterval = 20;
    _conn_opts.cleansession = 1;
    _conn_opts.onSuccess = onConnect;
    _conn_opts.onFailure = onConnectFailure;
    _conn_opts.context = _client;
    if ((rc = MQTTAsync_connect(client, &_conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }
}

Mqtt::~Mqtt()
{
}

#endif