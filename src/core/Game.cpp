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

#include "Game.h"

#include <algorithm>
#include <cmath>

#include "../util/Log.h"
#include "../util/Util.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <esp_task_wdt.h>
#endif


LOG_USE_TAG("Game")




namespace MINTGGGameEngine
{


Game::Game()
    : quitRequested(false), screen(nullptr), randGen(randDev()),
      collisionCb(nullptr),
      drawColliders(false), drawRayCasts(false),
      frameTime(1000/40), lastFrameTime(0),
      screenOffsetX(0), screenOffsetY(0), drawScaleX(1.0f), drawScaleY(1.0f),
      backgroundColor(Color(255, 255, 255)),
      cachedScaledBackgroundBmpX(0), cachedScaledBackgroundBmpY(0),
      backgroundSkipped(false)
{
}


void Game::begin(Screen& screen, uint16_t fps)
{
    TimerInit();

    this->screen = &screen;
    frameTime = 1000 / fps;
    lastFrameTime = TimerGetTickcountMs();
}


Screen& Game::getScreen()
{
    return *screen;
}


StorageEngine& Game::storage()
{
    return StorageEngine::getInstance();
}


AudioEngine& Game::audio()
{
    return audioEng;
}


InputEngine& Game::input()
{
    return inputEng;
}


NetworkEngine& Game::network()
{
    return networkEng;
}


void Game::setApplicationID(const std::string& id)
{
    appID = id;
}


const std::string& Game::getApplicationID() const
{
    return appID;
}


void Game::setApplicationName(const std::string& name)
{
    appName = name;
}


const std::string& Game::getApplicationName() const
{
    return appName;
}


void Game::quit()
{
    quitRequested = true;
}


bool Game::isQuitRequested() const
{
    return quitRequested;
}


void Game::beginFrame()
{
    inputEng.notifyBeginFrame();
}


void Game::endFrame()
{
    inputEng.notifyEndFrame();
}


void Game::checkCollisions(float shrink)
{
    auto gameObjsLocal = gameObjs;
    for (auto firstIt = gameObjsLocal.begin() ; firstIt != gameObjsLocal.end() ; ++firstIt) {
        auto secondIt = firstIt;
        for (++secondIt ; secondIt != gameObjsLocal.end() ; ++secondIt) {
            if (firstIt->collides(*secondIt, shrink)) {
                onCollision(*firstIt, *secondIt, shrink);
            }
        }
    }
}


void Game::draw(DrawStats* stats)
{
    drawBegin(stats);
    drawFinish(stats);
}


void Game::drawBegin(DrawStats* stats)
{
    if (!screen) {
        return;
    }

    const Vec2 worldOffset = -cameraOffset;
    const Vec2 combinedOffset (
        worldOffset.x()*drawScaleX + static_cast<float>(screenOffsetX),
        worldOffset.y()*drawScaleY + static_cast<float>(screenOffsetY)
        );

    timer_ustick_t timeFill = TimerGetTickcountUs();
    if (!backgroundSkipped) {
        if (backgroundBmp) {
            screen->drawBitmap (
                screenOffsetX, screenOffsetY,
                cachedScaledBackgroundBmp,
                FlipDir::None,
                Rotation::Rot0,
                1, 1
                );
        } else {
            screen->fill(backgroundColor, false);
        }
    }

    timer_ustick_t timeObjects = TimerGetTickcountUs();
    for (const GameObject& obj : gameObjs) {
        obj.draw(*screen, combinedOffset, drawScaleX, drawScaleY);
    }

    timer_ustick_t timeColliders = TimerGetTickcountUs();
    if (drawColliders) {
        for (const GameObject& obj : gameObjs) {
            obj.getWorldCollider().debugDraw(*screen, Color::fromRGB565(0xF81D), combinedOffset,
                drawScaleX, drawScaleY);
        }
    }
    
    // Always draw the infos in the list, so setDrawRayCasts() can be used to
    // selectively enable/disable it for individual ray casts.
    timer_ustick_t timeRays = TimerGetTickcountUs();
    for (const auto& info : rayCastDrawInfos) {
        RayCastResult::drawDebugRay(*screen, info.rayStart, info.rayEnd, combinedOffset, drawScaleX, drawScaleY);
        info.result.drawDebug(*screen, combinedOffset, drawScaleX, drawScaleY);
    }
    rayCastDrawInfos.clear();

    timer_ustick_t timeEnd = TimerGetTickcountUs();

    if (stats) {
        stats->timeFillUs = (uint32_t) (timeObjects-timeFill);
        stats->timeObjectsUs = (uint32_t) (timeColliders-timeObjects);
        stats->timeCollidersUs = (uint32_t) (timeRays-timeColliders);
        stats->timeRaysUs = (uint32_t) (timeEnd-timeRays);
    }
}

void Game::drawFinish(DrawStats* stats)
{
    if (!screen) {
        return;
    }
    
    const Vec2 worldOffset = -cameraOffset;
    const Vec2 combinedOffset (
        worldOffset.x()*drawScaleX + static_cast<float>(screenOffsetX),
        worldOffset.y()*drawScaleY + static_cast<float>(screenOffsetY)
        );

    const auto uDrawScaleX = static_cast<uint16_t>(drawScaleX);
    const auto uDrawScaleY = static_cast<uint16_t>(drawScaleY);

    timer_ustick_t timeTexts = TimerGetTickcountUs();
    for (const Text& text : texts) {
        if (text.isVisible()) {
            if (uDrawScaleX == 1  &&  uDrawScaleY == 1) {
                if (text.isWorldSpace()) {
                    screen->drawText (
                        text,
                        static_cast<int16_t>(std::round(combinedOffset.x())),
                        static_cast<int16_t>(std::round(combinedOffset.y()))
                        );
                } else {
                    screen->drawText(text, screenOffsetX, screenOffsetY);
                }
            } else {
                const auto origX = text.getX();
                const auto origY = text.getY();
                const auto origScaleX = text.getScaleX();
                const auto origScaleY = text.getScaleY();
                const auto origExplosion = text.getExplosion();

                Text& ncText = const_cast<Text&>(text);
                ncText.setPosition(origX*uDrawScaleX, origY*uDrawScaleY);
                ncText.setScale(origScaleX*uDrawScaleX, origScaleY*uDrawScaleY);
                ncText.setExplosion(origExplosion*uDrawScaleX);

                if (text.isWorldSpace()) {
                    screen->drawText (
                        text,
                        static_cast<int16_t>(std::round(combinedOffset.x())),
                        static_cast<int16_t>(std::round(combinedOffset.y()))
                        );
                } else {
                    screen->drawText(text, screenOffsetX, screenOffsetY);
                }

                ncText.setPosition(origX, origY);
                ncText.setScale(origScaleX, origScaleY);
                ncText.setExplosion(origExplosion);
            }
        }
    }

    timer_ustick_t timeCommit = TimerGetTickcountUs();
    screen->commit();

    timer_ustick_t timeEnd = TimerGetTickcountUs();

    if (stats) {
        stats->timeTextsUs = (uint32_t) (timeCommit-timeTexts);
        stats->timeCommitUs = (uint32_t) (timeEnd-timeCommit);
    }
}


void Game::setBackgroundBitmap(const Bitmap& bmp)
{
    if (    bmp == backgroundBmp
        &&  cachedScaledBackgroundBmpX == drawScaleX
        &&  cachedScaledBackgroundBmpY == drawScaleY
    ) {
        return;
    }
    backgroundBmp = bmp;
    cachedScaledBackgroundBmp = {}; // Do this first to free up memory
    cachedScaledBackgroundBmp = bmp.scaled(drawScaleX, drawScaleY);
    cachedScaledBackgroundBmpX = drawScaleX;
    cachedScaledBackgroundBmpY = drawScaleY;
}


bool Game::spawnObject(const GameObject& obj)
{
    if (obj.isSpawned()) {
        return false;
    }
    gameObjs.insert(obj);
    const_cast<GameObject&>(obj).updateScaleCache(drawScaleX, drawScaleY);
    obj.notifySpawned(this);
    return true;
}


bool Game::despawnObject(const GameObject& obj)
{
    if (gameObjs.erase(obj) != 0) {
        obj.notifyDespawned(this);
        return true;
    }
    return false;
}


bool Game::despawnObjects(const std::vector<GameObject>& objs)
{
    bool anyDespawned = false;
    for (const GameObject& obj : objs) {
        if (despawnObject(obj)) {
            anyDespawned = true;
        }
    }
    return anyDespawned;
}


bool Game::despawnObjectsWithTag(uint64_t tag)
{
    return despawnObjects(getGameObjectsWithTag(tag));
}


std::vector<GameObject> Game::getGameObjects() const
{
    std::vector<GameObject> res;
    res.reserve(gameObjs.size());
    for (const GameObject& go : gameObjs) {
        res.push_back(go);
    }
    return res;
}


std::vector<GameObject> Game::getGameObjectsWithTag(uint64_t tag) const
{
    std::vector<GameObject> res;
    res.reserve(gameObjs.size() > 10 ? 10 : gameObjs.size());
    for (const GameObject& go : gameObjs) {
        if (go.hasTag(tag)) {
            res.push_back(go);
        }
    }
    return res;
}


void Game::addText(const Text& text)
{
    texts.push_back(text);
}


bool Game::removeText(const Text& text)
{
    auto it = std::find(texts.begin(), texts.end(), text);
    if (it == texts.end()) {
        return false;
    }
    texts.erase(it);
    return true;
}


RayCastResult Game::castRay (
        const Vec2& start, const Vec2& end,
        const std::vector<GameObject>& gameObjects,
        bool sort
) {
    Vec2 startToEnd = end-start;
    float length;
    Vec2 direction = startToEnd.normalized(&length);
    
    //std::vector<RayCastHit> hits;
    //hits.reserve(8);
    RayCastResult res;
    res.getHits().reserve(10);
    
    for (const GameObject& go : gameObjects) {
        go.getWorldCollider().castRay(res.getHits(), start, direction, length, go);
    }
    
    if (sort) {
        std::sort(res.getHits().begin(), res.getHits().end(), [](const auto& a, const auto& b) {
            return a.rayOffset < b.rayOffset;
        });
    }
    
    if (drawRayCasts) {
        rayCastDrawInfos.emplace_back(start, end, res);
    }
    
    return res;
}


RayCastResult Game::castRay (
        const Vec2& start, const Vec2& end,
        bool sort
) {
    return castRay(start, end, getGameObjects(), sort);
}


void Game::setDrawScale(float scaleX, float scaleY)
{
    // Right now, only integer up-scaling is supported
    scaleX = std::max(std::round(scaleX), 1.0f);
    scaleY = std::max(std::round(scaleY), 1.0f);

    if (drawScaleX == scaleX  &&  drawScaleY == scaleY) {
        return;
    }

    drawScaleX = scaleX;
    drawScaleY = scaleY;

    setBackgroundBitmap(getBackgroundBitmap());
    for (const GameObject& obj : gameObjs) {
        const_cast<GameObject&>(obj).updateScaleCache(scaleX, scaleY);
    }
}


void Game::sleepNextFrame()
{
    long now = TimerGetTickcountMs();
    
    uint32_t delayTimeMs = 0;
    
    if (lastFrameTime == 0) {
        delayTimeMs = frameTime;
    } else if (now-lastFrameTime < frameTime) {
        delayTimeMs = frameTime - (now-lastFrameTime);
    }

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    TickType_t delayTicks = (delayTimeMs + portTICK_PERIOD_MS-1) / portTICK_PERIOD_MS;
    if (delayTicks == 0) {
        // Needed to yield to lower-priority tasks like the FreeRTOS idle task once in a while. This is a bit of a
        // waste of time, but seems to be the best solution for now. taskYIELD() is NOT enough, since it doesn't
        // yield to lower-priority tasks.
        // TODO: Maybe find a better way.
        delayTicks = 1;
    }
    vTaskDelay(delayTicks);
#else
    DelayTaskMs(delayTimeMs);
#endif
    
    lastFrameTime = TimerGetTickcountMs();
}


void Game::onCollision(const GameObject& a, const GameObject& b, float shrink)
{
    if (collisionCb) {
        collisionCb(GameObjectCollision(a, b));
    }
}


void Game::notifyGameObjectZOrderChanged(const GameObject& gobj)
{
    if (despawnObject(gobj)) {
        spawnObject(gobj);
    }
}


}
