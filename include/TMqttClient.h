#pragma once
#include <WiFiClient.h>
#include <SSLClient.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include "TSingleton.h"
#include "TTask.h"

namespace Conf {class TConf;}
namespace Task {class TTask;}

namespace MQTTClient
{
using namespace Singleton;
using namespace Task;

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

    static QueueHandle_t queue;
    static TTask *p_conn_task;

    static void conn_task(void *p);

    static bool ProcessQueue(void);

public:
    TMQTTClient();
    ~TMQTTClient();

    static void connect();
    static void run(void);

    static bool is_connected();
    static void send(const char *topic, const DynamicJsonDocument *p);
};
}