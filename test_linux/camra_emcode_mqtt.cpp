#include "Rpicamera.h"
#include "mqtt.h"
#include "Encoder.h"

const int QOS = 0;
const std::string DFLT_SERVER_ADDRESS{"mqtt://192.168.50.89:1883"};
const std::string CLIENT_ID{"paho_cpp_async_publish"};
const std::string PUBTOPIC{"data/video"};
const std::string SUBTOPIC{"data/control"};
Mqttclient client{DFLT_SERVER_ADDRESS, CLIENT_ID, PUBTOPIC, QOS};
Encoder encoder;

void encodercallback(const std::vector<uint8_t> & data)
{
    client.publish(data, QOS);
}

void cameracallback(std::vector<unsigned char> data)
{
    int bufflength = data.size();
    std::cout << "bufflength: " << bufflength << std::endl;
    encoder.Async_encode(data, encodercallback);
    std::cout << "sent!" << std::endl;
}

int main()
{
}