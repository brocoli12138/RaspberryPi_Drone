#ifndef __MQTT_H__
#define __MQTT_H__

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <atomic>
#include <chrono>
#include "mqtt/async_client.h"

// The will message when connection is interrupted
const char *LWT_PAYLOAD = "Last will and testament.";

// Changes may take place anytime when new mqtt messages arrive.
mqtt::const_message_ptr NEWEST_MESSAGE;

/////////////////////////////////////////////////////////////////////////////

/**
 * A Callback class for use with the main MQTT client.
 */
class Callback : public virtual mqtt::callback
{
private:
    /* data */
public:
    Callback(/* args */) {};
    /**
     * This method is called when the client is connected.
     * Note that, in response to an initial connect(), the token from the
     * connect call is also signaled with an on_success(). That occurs just
     * before this is called.
     */
    void connected(const std::string & /*cause*/) override
    {
        std::cout << "\nConnection success!" << std::endl;
    };
    /**
     * This method is called when the connection to the server is lost.
     */
    void connection_lost(const std::string &cause) override
    {
        std::cout << "\nConnection lost" << std::endl;
        if (!cause.empty())
            std::cout << "\tcause: " << cause << std::endl;
    };
    /**
     * This method is called when a message arrives from the server.
     */
    void message_arrived(mqtt::const_message_ptr msg) override
    {
        NEWEST_MESSAGE = msg;
    };
    /**
     * Called when delivery for a message has been completed, and all
     * acknowledgments have been received.
     */
    void delivery_complete(mqtt::delivery_token_ptr tok) override {
        // std::cout << "\tDelivery complete for token: " << (tok ? tok->get_message_id() : -1) << std::endl;
    };

    ~Callback() {};
};

class Mqttclient : public mqtt::async_client
{
private:
    Callback _callback;
    mqtt::connect_options _connectOption;
    mqtt::token_ptr _connectToken;
    mqtt::topic _publishTopic;
    std::string _serverURI;

public:
    Mqttclient(const std::string &serverURI, const std::string &clientId, const std::string &publishTopic, int qos);
    std::string get_newest_message()
    {
        return NEWEST_MESSAGE ? NEWEST_MESSAGE->to_string() : "";
    };
    // using default subscribe
    // token_ptr subscribe(const string& topicFilter, int qos) override;
    mqtt::delivery_token_ptr publish(std::vector<uint8_t> payload, int qos, bool retained = false)
    {
        mqtt::binary_ref payload_ref(reinterpret_cast<const char *>(payload.data()), payload.size());
        std::cout << "payloadlength: " << payload_ref.length() << std::endl;
        return _publishTopic.publish(payload_ref, qos, retained);
    };
    ~Mqttclient();
};

Mqttclient::Mqttclient(const std::string &serverURI, const std::string &clientId, const std::string &publishTopic, int qos)
    : async_client(serverURI, clientId, 128, nullptr), _callback(), _publishTopic(*this, publishTopic, qos), _serverURI(serverURI),
      _connectOption(mqtt::connect_options_builder().clean_session().v5().will(mqtt::message("wills", LWT_PAYLOAD, strlen(LWT_PAYLOAD), 1, false)).automatic_reconnect(std::chrono::seconds(1), std::chrono::seconds(10)).finalize()),
      _connectToken()
{
    set_callback(_callback);
    _connectToken = connect(_connectOption);
    _connectToken->wait();
    auto rsp = _connectToken->get_connect_response();
    // Make sure we were granted a v5 connection.
    if (rsp.get_mqtt_version() < MQTTVERSION_5)
    {
        throw mqtt::security_exception{1, std::string{"Did not get an MQTT v5 connection."}};
    }
}

Mqttclient::~Mqttclient()
{
    disconnect()->wait();
}

/////////////////////////////////////////////////////////////////////////////

#endif