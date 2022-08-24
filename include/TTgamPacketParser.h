#pragma once
#include <BluetoothSerial.h>

namespace TgamPacketParser
{
class TTgamPacketParser
{
  private:
    typedef void (*tpfn_calback)(unsigned char code, unsigned char *data, void *arg);
  
    static const char SYNC = 0xAA;
    static const char EXCODE = 0x55;

    BluetoothSerial *p_serial;

    int checksum;
    unsigned char payload[256];
    unsigned char pLength;
    unsigned char c;
    unsigned char i;

    tpfn_calback callback;
    void *cb_arg;

    uint8_t _read_byte(void);
    int _parse_payload( unsigned char *payload, unsigned char pLength );

  public:
    TTgamPacketParser(BluetoothSerial *p, tpfn_calback callback, void *cb_arg = NULL);
    void run(void);
};
}
