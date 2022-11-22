#pragma once
#include <Arduino.h>
#include <functional>

namespace Task
{
using namespace std;

class TTask
{
private:
    const size_t sz_stack;
    TaskHandle_t h_task;

public:
    TTask(
        TaskFunction_t cb,
        const char *name,
        const size_t sz_stack,
        void *p_par,
        UBaseType_t priority
    );

    TTask(
        TaskFunction_t cb,
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