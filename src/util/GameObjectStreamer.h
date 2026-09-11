#pragma once

#include "../Globals.h"

#include "../core/GameObject.h"
#include "WorkerTask.h"

#include <functional>
#include <vector>


namespace MINTGGGameEngine
{


/**
 * \brief Helper for streaming parts of GameObjects in and out based on what area of a game is currently active.
 *
 * Each object of this class manages a list of registered GameObjects. A GameObject can be registered with a fixed
 * width and height (registerGameObject()). You can then set the active area of the streamer object to an arbitrary
 * axis-aligned rectangle in the game world (setActiveArea()). All registered GameObjects that lie within this active
 * area (including overlapping) are considered active, while all other registered GameObjects are considered inactive.
 * The streamer automatically updates the active state whenever the active area changes, in the following way:
 *
 *  - All GameObjects start out as inactive when first registered.
 *  - Whenever an inactive GameObject is detected as lying inside the active area, it is *streamed in* and then becomes
 *    active.
 *  - Whenever an active GameObject is detected as lying outside the active area, it is *streamed out* and then becomes
 *    inactive.
 *
 * Different actions can be taken when an object is streamed in. Currently, the following are supported:
 *
 *  - enableSpriteBitmapFile(): The sprite of the GameObject is automatically loaded from a given file path when the
 *    object is streamed in, and is replaced by an empty sprite when it is streamed out. Sprite loading happens
 *    asynchronously in a separate task by default, so it does not block game execution.
 *  - enableCustomCallback(): A callback is called when the object is streamed in or out, allowing the user to run
 *    arbitrary code for streaming in/out.
 *
 * Multiple of these actions can be active for a single GameObject at the same time.
 *
 * This class is useful for creating game maps that are too large to fit into memory all at once, or that have too many
 * GameObjects with complex behaviors to execute all at once in real time. Instead, only the game area currently visible
 * on screen is set as a streamer's active area, so only part of the map is active at any time. Some specific use cases:
 *
 *  1. For a scrolling game, a large game background image (multiple times the screen size) can be implemented even
 *     if it doesn't fully fit into memory. Instead, the background image is separated into fixed-size chunks (either
 *     individual images, or a single image with fixed offsets), and each chunk is given its own GameObject registered
 *     to the streamer. The background sprite for the chunk GameObject is then streamed in or out using
 *     enableSpriteBitmapFile().
 *     Note that in this case, the active area should usually be *larger* than the screen size (buffer are), since image
 *     loading happens asynchronously and can take a few frames to complete, and the player might otherwise see missing
 *     chunks while loading is still in progress.
 *  2. For a scrolling game, GameObjects with computationally expensive behavior (e.g. movement, pathfinding, collision
 *     detection) can be registered to a streamer, and enableCustomCallback() can be used to enable this behavior only
 *     when the GameObject is currently visible on screen (or inside a larger "AI active" area).
 */
class GameObjectStreamer
{
public:
    typedef std::function<void (const GameObject&, bool)> StreamCbFunc;

private:
    enum StreamFlags
    {
        StreamFlagsSpriteBitmapFile = 0x00000001,
        StreamFlagsCustomCallback = 0x00008000,

        StreamFlagsSpriteBitmapMaskOnly = 0x00010000,
        StreamFlagsCustomCallbackInBlocking = 0x00020000,
        StreamFlagsCustomCallbackOutBlocking = 0x00040000,

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

        struct {
            StreamCbFunc streamCb;
        } customCb;
    };

public:
    GameObjectStreamer();
    ~GameObjectStreamer();
    
    
    /// \name Registering GameObjects
    ///@{

    /**
     * \brief Register a GameObject with a fixed width and height.
     *
     * No action on streaming is enabled by default. You should call the corresponding methods like
     * enableSpriteBitmapFile() or enableCustomCallback() after registering the GameObject.
     *
     * \param gobj The GameObject to register.
     * \param width Width of the GameObject in pixels.
     * \param height Height of the GameObject in pixels.
     */
    void registerGameObject(const GameObject& gobj, float width, float height);

    /**
     * \brief Unregister a previously registered GameObject.
     *
     * \param gobj The GameObject to unregister.
     * \return true if unregistered successfully, false otherwise (e.g. if it wasn't registered before).
     */
    bool unregisterGameObject(const GameObject& gobj);
    
    ///@}

    
    /// \name Streaming Actions
    ///@{

    /**
     * \brief Enable loading a GameObject's sprite from a bitmap file when active, and unloading when inactive.
     * 
     * Once the GameObject is streamed in, its sprite will be loaded from the given path, and with the given pixel
     * offset, width and height in the image file. Image loading happens asynchronously in a separate worker task, so
     * it doesn't block game execution. This also means that the sprite will not usually be loaded in the same frame
     * that the GameObject becomes active, but may be **delayed for several frames**.
     * 
     * Note that the pixel offset, width and height can be used to load different parts of a single image file for
     * different GameObjects, allowing a single image file to be streamed in multiple chunks.
     * 
     * \param gobj The GameObject to load a sprite for.
     * \param path Path to the sprite's image file.
     * \param ox X offset within the image file where the sprite starts, in pixels.
     * \param oy Y offset within the image file where the sprite starts, in pixels.
     * \param w Maximum width of the sprite to load from the image file, in pixels.
     * \param h Maximum height of the sprite to load from the image file, in pixels.
     * \param maskOnly false to load the image as a regular sprite, i.e. as a color Bitmap. true to load the colors of
     *      the image file into the Bitmap mask only (i.e. an invisible Bitmap).
     * \return true if successful, false otherwise (e.g. if the GameObject wasn't registered).
     * \see disableSpriteBitmapFile()
     */
    bool enableSpriteBitmapFile (
        const GameObject& gobj,
        const char* path,
        uint16_t ox = 0, uint16_t oy = 0,
        uint16_t w = UINT16_MAX, uint16_t h = UINT16_MAX,
        bool maskOnly = false
        );

    /**
     * \brief Disable loading a GameObject's sprite from a bitmap file when active.
     *
     * \param gobj The GameObject to disable sprite loading for.
     * \return true if successful, false otherwise (e.g. if the GameObject wasn't registered).
     */
    bool disableSpriteBitmapFile(const GameObject& gobj);


    /**
     * \brief Enable calling a user-specified callback function whenever a GameObject is streamed in or out.
     *
     * The same callback function is called for streaming in and streaming out, but with the second parameter to the
     * callback function set to true for streaming in, and false for streaming out.
     *
     * \param gobj The GameObject to enable calling a user-specific callback for.
     * \param streamCb The callback function. Called with the GameObject in question as first argument, and second
     *      argument as true when streaming in or false when streaming out.
     * \param inBlocking If true, the callback will be called from the game's main task when streaming in. If false,
     *      it will be called asynchonously in a worker task instead.
     * \param outBlocking If true, the callback will be called from the game's main task when streaming out. If false,
     *      it will be called asynchonously in a worker task instead.
     * \return true if successful, false otherwise (e.g. if the GameObject wasn't registered).
     */
    bool enableCustomCallback (
        const GameObject& gobj,
        const StreamCbFunc& streamCb,
        bool inBlocking = true, bool outBlocking = true
        );

    /**
     * \brief Disable calling a user-specified callback function for a GameObject when streamed in or out.
     *
     * \param gobj The GameObject to disable callback calling for.
     * \return true if successful, false otherwise (e.g. if the GameObject wasn't registered).
     */
    bool disableCustomCallback(const GameObject& gobj);
    
    ///@}


    /// \name Active Area Management
    ///@{

    /**
     * \brief Update the area in which this streamer considers a GameObject to be active.
     *
     * The active area is always an axis-aligned rectangle in the game world. All registered GameObjects that lie inside
     * or overlap this active area are considered active (to be streamed in), and all other registered GameObjects are
     * considered inactive (to be streamed out).
     *
     * This function should be called by the user whenever the active area changes, e.g. when the screen scrolls.
     *
     * \param x X coordinate of the active area's top-left corner, in pixels.
     * \param y Y coordinate of the active area's top-left corner, in pixels.
     * \param w Width of the active area, in pixels.
     * \param h Height of the active area, in pixels.
     */
    void setActiveArea(float x, float y, float w, float h);

    ///@}


    /// \name Miscellaneous
    ///@{

    size_t getActiveCount() const;
    size_t getMemoryUsage() const;

    ///@}

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
