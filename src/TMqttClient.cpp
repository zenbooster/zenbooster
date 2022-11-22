#include "TMQTTClient.h"
#include "TWiFiStuff.h"
#include "TWorker/TWorker.h"
#include <esp_mac.h>
#include <StreamString.h>
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

QueueHandle_t TMQTTClient::queue;
TTask *TMQTTClient::p_conn_task = NULL;

void TMQTTClient::conn_task(void *p)
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

    TTask *t = p_conn_task;
    p_conn_task = NULL;
    delete t;
}

TMQTTClient::TMQTTClient()
{
    uint8_t chipid[6];
    esp_efuse_mac_get_default(chipid);
    s_dev_id = TWorker::sprintf("zenbooster/%02x:%02x:%02x:%02x:%02x:%02x", chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]).get();

    p_ssl_cli = new SSLClient(wf_cli, TAs, (size_t)TAs_NUM, A0, 1, SSLClient::SSL_ERROR);
    p_mqtt_cli = new MqttClient(p_ssl_cli);

    p_mqtt_cli->setId(s_dev_id.c_str());
    p_mqtt_cli->setUsernamePassword(user, pass);

    queue = xQueueCreate(4, sizeof(DynamicJsonDocument *));
    if (queue == NULL) {
        throw String("TMQTTClient::TMQTTClient(..): ошибка создания очереди");
    }
}

TMQTTClient::~TMQTTClient()
{
    if(queue)
    {
        vQueueDelete(queue);
    }

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
    if(!p_conn_task)
    {
        p_conn_task = new TTask(conn_task, "TMQTTClient", TMQTTCLIENT_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2);
    }
}

bool TMQTTClient::ProcessQueue(void)
{
  bool res;
  DynamicJsonDocument *p = NULL;

  res = xQueuePeek(queue, &p, 0);
  if(res)
  {
    StreamString ss;
    serializeJson((*p)["msg"], ss);

    do
    {
        String topic = "devices/" + s_dev_id + "/";
        String part = (*p)["topic"];
        topic += part;

        Serial.printf("Sending message (%s)...", topic.c_str());
        if(!p_mqtt_cli->beginMessage(topic))
        {
            Serial.println("Err (beginMessage)!");
            res = false;
            break;
        }
        if(!p_mqtt_cli->print(ss))
        {
            Serial.println("Err (print)!");
            res = false;
            break;
        }
        if(!p_mqtt_cli->endMessage())
        {
            Serial.println("Err (endMessage)!");
            res = false;
            break;
        }
        Serial.println("Ok!");
        // теперь можно извлечь сообщение:
        res = xQueueReceive(queue, &p, 0);
        delete p;
    } while(false);
  }
  return res;
}

void TMQTTClient::run(void)
{
    if(p_conn_task)
        return;

    if(!p_mqtt_cli->connected())
    {
        connect();
        return;
    }

    p_mqtt_cli->poll();
    
    for(; ProcessQueue();)
    {
    }
}

bool TMQTTClient::is_connected()
{
    return p_mqtt_cli->connected();
}

void TMQTTClient::send(const char *topic, const DynamicJsonDocument *p)
{
  if(p_mqtt_cli)
  {
    DynamicJsonDocument *pdoc = new DynamicJsonDocument(256);
    (*pdoc)["topic"] = topic;
    (*pdoc)["msg"] = *p;
    xQueueSend(queue, &pdoc, portMAX_DELAY);
  }
}
}