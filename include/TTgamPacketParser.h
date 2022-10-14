#pragma once
#include <BluetoothSerial.h>
#include "TTgamParsedValues.h"
namespace TgamPacketParser
{
using namespace TgamParsedValues;

enum TEnumState
{
  e_sync,
  e_sync_check,
  e_payload_length,
  e_payload,
  e_chksum/*,
  e_wait_hi,
  e_wait_lo*/
};

typedef void (*tpfn_data_callback)(const TTgamParsedValues& tpv);

class TTgamPacketParser
{
  private:
    static const char SYNC = 0xAA;
    static const char EXCODE = 0x55;

    BluetoothSerial *p_serial;

    TEnumState state;
    uint8_t checksum;
    int payload_length;
    int payload_bytes_received;

    uint8_t payload[256];
    uint8_t pLength;
    uint8_t c;
    uint8_t i;

    tpfn_data_callback data_callback;

    static int int_from_12bit(const uint8_t *buf);

  public:
    TTgamPacketParser(BluetoothSerial *p, tpfn_data_callback data_callback);
    void run(uint8_t b);
    void parse_payload(void);
};
}
