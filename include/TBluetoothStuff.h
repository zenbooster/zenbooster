#pragma once
#include <Arduino.h>
#include <BluetoothSerial.h>
#include "TTgamPacketParser.h"

namespace MyApplication {class TMyApplication;}

namespace BluetoothStuff
{
using namespace std;
using namespace TgamPacketParser;
using namespace MyApplication;

enum TCallbackEvent
{
  eConnect,
  eDisconnect,
  eData
};

typedef void (*tpfn_callback)(const TTgamParsedValues *p_tpv, TCallbackEvent evt);

class TBluetoothDataProcessor;

class TBluetoothStuff
{
  private:
    static int ref_cnt;
    String dev_name;
    static tpfn_callback pfn_callback;

    BluetoothSerial SerialBT;

    String MACadd;
    uint8_t address[6];
    String name;
    String pin;

    static SemaphoreHandle_t xConnSemaphore;
    static bool is_connected;

    static TTgamPacketParser *p_tpp;
    static TBluetoothDataProcessor *dp;

    static void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
    static void task(void *p);

    friend class TBluetoothDataProcessor;

  public:
    TBluetoothStuff(String dev_name, tpfn_callback pfn_callback);
    ~TBluetoothStuff();
};
}
