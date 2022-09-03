#pragma once
#include <string>
#include <BluetoothSerial.h> // работает в 1.0.6; в 2.0.0 ... 2.0.4 не работает из-за багов в BluetoothSerial ((
#include "TTgamPacketParser.h"

class TMyApplication;

namespace BluetoothStuff
{
using namespace std;
using namespace TgamPacketParser;

//typedef void (*tpfn_callback)(unsigned char code, unsigned char *data, void *arg);

class TBluetoothStuff
{
  private:
    static int ref_cnt;
    string dev_name;
    TMyApplication *p_app;
    tpfn_callback pfn_callback;

    BluetoothSerial SerialBT;

    String MACadd;
    uint8_t address[6];
    String name;
    String pin;
    bool connected;

    TTgamPacketParser *p_tpp;

    static void task(void *p);

  public:
    TBluetoothStuff(string dev_name, TMyApplication *p_app, tpfn_callback pfn_callback);
    ~TBluetoothStuff();
};
}
