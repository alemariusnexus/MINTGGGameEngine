#pragma once

#include "../Globals.h"
#include "../audio/AudioEngine.h"
#include "../graphics/Screen.h"
#include "../graphics/Text.h"
#include "../input/InputEngine.h"
#include "../network/NetworkEngine.h"
#include "../physics/GameObjectCollision.h"
#include "../storage/StorageEngine.h"
#include "../util/RayCastResult.h"
#include "GameObject.h"

#include <list>
#include <random>
#include <set>
#include <vector>

#include "../util/Util.h"


namespace MINTGGGameEngine
{

/**
 * \brief The central class representing a game.
 */
class Game
{
    friend class GameObject;

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

    struct DrawStats
    {
        uint32_t timeFillUs;
        uint32_t timeObjectsUs;
        uint32_t timeCollidersUs;
        uint32_t timeRaysUs;
        uint32_t timeTextsUs;
        uint32_t timeCommitUs;
    };

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
    void begin(Screen& screen, uint16_t fps = 40);
    
    
    /// \name Engine Components
    ///@{

    /**
     * \brief Return the screen used for the game.
     *
     * @return The game screen.
     */
    Screen& getScreen();

    /**
     * \brief Return a reference to the storage engine.
     *
     * @return Storage engine reference.
     */
    StorageEngine& storage();

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

    /**
     * \brief Return a reference to the network engine.
     *
     * \return Network engine reference.
     */
    NetworkEngine& network();
    
    ///@}


    /// \name Engine Components
    ///@{

    /**
     * \brief Set the internal ID for the game application.
     *
     * This can be an arbitrary value, but should be unique to each game. It should
     * also be pretty short, as it may be used e.g. as a NVS namespace for storing
     * data on an ESP32's NVS.
     *
     * @param id The application ID.
     */
    void setApplicationID(const std::string& id);

    /**
     * \brief Return the application ID, which uniquely identifies the game.
     *
     * @return The application ID.
     * @see setApplicationID()
     */
    const std::string& getApplicationID() const;

    ///@}
    
    
    /// \name Frames
    ///@{

    /**
     * \brief Notify the game that frame processing is about to start.
     *
     * Must be called before any per-frame processing is done.
     *
     * @see endFrame()
     */
    void beginFrame();

    /**
     * \brief Notify the game that frame processing has just finished.
     *
     * Must be called after all per-frame processing is done (but before
     * any waiting time for the next frame).
     *
     * @see beginFrame()
     */
    void endFrame();
    
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
    void draw(DrawStats* stats = nullptr);

    /**
     * \brief Set the background color used for rendering.
     *
     * This color is used to fill the entire screen at the start of each frame rendering.
     * Note that if a background bitmap is set, the bitmap takes precedence and the
     * background color is not used (not even if the background bitmap only partl covers
     * the screen).
     *
     * @param color The background color.
     * @see setBackgroundBitmap()
     */
    void setBackgroundColor(const Color& color) { backgroundColor = color; backgroundBmp = Bitmap(); }

    /**
     * \brief Return the background color used for rendering.
     *
     * @return The background color.
     */
    Color getBackgroundColor() const { return backgroundColor; }

    /**
     * \brief Set a bitmap that is rendered as a background each frame.
     *
     * If a valid bitmap is passed, it is drawn as the first step on each
     * frame rendering at coordiantes (0, 0). Note that if such a background
     * bitmap is used, the background color will NOT be applied, even if the
     * bitmap only partially covers the screen.
     *
     * Using this method for setting a background is faster than using a simple
     * background GameObject, since this method will cause the step of filling
     * the screen with a solid color each frame to be skipped.
     *
     * @param bmp The background bitmap. Pass an invalid bitmap to disable
     *  the background bitmap and use a background color instead.
     * @see setBackgroundColor()
     */
    void setBackgroundBitmap(const Bitmap& bmp) { backgroundBmp = bmp; }

    /**
     * \brief Return the background bitmap.
     *
     * @return The background bitmap.
     */
    Bitmap getBackgroundBitmap() const { return backgroundBmp; }
    
    ///@}
    
    
    /// \name Game Objects
    ///@{

    /**
     * \brief Spawn the given GameObject.
     *
     * This will add the GameObject to the lists for drawing, collision checking
     * etc.
     *
     * \param obj The GameObject to spawn.
     * \return true if spawned, false otherwise.
     */
    bool spawnObject(const GameObject& obj);
    
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
     * \brief Despawn all objects that have the given tag.
     *
     * @param tag The tag to search for. Only a single tag is allowed here.
     * @return true if **any** object was successfully despawned, false otherwise.
     * @see despawnObjects()
     * @see getGameObjectsWithTag()
     */
    bool despawnObjectsWithTag(uint64_t tag);
    
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
    void drawBegin(DrawStats* stats);
    void drawFinish(DrawStats* stats);

    void onCollision(const GameObject& a, const GameObject& b, float shrink);

    void notifyGameObjectZOrderChanged(const GameObject& gobj);

private:
    std::string appID;

    Screen* screen;
    std::set<GameObject, GOZOrderComparator> gameObjs;
    std::list<Text> texts;

    std::random_device randDev;
    std::mt19937 randGen;

    AudioEngine audioEng;
    InputEngine inputEng;
    NetworkEngine networkEng;

    CollisionCb collisionCb;

    bool drawColliders;
    bool drawRayCasts;
    
    uint16_t frameTime;
    timer_mstick_t lastFrameTime;
    
    std::vector<RayCastDrawInfo> rayCastDrawInfos;
    
    Vec2 cameraOffset;

    Color backgroundColor;
    Bitmap backgroundBmp;
};

}
