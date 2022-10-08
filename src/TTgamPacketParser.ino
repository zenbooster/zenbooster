#include <exception>
#include "TTgamPacketParser.h"

namespace TgamPacketParser
{
using namespace std;

TTgamPacketParser::TTgamPacketParser(BluetoothSerial *p, tpfn_data_callback data_callback, tpfn_callback callback, void *cb_arg):
  p_serial(p),
  state(e_sync),
  payload_length(0),
  payload_bytes_received(0),
  data_callback(data_callback),
  callback(callback),
  cb_arg(cb_arg)
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
        state = e_payload;
      }
      break;
    
    case e_payload:
      payload[payload_bytes_received++] = b;
      if(payload_bytes_received >= payload_length)
      {
        parse_payload();
        state = e_sync;
      }
      break;
  }
}

void TTgamPacketParser::parse_payload(void)
{
  uint8_t i = 0;
  uint8_t extendedCodeLevel = 0;
  uint8_t code = 0;
  uint8_t numBytes = 0;

  /* Parse all bytes from the payload[] */
  while( i < payload_length )
  {
    /* Parse possible EXtended CODE bytes */
    while( payload[i] == 0x55 )
    {
        extendedCodeLevel++;
        i++;
    }

    /* Parse CODE */
    code = payload[i++];

    /* Parse value length */
    if( code >= 0x80 ) numBytes = payload[i++];
    else               numBytes = 1;

    /* Call the callback function to handle the DataRow value */
    /*if( parser->handleDataValue ) {
        parser->handleDataValue( extendedCodeLevel, code, numBytes,
                                parser->payload+i, parser->customData );
    }*/
    if(code == 0x83)
    {
      if(data_callback)
      {
        //Serial.printf("HIT.2\n");
        data_callback(payload+i, numBytes);
      }
    }
    i = (uint8_t)(i + numBytes);
  }
}

/*int TTgamPacketParser::parse_data(const uint8_t *payload, size_t pLength)
{
  uint8_t bytesParsed = 0;
  uint8_t code;
  uint8_t length;
  uint8_t extendedCodeLevel;

  // Loop until all bytes are parsed from the payload[] array...
  while( bytesParsed < pLength )
  {
    // Parse the extendedCodeLevel, code, and length
    extendedCodeLevel = 0;

    while( payload[bytesParsed] == EXCODE )
    {
        extendedCodeLevel++;
        bytesParsed++;
    }

    code = payload[bytesParsed++];

    if( code & 0x80 )
    {
      length = payload[bytesParsed++];
    }
    else
    {
      length = 1;
    }
    // TODO: Based on the extendedCodeLevel, code, length,
    // and the [CODE] Definitions Table, handle the next
    // "length" bytes of data from the payload as
    // appropriate for your application.

    //if(code != 0x80)
    //{
    //  SerialPrintf( "EXCODE level: %d CODE: 0x%02X length: %d\n", extendedCodeLevel, code, length );
    //  SerialPrintf( "Data value(s):" );
    //  for( i=0; i<length; i++ )
    //  {
    //    SerialPrintf( " %02X", payload[bytesParsed+i] & 0xFF );
    //  }
    //  SerialPrintf( "\n" );
    //}
    if(callback)
    {
      callback(payload + bytesParsed, cb_arg);
    }
    
    // Increment the bytesParsed by the length of the Data Value
    bytesParsed += length;
  }
  return( 0 );
}*/
}
