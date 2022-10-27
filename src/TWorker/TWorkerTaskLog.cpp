#include "TWorker/TWorkerTaskLog.h"

namespace Worker
{
TWorkerTaskLog::TWorkerTaskLog(const String& text):
    text(text)
{
}

void TWorkerTaskLog::run(void)
{
    Serial.print(text);
}
}