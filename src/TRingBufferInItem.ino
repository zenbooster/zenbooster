#include "TRingBufferInItem.h"

namespace RingBufferInItem
{
TRingBufferInItem& TRingBufferInItem::operator=(const TTgamParsedValues& other)
{
    if(this != (TRingBufferInItem*)&other)
    {
        *this = *(TRingBufferInItem*)&other;
    }
    return *this;
}
}