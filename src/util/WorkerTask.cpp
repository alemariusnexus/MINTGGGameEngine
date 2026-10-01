#include "WorkerTask.h"

#include "Log.h"
#include "Util.h"


LOG_USE_TAG("WorkerTask")


namespace MINTGGGameEngine
{

WorkerTask::WorkerTask(size_t stackSizeBytes, unsigned int priority, const char* taskName)
    : stackSizeBytes(stackSizeBytes), priority(priority),
      thread(taskName ? taskName : "WorkerTask"), stopRequested(false)
{
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    workQueueMtx = xSemaphoreCreateMutex();
#endif
}

WorkerTask::~WorkerTask()
{
    stop();

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    vSemaphoreDelete(workQueueMtx);
#endif
}

bool WorkerTask::start()
{
    stopRequested = false;

    bool ok = thread.start([this] { taskMain(); }, stackSizeBytes, priority);
    if (!ok) {
        LogError("ERROR: Unable to create task '%s'.", thread.getName().c_str());
        return false;
    }
    return true;
}

bool WorkerTask::stop()
{
    stopRequested = true;

    // TODO: Make wait time configurable
    while (stopRequested) {
        DelayTaskMs(1);
    }

    return !stopRequested;
}

void WorkerTask::addWorkItem(const WorkFunc& func)
{
    lockMutex();

    workQueue.emplace_back();
    WorkItem& item = workQueue.back();
    item.func = func;

    unlockMutex();
}

void WorkerTask::taskMain()
{
    while (!stopRequested) {
        lockMutex();
        while (!workQueue.empty()) {
            WorkItem& item = workQueue.front();
            unlockMutex();
            doItem(item);
            lockMutex();
            workQueue.pop_front();
        }
        unlockMutex();

        // TODO: Do something better (e.g. task notification, or proper queue)
        DelayTaskMs(1);
    }
}

void WorkerTask::doItem(WorkItem& item)
{
    item.func();
}


}
