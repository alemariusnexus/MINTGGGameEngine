#pragma once

#include "Globals.h"
#include "AudioEngine.h"
#include "GameObjectCollision.h"
#include "GameObject.h"
#include "InputEngine.h"
#include "RayCastResult.h"
#include "Screen.h"
#include "Text.h"

#include <list>
#include <random>
#include <set>
#include <vector>


namespace MINTGGGameEngine
{

/**
 * \brief The central class representing a game.
 */
class Game
{
private:
    struct GOZOrderComparator
    {
        bool operator()(const GameObject& a, const GameObject& b) const
        {
            if (a.getZOrder() < b.getZOrder()) {
                return true;
            } else if (a.getZOrder() == b.getZOrder()) {
                return a < b;
            }
            return a.getZOrder() < b.getZOrder();
        }
    };
    
    struct RayCastDrawInfo
    {
        Vec2 rayStart;
        Vec2 rayEnd;
        RayCastResult result;
    };
    
public:
    /**
     * \brief Callback function for when a collision occurs.
     * 
     * \param coll The collision info object.
     */
    typedef void (*CollisionCb)(const GameObjectCollision& coll);

public:
    /**
     * \brief Create a new game.
     *
     * Note that the actual initialization must be done later by calling
     * begin().
     */
    Game();

    /**
     * \brief Initializes the game.
     *
     * \param screen The screen to draw on.
     * \param fps The targeted value for frames per second. This influences the
     *      wait time in sleepNextFrame().
     */
    void begin(Screen& screen, uint16_t fps = 50);
    
    
    /// \name Engine Components
    ///@{
    
    /**
     * \brief Return a reference to the audio engine.
     *
     * \return Audio engine reference.
     */
    AudioEngine& audio();
    
    /**
     * \brief Return a reference to the input engine.
     *
     * \return Input engine reference.
     */
    InputEngine& input();
    
    ///@}
    
    
    /// \name Frames
    ///@{
    
    /**
     * \brief Return the targeted time for a single frame.
     *
     * This value is calculated from the targeted FPS passed to begin().
     *
     * \return The frame time, in milliseconds.
     */
    uint16_t getFrameTime() const { return frameTime; }
    
    /**
     * \brief Delay program execution until the next frame.
     *
     * The delay depends on the target FPS value passed to begin().
     */
    void sleepNextFrame();
    
    ///@}
    
    
    /// \name Collision Detection
    ///@{

    /**
     * \brief Set the function to be called when a collision occurs.
     *
     * \param cb The collision callback function.
     */
    void setCollisionCallback(CollisionCb cb) { collisionCb = cb; }
    
    /**
     * \brief Enable or disable debug drawing of colliders.
     *
     * \param drawColliders true to draw colliders, false to disable drawing.
     */
    void setDrawColliders(bool drawColliders) { this->drawColliders = drawColliders; }
    
    /**
     * \brief Enable or disable debug drawing of ray casts and results.
     *
     * \param drawRayCasts true to draw ray casts and results, false to disable.
     */
    void setDrawRayCasts(bool drawRayCasts) { this->drawRayCasts = drawRayCasts; }
    
    /**
     * \brief Run collision detection on all objects.
     *
     * This will check all pairs of GameObjects for collision with each other.
     * For each collision, the callbck set by setCollisionCallback() will be
     * called.
     *
     * \param shrink The amount to shrink each collider when checking for
     *      collision. Can be useful to avoid corner cases when two colliders
     *      are touching exactly on an edge. See Collider class for more info.
     */
    void checkCollisions(float shrink = 0.0f);
    
    ///@}
    
    
    /// \name Drawing
    ///@{

    /**
     * \brief Draw the entire scene on the screen after clearing it.
     *
     * This will first clear the screen, then draw all GameObject and Text
     * instances that are visible. It can also optionally draw collider outlines
     * and ray casts for debugging purposes (see setDrawColliders() and
     * setDrawRayCasts()).
     */
    void draw();
    
    ///@}
    
    
    /// \name Game Objects
    ///@{

    /**
     * \brief Spawn the given GameObject.
     *
     * This will add the GameObject to the lists for drawing, collision checking
     * etc.
     * A GameObject should only be spawned once. Spawning it multiple times
     * (whout despawning in-between) results in undefined behavior.
     *
     * \param obj The GameObject to spawn.
     */
    void spawnObject(const GameObject& obj);
    
    /**
     * \brief Despawn the given GameObject.
     *
     * This will remove the GameObject from the lists for drawing, collision
     * checking etc.
     * Spawning a GameObject that was despawned using this method is allowed.
     *
     * \brief obj The GameObject to spawn.
     * \return true if despawned, false otherwise (e.g. if the GameObject wasn't
     *      previously spawned).
     */
    bool despawnObject(const GameObject& obj);
    
    /**
     * \brief Despawn a list of objects.
     *
     * \return true if **any** of the objects in the list was successfully
     *      despawned, false otherwise.
     * \see despawnObject()
     */
    bool despawnObjects(const std::vector<GameObject>& objs);
    
    /**
     * \brief Get a list of all GameObjects.
     *
     * Only spawned objects will be considered.
     *
     * \return List of spawned GameObjects.
     */
    std::vector<GameObject> getGameObjects() const;
    
    /**
     * \brief Get a list with all GameObjects that have the given tag.
     *
     * Only spawned objects will be considered.
     * 
     * \param tag The tag to search for. Only a single tag is allowed here.
     * \param List of GameObjects with the tag.
     * \return List of spawned GameObjects with the tag.
     */
    std::vector<GameObject> getGameObjectsWithTag(uint64_t tag) const;
    
    ///@}
    
    
    /// \name Text
    ///@{
    
    /**
     * \brief Add the given Text object to the scene.
     *
     * \param text The Text object to add.
     */
    void addText(const Text& text);
    
    /**
     * \brief Remove the given Text object from the scene.
     *
     * \param text The Text object to remove.
     * \param true if removed, false otherwise (e.g. if it wasn't added before).
     */
    bool removeText(const Text& text);
    
    ///@}
    
    
    /// \name Ray Casting
    ///@{
    
    /**
     * \brief Cast a ray against a set of GameObjects, calculating intersections
     *      between them.
     *
     * The ray is taken as a line segment, with definite start and end points,
     * i.e. it does NOT extend indefinitely in either direction.
     *
     * The ray is checked against the GameObjects' colliders, not their sprites.
     *
     * \param start Start point of the ray.
     * \param end End point of the ray.
     * \param gameObjects GameObjects against which to check intersection.
     * \param sort true to sort the resulting hit points from ray start to ray
     *      end. If false, the order is undefined.
     * \return The result, containing all intersection points.
     */
    RayCastResult castRay (
            const Vec2& start, const Vec2& end,
            const std::vector<GameObject>& gameObjects,
            bool sort = true
            );
    
    /**
     * \brief Cast a ray against a set of GameObjects, calculating intersections
     *      between them.
     *
     * See castRay(const Vec2&, const Vec2&, const std::vector<GameObject>&, bool)
     * for details. This method checks the ray against all spawned GameObjects
     * (which might be slow!)
     */
    RayCastResult castRay (
            const Vec2& start, const Vec2& end,
            bool sort = true
            );
    
    ///@}
    
    
    /// \name Camera & Scrolling
    ///@{
    
    /**
     * \brief Set the positional offset of the camera through which the world is
     *      viewed.
     *
     * This can be used to change the part of the world that's visible on
     * screen. It is useful e.g. for implementing scrolling games, or games
     * where the camera follows the player.
     *
     * \param offset The camera offset.
     * \see scroll(const Vec2&)
     */
    void setCameraOffset(const Vec2& offset) { cameraOffset = offset; }
    
    /**
     * \brief Set the positional offset of the camera through which the world is
     *      viewed.
     *
     * \see setCameraOffset(const Vec2&)
     */
    void setCameraOffset(float ox, float oy) { setCameraOffset(Vec2(ox, oy)); }
    
    /**
     * \brief Get the positional offset of the camera through which the world is
     *      viewed.
     * 
     * \return The camera offset.
     * \see setCameraOffset(const Vec2&)
     */
    Vec2 getCameraOffset() const { return cameraOffset; }
    
    /**
     * \brief Scroll the world (actually the camera) by the given amount.
     *
     * This simply uses setCameraOffset(const Vec2&).
     *
     * \param delta The scroll delta.
     */
    void scroll(const Vec2& delta) { setCameraOffset(getCameraOffset() + delta); }
    
    /**
     * \brief Scroll the world (actually the camera) by the given amount.
     *
     * This simply uses setCameraOffset(const Vec2&).
     *
     * \param dx The scroll delta in x direction.
     * \param dy The scroll delta in y direction.
     */
    void scroll(float dx, float dy) { scroll(Vec2(dx, dy)); }
    
    ///@}
    
    
    /// \name Randomness
    ///@{

    /**
     * \brief Return a random integer between min and max (both inclusive).
     *
     * \param min The minimum value.
     * \param max The maximum value.
     * \return A random integer in the range [min, max].
     */
    template <typename IntT>
    IntT randInt(IntT min, IntT max) const { return std::uniform_int_distribution<IntT>(min, max)(const_cast<std::mt19937&>(randGen)); }
    
    /**
     * \brief Return a random integer between 0 and max (both inclusive).
     *
     * \param max The maximum value.
     * \return A random integer in the range [0, max].
     */
    template <typename IntT>
    IntT randInt(IntT max) const { return randInt<IntT>(0, max); }

    /**
     * \brief Return a random real number between min (inclusive) and max
     *      (exclusive).
     *
     * \param min The minimum value.
     * \param max The maximum value.
     * \return A random integer in the range [min, max).
     */
    template <typename RealT>
    RealT randReal(RealT min, RealT max) const { return std::uniform_real_distribution<RealT>(min, max)(const_cast<std::mt19937&>(randGen)); }
    
    /**
     * \brief Return a random real number between 0 (inclusive) and max
     *      (exclusive).
     *
     * \param max The maximum value.
     * \return A random integer in the range [0, max).
     */
    template <typename RealT>
    RealT randReal(RealT max = RealT(1.0)) const { return randReal<RealT>(RealT(0), max); }
    
    ///@}

private:
    void drawBegin();
    void drawFinish();

    void onCollision(const GameObject& a, const GameObject& b, float shrink);

private:
    Screen* screen;
    std::set<GameObject, GOZOrderComparator> gameObjs;
    std::list<Text> texts;

    std::random_device randDev;
    std::mt19937 randGen;
    
    AudioEngine audioEng;
    InputEngine inputEng;

    CollisionCb collisionCb;

    bool drawColliders;
    bool drawRayCasts;
    
    uint16_t frameTime;
    long lastFrameTime;
    
    std::vector<RayCastDrawInfo> rayCastDrawInfos;
    
    Vec2 cameraOffset;
};

}
