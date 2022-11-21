#pragma once
#include <WiFiClient.h>
#include <SSLClient.h>
#include <ArduinoMqttClient.h>
#include "TSingleton.h"

namespace Conf {class TConf;}

namespace MQTTClient
{
using namespace Singleton;

class TMQTTClient: TSingleton<TMQTTClient>
{
private:
    static String server;
    static uint16_t port;
    static String user;
    static String pass;

    friend class Conf::TConf;

    static WiFiClient wf_cli;
    static SSLClient *p_ssl_cli;
    static MqttClient *p_mqtt_cli;
    static String s_dev_id;

public:
    TMQTTClient();
    ~TMQTTClient();

    void connect();
    void run(void);
};
}