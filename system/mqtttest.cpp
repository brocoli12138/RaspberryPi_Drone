#include "mqtt.h"

// The QoS for sending data
const int QOS = 1;
const std::string DFLT_SERVER_ADDRESS{"mqtt://192.168.50.89:1883"};
const std::string CLIENT_ID{"paho_cpp_async_publish"};
const std::string PUBTOPIC{"data/video"};
const std::string SUBTOPIC{"data/control"};
int main()
{
    char msg[]{0x01, 0x02, 0x03};
    Mqttclient client{DFLT_SERVER_ADDRESS, CLIENT_ID, PUBTOPIC, QOS};
    client.subscribe(SUBTOPIC, QOS);
    client.publish(msg, QOS);
    //std::cout << client.get_newest_message();
    return 0;
}