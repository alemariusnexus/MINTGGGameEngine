#include "GameObjectStreamer.h"

#include "Log.h"
#include "MathUtils.h"
#include "Util.h"


LOG_USE_TAG("GameObjectStreamer")


namespace MINTGGGameEngine
{


GameObjectStreamer::GameObjectStreamer()
    : activeX(0), activeY(0), activeW(0), activeH(0), workerTask(4096, 1)
{
    workerTask.start();
}

GameObjectStreamer::~GameObjectStreamer()
{
    workerTask.stop();
}

void GameObjectStreamer::registerGameObject(const GameObject& gobj, float width, float height)
{
    StreamedObject* sobj = findObject(gobj);
    if (!sobj) {
        streamedObjs.emplace_back();
        sobj = &streamedObjs.back();
        sobj->gobj = gobj;
        sobj->flags = 0;
    }
    sobj->width = width;
    sobj->height = height;
}

bool GameObjectStreamer::unregisterGameObject(const GameObject& gobj)
{
    for (auto it = streamedObjs.begin() ; it != streamedObjs.end() ; ++it) {
        if (it->gobj == gobj) {
            streamedObjs.erase(it);
            return true;
        }
    }
    return false;
}

bool GameObjectStreamer::enableSpriteBitmapFile (
    const GameObject& gobj,
    const char* path,
    uint16_t ox, uint16_t oy,
    uint16_t w, uint16_t h
) {
    StreamedObject* sobj = findObject(gobj);
    if (!sobj) {
        return false;
    }
    sobj->flags |= StreamFlagsSpriteBitmapFile;
    sobj->bmpFile.path = path;
    sobj->bmpFile.ox = ox;
    sobj->bmpFile.oy = oy;
    sobj->bmpFile.w = w;
    sobj->bmpFile.h = h;
    return true;
}

bool GameObjectStreamer::disableSpriteBitmapFile(const GameObject& gobj)
{
    StreamedObject* sobj = findObject(gobj);
    if (!sobj) {
        return false;
    }
    sobj->flags &= ~StreamFlagsSpriteBitmapFile;
    sobj->bmpFile.path.clear();
    return true;
}

void GameObjectStreamer::setActiveArea(float x, float y, float w, float h)
{
    if (x == activeX  &&  y == activeY  &&  w == activeW  &&  h == activeH) {
        return;
    }

    activeX = x;
    activeY = y;
    activeW = w;
    activeH = h;

    update();
}

size_t GameObjectStreamer::getActiveCount() const
{
    size_t numActive = 0;
    for (const StreamedObject& sobj : streamedObjs) {
        if ((sobj.flags & StreamFlagsActive) != 0) {
            numActive++;
        }
    }
    return numActive;
}

size_t GameObjectStreamer::getMemoryUsage() const
{
    size_t memUsage = 0;
    for (const StreamedObject& sobj : streamedObjs) {
        if ((sobj.flags & StreamFlagsActive) != 0) {
            Sprite sprite = sobj.gobj.getSprite();
            if (sprite.getType() == Sprite::Type::Bitmap) {
                memUsage += sprite.getBitmap().getMemoryUsage();
            }
        }
    }
    return memUsage;
}

GameObjectStreamer::StreamedObject* GameObjectStreamer::findObject(const GameObject& gobj)
{
    for (auto it = streamedObjs.begin() ; it != streamedObjs.end() ; ++it) {
        if (it->gobj == gobj) {
            return &*it;
        }
    }
    return nullptr;
}

void GameObjectStreamer::update()
{
    for (StreamedObject& sobj : streamedObjs) {
        bool active = IntersectAABoxAABox (
            sobj.gobj.getX(), sobj.gobj.getY(), sobj.width, sobj.height,
            activeX, activeY, activeW, activeH
            );
        //LogInfo("Active: %s", active ? "yes" : "no");

        if (active  &&  (sobj.flags & StreamFlagsActive) == 0) {
            streamIn(sobj);
        } else if (!active  &&  (sobj.flags & StreamFlagsActive) != 0) {
            streamOut(sobj);
        }
    }
}

void GameObjectStreamer::streamIn(StreamedObject& sobj)
{
    //LogInfo("Stream IN");

    if (sobj.flags & StreamFlagsSpriteBitmapFile) {
        workerTask.addWorkItem([&]() {
            const char* errmsg;
            timer_mstick_t s = TimerGetTickcountMs();
            Bitmap bmp = Bitmap::loadBMP (
                sobj.bmpFile.path.c_str(),
                sobj.bmpFile.ox, sobj.bmpFile.oy,
                sobj.bmpFile.w, sobj.bmpFile.h,
                &errmsg
                );
            timer_mstick_t e = TimerGetTickcountMs();
            //LogInfo("BMP loading took %ums", (uint32_t) (e-s));
            if (bmp) {
                sobj.gobj.setSprite(Sprite::createBitmap(bmp));
            } else {
                LogError("Error loading bitmap for streaming from %s: %s",
                    sobj.bmpFile.path.c_str(), errmsg);
            }
        });
    }

    sobj.flags |= StreamFlagsActive;
}

void GameObjectStreamer::streamOut(StreamedObject& sobj)
{
    //LogInfo("Stream OUT");

    if (sobj.flags & StreamFlagsSpriteBitmapFile) {
        sobj.gobj.setSprite(Sprite());
    }

    sobj.flags &= ~StreamFlagsActive;
}


}
