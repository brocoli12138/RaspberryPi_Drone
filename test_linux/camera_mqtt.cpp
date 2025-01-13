#include "Rpicamera.h"
#include "mqtt.h"
#include <vector>

const int QOS = 0;
const std::string DFLT_SERVER_ADDRESS{"mqtt://192.168.50.89:1883"};
const std::string CLIENT_ID{"paho_cpp_async_publish"};
const std::string PUBTOPIC{"data/video"};
const std::string SUBTOPIC{"data/control"};
Mqttclient client{DFLT_SERVER_ADDRESS, CLIENT_ID, PUBTOPIC, QOS};

void mycallback(std::vector<unsigned char> data)
{
    int bufflength = data.size();
    std::cout << "bufflength: " << bufflength << std::endl;
    client.publish(data, QOS);
    std::cout << "sent!" << std::endl;
}

int main()
{
    Rpicamera camera;
    camera.start(mycallback);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}