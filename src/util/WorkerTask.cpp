#include "WorkerTask.h"

#include "Log.h"


LOG_USE_TAG("WorkerTask")


namespace MINTGGGameEngine
{


void WorkerTaskMain(void* params)
{
    static_cast<WorkerTask*>(params)->taskMain();
}


WorkerTask::WorkerTask(size_t stackSizeBytes, unsigned int priority, const char* taskName)
    : stackSizeBytes(stackSizeBytes), priority(priority),
      taskName(taskName ? taskName : "WorkerTask"), task(nullptr), stopRequested(false)
{
    workQueueMtx = xSemaphoreCreateMutex();
}

WorkerTask::~WorkerTask()
{
    stop();
    vSemaphoreDelete(workQueueMtx);
}

bool WorkerTask::start()
{
    stopRequested = false;

    BaseType_t res = xTaskCreate(&WorkerTaskMain, taskName, stackSizeBytes,
            this, priority, &task);
    if (res != pdPASS) {
        LogError("ERROR: Unable to create task '%s'.", taskName);
        return false;
    }
    return true;
}

bool WorkerTask::stop()
{
    stopRequested = true;

    // TODO: Make wait time configurable
    while (stopRequested) {
        vTaskDelay(1);
    }

    return !stopRequested;
}

void WorkerTask::addWorkItem(const WorkFunc& func)
{
    xSemaphoreTake(workQueueMtx, portMAX_DELAY);

    workQueue.emplace_back();
    WorkItem& item = workQueue.back();
    item.func = func;

    xSemaphoreGive(workQueueMtx);
}

void WorkerTask::taskMain()
{
    while (!stopRequested) {
        xSemaphoreTake(workQueueMtx, portMAX_DELAY);
        while (!workQueue.empty()) {
            WorkItem& item = workQueue.front();
            xSemaphoreGive(workQueueMtx);
            doItem(item);
            xSemaphoreTake(workQueueMtx, portMAX_DELAY);
            workQueue.pop_front();
        }
        xSemaphoreGive(workQueueMtx);

        // TODO: Do something better (e.g. task notification, or proper queue)
        vTaskDelay(1);
    }
}

void WorkerTask::doItem(WorkItem& item)
{
    item.func();
}


}
