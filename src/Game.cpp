#include "Game.h"


#include <algorithm>
#include <cmath>




namespace MINTGGGameEngine
{


Game::Game()
    : screen(nullptr), randGen(randDev()),
      drawColliders(false), drawRayCasts(false),
      lastFrameTime(-1), frameTime(1000/50)
{
}


void Game::begin(Screen& screen, uint16_t fps)
{
    this->screen = &screen;
    frameTime = 1000 / fps;
    lastFrameTime = millis();
}


AudioEngine& Game::audio()
{
    return audioEng;
}


InputEngine& Game::input()
{
    return inputEng;
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


void Game::draw()
{
    drawBegin();
    drawFinish();
}


void Game::drawBegin()
{
    if (!screen) {
        return;
    }
    
    Vec2 drawOffset = -cameraOffset;
    
    screen->fillScreen(Color::WHITE);

    for (const GameObject& obj : gameObjs) {
        obj.draw(*screen, drawOffset);
    }

    if (drawColliders) {
        for (const GameObject& obj : gameObjs) {
            obj.getWorldCollider().debugDraw(*screen, 0xF81D, drawOffset);
        }
    }
    
    // Always draw the infos in the list, so setDrawRayCasts() can be used to
    // selectively enable/disable it for individual ray casts.
    for (const auto& info : rayCastDrawInfos) {
        RayCastResult::drawDebugRay(*screen, info.rayStart, info.rayEnd, drawOffset);
        info.result.drawDebug(*screen, drawOffset);
    }
    rayCastDrawInfos.clear();
}

void Game::drawFinish()
{
    if (!screen) {
        return;
    }
    
    Vec2 drawOffset = -cameraOffset;
    
    for (const Text& text : texts) {
        if (text.isVisible()) {
            if (text.isWorldSpace()) {
                screen->drawText(text, (int16_t) (drawOffset.x()+0.5f), (int16_t) (drawOffset.y()+0.5f));
            } else {
                screen->drawText(text);
            }
        }
    }
    
    screen->commit();
}


void Game::spawnObject(const GameObject& obj)
{
    gameObjs.insert(obj);
}


bool Game::despawnObject(const GameObject& obj)
{
    return gameObjs.erase(obj) != 0;
    /*auto it = std::find(gameObjs.begin(), gameObjs.end(), obj);
    if (it == gameObjs.end()) {
        return false;
    }
    gameObjs.erase(it);
    return true;*/
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
        size_t numIntersections = go.getWorldCollider()
                .castRay(res.getHits(), start, direction, length, go);
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


void Game::sleepNextFrame()
{
    long now = millis();
    
    uint32_t delayTimeMs = 0;
    
    if (lastFrameTime < 0) {
        delayTimeMs = frameTime;
    } else if (now-lastFrameTime < frameTime) {
        delayTimeMs = frameTime - (now-lastFrameTime);
    }
    
    vTaskDelay(delayTimeMs / portTICK_PERIOD_MS);
    // delay(delayTimeMs);
    
    lastFrameTime = millis();
}


void Game::onCollision(const GameObject& a, const GameObject& b, float shrink)
{
    if (collisionCb) {
        collisionCb(GameObjectCollision(a, b));
    }
}


}
