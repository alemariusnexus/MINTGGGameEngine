#pragma once

#include "Globals.h"
#include "AudioEngine.h"
#include "GameObjectCollision.h"
#include "GameObject.h"
#include "Screen.h"
#include "Text.h"

#include <list>
#include <random>
#include <set>
#include <vector>


namespace MINTGGGameEngine
{

class Game
{
private:
    struct GOZOrderComparator
    {
        static bool operator()(const GameObject& a, const GameObject& b)
        {
            if (a.getZOrder() < b.getZOrder()) {
                return true;
            } else if (a.getZOrder() == b.getZOrder()) {
                return a < b;
            }
            return a.getZOrder() < b.getZOrder();
        }
    };
    
public:
    typedef void (*CollisionCb)(const GameObjectCollision& coll);

public:
    Game();

    void begin(Screen& screen, uint16_t fps = 50);
    
    AudioEngine& audio();
    
    uint16_t getFrameTime() const { return frameTime; }

    void setCollisionCallback(CollisionCb cb) { collisionCb = cb; }
    
    void checkCollisions(float shrink = 0.0f);

    void draw();

    void spawnObject(const GameObject& obj);
    bool despawnObject(const GameObject& obj);
    bool despawnObjects(const std::vector<GameObject>& objs);
    
    std::vector<GameObject> getGameObjectsWithTag(uint64_t tag);
    
    void addText(const Text& text);
    bool removeText(const Text& text);

    void setDrawColliders(bool drawColliders) { this->drawColliders = drawColliders; }
    
    void sleepNextFrame();

    template <typename IntT>
    IntT randInt(IntT min, IntT max) const { return std::uniform_int_distribution<IntT>(min, max)(const_cast<std::mt19937&>(randGen)); }
    template <typename IntT>
    IntT randInt(IntT max) const { return randInt<IntT>(0, max); }

    template <typename RealT>
    RealT randReal(RealT min, RealT max) const { return std::uniform_real_distribution<RealT>(min, max)(const_cast<std::mt19937&>(randGen)); }
    template <typename RealT>
    RealT randReal(RealT max) const { return randReal<RealT>(RealT(0), max); }

private:
    void onCollision(const GameObject& a, const GameObject& b, float shrink);

private:
    Screen* screen;
    std::set<GameObject, GOZOrderComparator> gameObjs;
    std::list<Text> texts;

    std::random_device randDev;
    std::mt19937 randGen;
    
    AudioEngine audioEng;

    CollisionCb collisionCb;

    bool drawColliders;
    
    uint16_t frameTime;
    long lastFrameTime;
};

}
