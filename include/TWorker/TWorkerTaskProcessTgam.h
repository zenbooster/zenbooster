#pragma once
#include "TWorker/TWorker.h"
#include "TWorker/TWorkerTaskAsyncBase.h"
#include "TTgamParsedValues.h"
#include "TBluetoothStuff.h"

namespace Worker
{
using namespace TgamParsedValues;

class TWorkerTaskProcessTgam: public TWorkerTaskAsyncBase
{
private:
    BluetoothStuff::tpfn_callback cb;
    TTgamParsedValues v;

public:
    TWorkerTaskProcessTgam(BluetoothStuff::tpfn_callback cb, const TTgamParsedValues& v);

    void run(void);
};
}