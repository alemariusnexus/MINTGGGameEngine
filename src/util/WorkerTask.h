#pragma once

#include "../Globals.h"

#include "EngineThread.h"

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <mutex>
#else
#   include <freertos/semphr.h>
#endif

#include <functional>
#include <list>


namespace MINTGGGameEngine
{


class WorkerTask
{
    friend void WorkerTaskMain(void* params);

public:
    typedef std::function<void()> WorkFunc;

private:
    struct WorkItem
    {
        WorkFunc func;
    };

public:
    WorkerTask(size_t stackSizeBytes, unsigned int priority, const char* taskName = nullptr);
    ~WorkerTask();

    bool start();
    bool stop();

    void addWorkItem(const WorkFunc& func);

private:
    void taskMain();

    void doItem(WorkItem& item);

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    void lockMutex() { workQueueMtx.lock(); }
    void unlockMutex() { workQueueMtx.unlock(); }
#else
    void lockMutex() { xSemaphoreTake(workQueueMtx, portMAX_DELAY); }
    void unlockMutex() { xSemaphoreGive(workQueueMtx); }
#endif

private:
    size_t stackSizeBytes;
    unsigned int priority;
    EngineThread thread;
    volatile bool stopRequested;

    std::list<WorkItem> workQueue;

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    std::mutex workQueueMtx;
#else
    SemaphoreHandle_t workQueueMtx;
#endif
};


}
