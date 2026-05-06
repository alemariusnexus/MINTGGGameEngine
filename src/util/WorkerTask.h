#pragma once

#include "../Globals.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

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

private:
    size_t stackSizeBytes;
    unsigned int priority;
    const char* taskName;
    TaskHandle_t task;
    volatile bool stopRequested;

    std::list<WorkItem> workQueue;
    SemaphoreHandle_t workQueueMtx;
};


}
