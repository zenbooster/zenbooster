#pragma once
#include "TUtil.h"

namespace Singleton
{
template <class T>
class TSingleton
{
    private:
        static TSingleton *p_instance;

        virtual const char *get_class_name()
        {
            return "TSingleton<T>";
        }

    public:
        TSingleton()
        {
            if(p_instance)
            {
                throw String("Разрешён только один экземпляр ") + get_class_name();
            }
            else
            {
                p_instance = this;
            }
        }

        virtual ~TSingleton() {p_instance = NULL;};

        static TSingleton *get_instance()
        {
            return p_instance;
        }
};

template <class T>
TSingleton<T> *TSingleton<T>::p_instance = NULL;
}