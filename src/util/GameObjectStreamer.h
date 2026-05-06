#pragma once

#include "../Globals.h"

#include "../core/GameObject.h"
#include "WorkerTask.h"

#include <vector>


namespace MINTGGGameEngine
{


class GameObjectStreamer
{
private:
    enum StreamFlags
    {
        StreamFlagsSpriteBitmapFile = 0x00000001,

        StreamFlagsActive = 0x80000000
    };

    struct StreamedObject
    {
        GameObject gobj;
        float width;
        float height;
        uint32_t flags;

        struct {
            std::string path;
            uint16_t ox;
            uint16_t oy;
            uint16_t w;
            uint16_t h;
        } bmpFile;
    };

public:
    GameObjectStreamer();
    ~GameObjectStreamer();

    void registerGameObject(const GameObject& gobj, float width, float height);
    bool unregisterGameObject(const GameObject& gobj);

    bool enableSpriteBitmapFile (
        const GameObject& gobj,
        const char* path,
        uint16_t ox = 0, uint16_t oy = 0,
        uint16_t w = UINT16_MAX, uint16_t h = UINT16_MAX
        );
    bool disableSpriteBitmapFile(const GameObject& gobj);

    void setActiveArea(float x, float y, float w, float h);

    size_t getActiveCount() const;
    size_t getMemoryUsage() const;

private:
    StreamedObject* findObject(const GameObject& gobj);

    void update();

    void streamIn(StreamedObject& sobj);
    void streamOut(StreamedObject& sobj);

private:
    std::vector<StreamedObject> streamedObjs;

    float activeX;
    float activeY;
    float activeW;
    float activeH;

    WorkerTask workerTask;
};


}
