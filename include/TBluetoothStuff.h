#pragma once
#include <Arduino.h>
#include <BluetoothSerial.h> // работает в 1.0.6; в 2.0.0 ... 2.0.4 не работает из-за багов в BluetoothSerial ((
#include "TTgamPacketParser.h"
//#include "TRingBufferInItem.h"

namespace MyApplication {class TMyApplication;}

namespace BluetoothStuff
{
using namespace std;
using namespace TgamPacketParser;
//using namespace RingBufferInItem;
using namespace MyApplication;

//typedef void (*tpfn_callback)(unsigned char code, unsigned char *data, void *arg);

class TBluetoothDataProcessor;

class TBluetoothStuff
{
  private:
    static int ref_cnt;
    String dev_name;
    static TMyApplication *p_app;
    static tpfn_callback pfn_callback;

    BluetoothSerial SerialBT;

    String MACadd;
    uint8_t address[6];
    String name;
    String pin;
    static bool connected;

    static TTgamPacketParser *p_tpp;
    static TBluetoothDataProcessor *dp;

    static void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
    static void on_data(const uint8_t *buffer, size_t size);
    static void task(void *p);

    friend class TBluetoothDataProcessor;

  public:
    TBluetoothStuff(String dev_name, TMyApplication *p_app, tpfn_callback pfn_callback);
    ~TBluetoothStuff();
};
}
