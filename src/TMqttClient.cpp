#include "TMQTTClient.h"
#include "TWiFiStuff.h"
#include "TWorker/TWorker.h"
#include <esp_mac.h>

namespace MQTTClient
{
using namespace WiFiStuff;
using namespace Worker;

String TMQTTClient::server;
uint16_t TMQTTClient::port;
String TMQTTClient::user;
String TMQTTClient::pass;
const char TMQTTClient::cert[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDpzCCAo+gAwIBAgIJAOHc50EeJ6aeMA0GCSqGSIb3DQEBDQUAMGoxFzAVBgNV\n"
"BAMMDkFuIE1RVFQgYnJva2VyMRYwFAYDVQQKDA1Pd25UcmFja3Mub3JnMRQwEgYD\n"
"VQQLDAtnZW5lcmF0ZS1DQTEhMB8GCSqGSIb3DQEJARYSbm9ib2R5QGV4YW1wbGUu\n"
"bmV0MB4XDTE3MDgyNzAyMDQwNFoXDTMyMDgyMzAyMDQwNFowajEXMBUGA1UEAwwO\n"
"QW4gTVFUVCBicm9rZXIxFjAUBgNVBAoMDU93blRyYWNrcy5vcmcxFDASBgNVBAsM\n"
"C2dlbmVyYXRlLUNBMSEwHwYJKoZIhvcNAQkBFhJub2JvZHlAZXhhbXBsZS5uZXQw\n"
"ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC8mQfR058k7LBV7Xt6RiZL\n"
"BIpC5ewSweAnsaS6Ue+jx6uruJsccnIJbiHJVjOcSYV3JLMt3un7d0JTottW6Zx/\n"
"VleYC4CUzf3bcHLe7Qhl0veP9QAdZceblMCbEqAM9227EaXKQVzAYGwRnuxvfJM7\n"
"P84iu2J/zB4o+AEWUJDIW38SO4b+0i1FYHmKKxEx7dztDyCJvTsn35wX3oKlJ5e+\n"
"na/6csD+Ngh9QQ+wDd02TBqXKB7UXyRT5ECesu19U6CTE7Wr9xECbDO262aD6GtC\n"
"HpeSAWqa6HKbKjZ20pL5V5ceS85TdWbvPdVrFUDWMIhK9dNuwhIK0s6VyRcJkbe5\n"
"AgMBAAGjUDBOMB0GA1UdDgQWBBQPZ0u1jIg65jT3Th42VGMnya6VDDAfBgNVHSME\n"
"GDAWgBQPZ0u1jIg65jT3Th42VGMnya6VDDAMBgNVHRMEBTADAQH/MA0GCSqGSIb3\n"
"DQEBDQUAA4IBAQB8JrNsV32aXBOLSX8WqoHuU2NOJH+1fTdGDLYLEAPe/x3YxkNC\n"
"MJDx8TGLSQsz4L0kCsLEQ/qsnkuPJUt3x7eNIMOrvS+zDv9wXqnGwMgeAOtEkG5n\n"
"NbPezM50ovgZAuEZMphp2s/Zl0EROydnqpkweWa5bxYRkiQBlp0nUkCtec3PZ6iO\n"
"QVPj/bHHi4T3jkbL0+oLNShv9Zw9hr6BkKCVqeHe9prlOMVuEJOVrEKzkhhAY90p\n"
"Wwrj+OQYCLllJVD9VQhLVOXLg1bSuqkld3PCQBFNvrvk4Up61wuu/Oj5c5g8vwYd\n"
"0ul29sU3heTw4ZXifpbMXRM36LFNH5uLrSba\n"
"-----END CERTIFICATE-----\n";
// "ISRG_Root_X1.pem":
/*const char TMQTTClient::cert[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
"WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
"ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
"MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
"h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
"0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
"A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
"T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
"B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
"B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
"KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
"OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
"jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
"qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
"rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
"HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
"hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
"ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
"3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
"NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
"ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
"TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
"jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
"oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
"4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
"mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
"emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
"-----END CERTIFICATE-----\n";*/

WiFiClient TMQTTClient::wf_cli;
//SSLClient *TMQTTClient::p_ssl_cli = NULL;
BearSSLClient *TMQTTClient::p_ssl_cli;
//PubSubClient *TMQTTClient::p_mqtt_cli = NULL;
MqttClient *TMQTTClient::p_mqtt_cli;

static unsigned long getTime()
{
    return TWiFiStuff::getEpochTime();
}

TMQTTClient::TMQTTClient()
{
    uint8_t chipid[6];
    esp_efuse_mac_get_default(chipid);
    String s_dev_id = TWorker::sprintf("zenbooster/%02x:%02x:%02x:%02x:%02x:%02x", chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]).get();

    p_ssl_cli = new BearSSLClient(wf_cli);
    p_mqtt_cli = new MqttClient(p_ssl_cli);

    ArduinoBearSSL.onGetTime(getTime);
    p_ssl_cli->setEccSlot(0, cert);
    p_mqtt_cli->setId(s_dev_id.c_str());
    p_mqtt_cli->setUsernamePassword(user, pass);

    Serial.print("Attempting to MQTT broker...");
    while (!p_mqtt_cli->connect(server.c_str(), port))
    {
        Serial.print(".");
        Serial.println(p_mqtt_cli->connectError());
        delay(250);
    }
    Serial.println("Ok!");

    for(;;)
    {
        p_mqtt_cli->poll();
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
        delay(250);
    }
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
}