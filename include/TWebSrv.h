#pragma once
#include <WebServer.h>

namespace WebSrv
{
class TWebSrv
{
  private:
    static const String responseHTML;
    WebServer srv;
    static int ref_cnt;

  public:
    TWebSrv();
    ~TWebSrv();

    void run(void);
};
}
