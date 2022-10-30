#pragma once
#include <Arduino.h>

namespace Task
{
using namespace std;

typedef void(*TCbTaskFunction)(void *);

class TTask
{
private:
    const size_t sz_stack;
    TaskHandle_t h_task;

public:
    TTask(
        TCbTaskFunction cb,
        const char *name,
        const size_t sz_stack,
        void *p_par,
        UBaseType_t priority
    );

    TTask(
        TCbTaskFunction cb,
        const char *name,
        const size_t sz_stack,
        void *p_par,
        UBaseType_t priority,
        const BaseType_t core_id
    );

    ~TTask();

    TaskHandle_t get_handle();
    char *get_name();
    size_t GetMaxStackSizeUsage(void);
};
}