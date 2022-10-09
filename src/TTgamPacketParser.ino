#include <exception>
#include <time.h>
#include "TTgamPacketParser.h"
#include "TUtil.h"

namespace TgamPacketParser
{
using namespace std;

TTgamPacketParser::TTgamPacketParser(BluetoothSerial *p, tpfn_data_callback data_callback):
  p_serial(p),
  state(e_sync),
  payload_length(0),
  payload_bytes_received(0),
  data_callback(data_callback)
{
  //
}

void TTgamPacketParser::run(uint8_t b)
{
  //Serial.printf("HIT.0: b=%x, state=%d\n", b, state);
  switch(state)
  {
    case e_sync:
      if(b == SYNC)
      {
        state = e_sync_check;
      }
      break;

    case e_sync_check:
      if(b == SYNC)
      {
        state = e_payload_length;
      }
      else
      {
        state = e_sync;
      }
      break;
    
    case e_payload_length:
      payload_length = b;

      if(payload_length > 170)
      {
        state = e_sync;
      }
      else
      if(payload_length == 170)
      {
        //
      }
      else
      {
        payload_bytes_received = 0;
        checksum = 0;
        state = e_payload;
      }
      break;
    
    case e_payload:
      payload[payload_bytes_received++] = b;
      checksum = (uint8_t)(checksum + b);
      if(payload_bytes_received >= payload_length)
      {
        state = e_chksum;
      }
      break;

    case e_chksum:
      state = e_sync;
      if(b == ((~checksum) & 0xff))
      {
        parse_payload();
      }
      break;
  }
}

int TTgamPacketParser::int_from_12bit(const uint8_t *buf)
{
  return (*buf << 16) + (buf[1] << 8) + buf[2];
}

void TTgamPacketParser::parse_payload(void)
{
  uint8_t i = 0;
  uint8_t extendedCodeLevel = 0;
  uint8_t code = 0;
  uint8_t numBytes = 0;
  TRingBufferInItem rbi = {};
  bool is_has_83 = false;
  
  /* Parse all bytes from the payload[] */
  while( i < payload_length )
  {
    /* Parse possible EXtended CODE bytes */
    while( payload[i] == EXCODE )
    {
        extendedCodeLevel++;
        i++;
    }

    /* Parse CODE */
    code = payload[i++];

    /* Parse value length */
    if( code >= 0x80 )
    {
      numBytes = payload[i++];
    }
    else
    {
      numBytes = 1;
    }

    if(code == 0x83)
    {
      is_has_83 = true;

      uint8_t *data = payload + i;
      time_t now;
      time(&now);

      rbi.time = now;
      rbi.delta = int_from_12bit(data);
      rbi.theta = int_from_12bit(data + 3);
      rbi.alpha_lo = int_from_12bit(data + 6);
      rbi.alpha_hi = int_from_12bit(data + 9);
      rbi.beta_lo = int_from_12bit(data + 12);
      rbi.beta_hi = int_from_12bit(data + 15);
      rbi.gamma_lo = int_from_12bit(data + 18);
      rbi.gamma_md = int_from_12bit(data + 21);
    }
    else
    if(code == 4)
    {
      rbi.esense_att = payload[i];
    }
    else
    if(code == 5)
    {
      rbi.esense_med = payload[i];
    }

    i = (uint8_t)(i + numBytes);
  }

  if(data_callback && is_has_83)
  {
    data_callback(rbi);
  }
}
}
