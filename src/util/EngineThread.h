#pragma once

#include "../Globals.h"

#include <functional>

#include <string>

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <thread>
#else
#   include <freertos/FreeRTOS.h>
#   include <freertos/task.h>
#endif


namespace MINTGGGameEngine
{

class EngineThread
{
public:
    typedef std::function<void ()> MainFunc;

public:
    EngineThread(const std::string& name);
    virtual ~EngineThread();

    bool start(const MainFunc& main, size_t stackSize, unsigned int priority);
    void stopAndWait();

    bool isStopRequested() const;

    std::string getName() const { return name; }

private:
    static void staticMain(void* thread);
    void main();

private:
    std::string name;
    MainFunc mainFunc;

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    std::thread thread;
#else
    TaskHandle_t task;
#endif

    volatile bool stopRequested;
};

}
