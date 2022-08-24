#include "TWebSrv.h"

namespace WebSrv
{
const String TWebSrv::responseHTML =
  "<!DOCTYPE html><html>"
  "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
  "</style></head>"
  "<body><h1>ESP32 Web Server</h1>"
  "<p>Hello World</p>"
  "</body></html>";
int TWebSrv::ref_cnt = 0;

void TWebSrv::run(void)
{
    srv.handleClient();
}

TWebSrv::TWebSrv():
  srv(80)
{
  if(ref_cnt)
  {
    throw "Only one instance of TWebSrv allowed!";
  }
  ref_cnt++;

  srv.onNotFound([this]() {
    srv.send(200, "text/html", responseHTML);
  });

  srv.begin();
}

TWebSrv::~TWebSrv()
{
  --ref_cnt;
}
}
