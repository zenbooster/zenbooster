#include <exception>
#include "TTgamPacketParser.h"

namespace TgamPacketParser
{
using namespace std;

TTgamPacketParser::TTgamPacketParser(BluetoothSerial *p, tpfn_callback callback, void *cb_arg):
  p_serial(p),
  callback(callback),
  cb_arg(cb_arg)
{
  //
}

uint8_t TTgamPacketParser::_read_byte(void)
{
  int r;

  do
  {
    r = p_serial->read();
  } while(r == -1 && p_serial->connected());

  if(r == -1)
    throw exception();

  return r & 0xff;
}

void TTgamPacketParser::run(void)
{
  c = _read_byte();
  if( c != SYNC ) return;
  c = _read_byte();
  if( c != SYNC ) return;
  // Parse [PLENGTH] byte
  while( true )
  {
    pLength = _read_byte();
    
    if( pLength != 170 ) break;
  }
  if( pLength > 169 ) return;
  // Collect [PAYLOAD...] bytes
  for(int j = 0; j < pLength; j++)
    payload[j] = _read_byte();

  // Calculate [PAYLOAD...] checksum
  checksum = 0;
  for( i=0; i<pLength; i++ ) checksum += payload[i];
  checksum &= 0xFF;
  checksum = ~checksum & 0xFF;
  // Parse [CKSUM] byte
  c = _read_byte();
  // Verify [CKSUM] byte against calculated [PAYLOAD...] checksum
  if( c != checksum ) return;
  // Since [CKSUM] is OK, parse the Data Payload
  _parse_payload( payload, pLength );
}

int TTgamPacketParser::_parse_payload( unsigned char *payload, unsigned char pLength )
{
  unsigned char bytesParsed = 0;
  unsigned char code;
  unsigned char length;
  unsigned char extendedCodeLevel;

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
    if( code & 0x80 ) length = payload[bytesParsed++];
    else length = 1;
    // TODO: Based on the extendedCodeLevel, code, length,
    // and the [CODE] Definitions Table, handle the next
    // "length" bytes of data from the payload as
    // appropriate for your application.

    /*if(code != 0x80)
    {
      SerialPrintf( "EXCODE level: %d CODE: 0x%02X length: %d\n", extendedCodeLevel, code, length );
      SerialPrintf( "Data value(s):" );
      for( i=0; i<length; i++ )
      {
        SerialPrintf( " %02X", payload[bytesParsed+i] & 0xFF );
      }
      SerialPrintf( "\n" );
    }*/
    if(callback)
      callback(code, payload + bytesParsed, cb_arg);
    
    // Increment the bytesParsed by the length of the Data Value
    bytesParsed += length;
  }
  return( 0 );
}
}
