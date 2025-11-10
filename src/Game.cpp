#include "Game.h"


#include <algorithm>
#include <cmath>




namespace MINTGGGameEngine
{


Game::Game()
    : screen(nullptr), randGen(randDev()), drawColliders(false), lastFrameTime(-1), frameTime(1000/50)
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
    if (!screen) {
        return;
    }
    
    screen->fillScreen(Color::WHITE);

    for (const GameObject& obj : gameObjs) {
        obj.draw(*screen);
    }

    if (drawColliders) {
        for (const GameObject& obj : gameObjs) {
            obj.getWorldCollider().debugDraw(*screen, 0xF81D);
        }
    }
    
    for (const Text& text : texts) {
        if (text.isVisible()) {
            screen->drawText(text);
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


std::vector<GameObject> Game::getGameObjectsWithTag(uint64_t tag)
{
    std::vector<GameObject> res;
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
