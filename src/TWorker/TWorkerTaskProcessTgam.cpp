#include "TWorker/TWorkerTaskProcessTgam.h"

namespace Worker
{
TWorkerTaskProcessTgam::TWorkerTaskProcessTgam(BluetoothStuff::tpfn_callback cb, const TTgamParsedValues& v):
    cb(cb),
    v(v)
{
}

void TWorkerTaskProcessTgam::run(void)
{
    cb(&v, BluetoothStuff::TCallbackEvent::eData);
}
}