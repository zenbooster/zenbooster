#include "TMQTTClient.h"
#include "TWiFiStuff.h"
#include "TWorker/TWorker.h"
#include <esp_mac.h>
#include "mqtt_certificates.h"

namespace MQTTClient
{
using namespace WiFiStuff;
using namespace Worker;

String TMQTTClient::server;
uint16_t TMQTTClient::port;
String TMQTTClient::user;
String TMQTTClient::pass;

WiFiClient TMQTTClient::wf_cli;
SSLClient *TMQTTClient::p_ssl_cli;
MqttClient *TMQTTClient::p_mqtt_cli;
String TMQTTClient::s_dev_id;

TMQTTClient::TMQTTClient()
{
    uint8_t chipid[6];
    esp_efuse_mac_get_default(chipid);
    s_dev_id = TWorker::sprintf("zenbooster/%02x:%02x:%02x:%02x:%02x:%02x", chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]).get();

    p_ssl_cli = new SSLClient(wf_cli, TAs, (size_t)TAs_NUM, A0, 1, SSLClient::SSL_ERROR);
    p_mqtt_cli = new MqttClient(p_ssl_cli);

    p_mqtt_cli->setId(s_dev_id.c_str());
    p_mqtt_cli->setUsernamePassword(user, pass);
}

TMQTTClient::~TMQTTClient()
{
    if(p_mqtt_cli)
    {
        delete p_mqtt_cli;
    }

    if(p_ssl_cli)
    {
        delete p_ssl_cli;
    }
}

void TMQTTClient::connect()
{
    Serial.print("Attempting to MQTT broker...");
    if(p_mqtt_cli->connect(server.c_str(), port))
    {
        Serial.println("Ok!");
    }
    else
    {
        Serial.println(TWorker::sprintf("Err (%d)!", p_mqtt_cli->connectError()).get());
    }
}

void TMQTTClient::run(void)
{
    p_mqtt_cli->poll();

    if(!p_mqtt_cli->connected())
    {
        connect();
    }
    
    if(p_mqtt_cli->connected())
    {
        do
        {
            String topic = "devices/" + s_dev_id + "/debug";
            Serial.printf("Sending message (%s)...", topic.c_str());
            if(!p_mqtt_cli->beginMessage(topic))
            {
                Serial.println("Err (beginMessage)!");
                break;
            }
            if(!p_mqtt_cli->print("debug"))
            {
                Serial.println("Err (print)!");
                break;
            }
            if(!p_mqtt_cli->endMessage())
            {
                Serial.println("Err (endMessage)!");
                break;
            }
            Serial.println("Ok!");
        } while(false);
    }
}
}