/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "GameObjectStreamer.h"

#include "Log.h"
#include "MathUtils.h"
#include "Util.h"


LOG_USE_TAG("GameObjectStreamer")


namespace MINTGGGameEngine
{


// TODO: Implement proper synchronization with workerTask (e.g. what if GameObject is unregistered in the middle of
//  streaming in/out?)


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
    float scaleX, float scaleY,
    uint16_t ox, uint16_t oy,
    uint16_t w, uint16_t h,
    bool alphaOnly
) {
    StreamedObject* sobj = findObject(gobj);
    if (!sobj) {
        return false;
    }
    sobj->flags |= StreamFlagsSpriteBitmapFile;
    if (alphaOnly) {
        sobj->flags |= StreamFlagsSpriteBitmapAlphaOnly;
    } else {
        sobj->flags &= ~StreamFlagsSpriteBitmapAlphaOnly;
    }
    sobj->bmpFile.path = path;
    sobj->bmpFile.ox = ox;
    sobj->bmpFile.oy = oy;
    sobj->bmpFile.w = w;
    sobj->bmpFile.h = h;
    sobj->bmpFile.scaleX = scaleX;
    sobj->bmpFile.scaleY = scaleY;
    return true;
}

bool GameObjectStreamer::disableSpriteBitmapFile(const GameObject& gobj)
{
    StreamedObject* sobj = findObject(gobj);
    if (!sobj) {
        return false;
    }
    sobj->flags &= ~(StreamFlagsSpriteBitmapFile | StreamFlagsSpriteBitmapAlphaOnly);
    sobj->bmpFile.path.clear();
    return true;
}

bool GameObjectStreamer::enableCustomCallback (
        const GameObject& gobj,
        const StreamCbFunc& streamCb,
        bool inBlocking, bool outBlocking
) {
    StreamedObject* sobj = findObject(gobj);
    if (!sobj) {
        return false;
    }
    sobj->flags |= StreamFlagsCustomCallback;
    if (inBlocking) {
        sobj->flags |= StreamFlagsCustomCallbackInBlocking;
    } else {
        sobj->flags &= ~StreamFlagsCustomCallbackInBlocking;
    }
    if (outBlocking) {
        sobj->flags |= StreamFlagsCustomCallbackOutBlocking;
    } else {
        sobj->flags &= ~StreamFlagsCustomCallbackOutBlocking;
    }
    sobj->customCb.streamCb = streamCb;
    return true;
}

bool GameObjectStreamer::disableCustomCallback(const GameObject& gobj)
{
    StreamedObject* sobj = findObject(gobj);
    if (!sobj) {
        return false;
    }
    sobj->flags &= ~(StreamFlagsCustomCallback | StreamFlagsCustomCallbackInBlocking
        | StreamFlagsCustomCallbackOutBlocking);
    sobj->customCb.streamCb = {};
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
        memUsage += sizeof(sobj);
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
            //timer_mstick_t s = TimerGetTickcountMs();
            Bitmap bmp;
            if (sobj.flags & StreamFlagsSpriteBitmapAlphaOnly) {
                bmp = Bitmap::loadBMPAlphaOnly (
                    sobj.bmpFile.path.c_str(),
                    sobj.bmpFile.scaleX, sobj.bmpFile.scaleY,
                    sobj.bmpFile.ox, sobj.bmpFile.oy,
                    sobj.bmpFile.w, sobj.bmpFile.h,
                    &errmsg
                    );
            } else {
                bmp = Bitmap::loadBMP (
                    sobj.bmpFile.path.c_str(),
                    sobj.bmpFile.scaleX, sobj.bmpFile.scaleY,
                    sobj.bmpFile.ox, sobj.bmpFile.oy,
                    sobj.bmpFile.w, sobj.bmpFile.h,
                    &errmsg
                    );
            }
            //timer_mstick_t e = TimerGetTickcountMs();
            //LogInfo("BMP loading took %ums", (uint32_t) (e-s));
            if (bmp) {
                sobj.gobj.setSprite(Sprite::createBitmap(bmp));
            } else {
                LogError("Error loading bitmap for streaming from %s: %s",
                    sobj.bmpFile.path.c_str(), errmsg);
            }
        });
    }

    if (sobj.flags & StreamFlagsCustomCallback) {
        if (sobj.flags & StreamFlagsCustomCallbackInBlocking) {
            sobj.customCb.streamCb(sobj.gobj, true);
        } else {
            workerTask.addWorkItem([&]() {
                sobj.customCb.streamCb(sobj.gobj, true);
            });
        }
    }

    sobj.flags |= StreamFlagsActive;
}

void GameObjectStreamer::streamOut(StreamedObject& sobj)
{
    //LogInfo("Stream OUT");

    if (sobj.flags & StreamFlagsCustomCallback) {
        if (sobj.flags & StreamFlagsCustomCallbackOutBlocking) {
            sobj.customCb.streamCb(sobj.gobj, false);
        } else {
            workerTask.addWorkItem([&]() {
                sobj.customCb.streamCb(sobj.gobj, false);
            });
        }
    }

    if (sobj.flags & StreamFlagsSpriteBitmapFile) {
        sobj.gobj.setSprite(Sprite());
    }

    sobj.flags &= ~StreamFlagsActive;
}


}
