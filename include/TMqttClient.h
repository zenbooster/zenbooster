#pragma once
//#include <PubSubClient.h>
#include <WiFiClient.h>
//#include <SSLClient.h>
#include <ArduinoBearSSL.h>
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
    static const char cert[];

    friend class Conf::TConf;

    static WiFiClient wf_cli;
    //static SSLClient *p_ssl_cli;
    static BearSSLClient *p_ssl_cli;
    //static PubSubClient *p_mqtt_cli;
    static MqttClient *p_mqtt_cli;

public:
    TMQTTClient();
    ~TMQTTClient();
};
}