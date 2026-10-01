#pragma once

#include "../Globals.h"

#include "Engine.h"
#include "Game.h"

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <QApplication>
#endif


namespace MINTGGGameEngine
{


/**
 * \brief Class that encapsulates the typical lifecycle of a game.
 *
 * This class is there to simplify game code that uses the engine in a "typical"
 * way. It simplifies startup and per-frame logic while keeping the most important
 * aspects configurable.
 *
 * If more advanced control over the engine is desired, a custom subclass can be
 * created.
 */
class DefaultEngine : public Engine
{
public:
    struct SetupConfig
    {
        /**
         * \brief Game object. Must be created by the user and remain valid for the
         *  entire engine lifetime.
         */
        Game* game;

        /**
         * \brief Internal ID for the application/game.
         *
         * Should be a short string that uniquely identifies the game.
         *
         * This is currently used to store per-game information, like
         * NVS data on ESP32.
         *
         * If null, a default value of "mygame" is used.
         *
         * \see Game::setApplicationID()
         */
        const char* appID;

        /**
         * \brief User-readable name for the application/game.
         *
         * This is used in places where the game is identified for the
         * user.
         *
         * If null, the application ID is used instead.
         *
         * \see Game::setApplicationName()
         */
        const char* appName;

        /**
         * \brief Mount point for an SD card, if any.
         *
         * If set to null, a default value of "/sdcard" is used.
         */
        const char* sdCardMountPoint;

        /**
         * \brief Mount point for internal storage (e.g. SPIFFS for ESP32).
         *
         * If set to null, a default value of "/storage" is used.
         */
        const char* internalStorageMountPoint;

        Screen* screen;

        /**
         * \brief Pin configuration.
         */
        struct {
            /**
             * \brief MISO pin for the SPI bus, or -1 if unused.
             */
            gpionum_t spiMISO;

            /**
             * \brief MOSI pin for the SPI bus, or -1 if unused.
             */
            gpionum_t spiMOSI;

            /**
             * \brief SCK pin for the SPI bus, or -1 if unused.
             */
            gpionum_t spiSCK;

            /**
             * \brief CS pin for the SD card on the SPI bus, or -1 if unused.
             */
            gpionum_t sdCardCS;

            /**
             * \brief Pin for the piezo speaker, or -1 if unused.
             */
            gpionum_t speaker;
        } pins;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
        struct
        {
            spi_host_device_t spiHost;
            size_t spiMaxTransferSize;
        } pfESPIDF;
#endif
    };

public:
    static void initConfig(SetupConfig& cfg);

public:
    DefaultEngine();

    virtual bool earlySetup();

    /**
     * \brief Run the initial engine setup.
     *
     * This sets up all engine components like the screen, input, audio, storage and
     * other subsystems.
     *
     * This method should be called exactly once at game startup, before the first
     * frame processing is done.
     *
     * @param cfg Engine configuration.
     * @return true if initialization is successful, false otherwise.
     */
    virtual bool setup(SetupConfig* cfg);

    /**
     * \brief Run the engine loop for a single frame.
     *
     * This includes collision checking, calling the user-defined game loop function,
     * rendering and other per-frame functions. It also handles sleeping at the end
     * of a frame to ensure a steady framerate.
     *
     * A user should call this method exactly once for every frame.
     *
     * @param gameLoopFunc User-defined game loop function. Can be null.
     * @return true if the game should keep running, false if it should quit.
     */
    virtual bool doFrame(void (*gameLoopFunc)(float), void (*postDrawFunc)(float));

    /**
     * \brief Shut down the engine.
     *
     * This method should be called once when the game has ended, i.e. after doFrame()
     * has returned false.
     */
    virtual void shutdown();

    /**
     * \brief Return the game object.
     *
     * @return The game object.
     */
    Game* getGame() { return game; }

    /**
     * \brief Return the screen used for the game.
     *
     * @return The game screen.
     */
    Screen* getScreen() { return screen; }

    /**
     * \brief Enable or disable logging of frame statistics to the console.
     *
     * @param print true to log statistics, false otherwise.
     */
    void setPrintFrameStatistics(bool print) { printFrameStats = print; }

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    spi_host_device_t getESPSPIHostDevice() const override;
    size_t getESPSPIMaxTransferSize() const override;
#endif

protected:
    virtual void initSPI(SetupConfig* cfg);
    virtual void shutdownSPI();

    virtual void initAudio(SetupConfig* cfg);
    virtual void shutdownAudio();

    virtual void initInput(SetupConfig* cfg);
    virtual void shutdownInput();

    virtual void initNetwork(SetupConfig* cfg);
    virtual void shutdownNetwork();

    virtual void initStorage(SetupConfig* cfg);
    virtual void shutdownStorage();

    virtual void initScreen(SetupConfig* cfg);
    virtual void shutdownScreen();

    virtual bool mountInternalStorage(SetupConfig* cfg);
    virtual void unmountInternalStorage();

    virtual bool mountSDCard(SetupConfig* cfg);
    virtual void unmountSDCard();

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    virtual void initSerial();
#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)
#endif

protected:
    SetupConfig setupCfg;

    bool earlySetupDone;
    Game* game;
    Screen* screen;

    bool printFrameStats;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    struct
    {
        spi_host_device_t spiHost;
        size_t spiMaxTransferSize;
    } pfESPIDF;
#endif

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    struct
    {
        QApplication* qapp;
    } pfDesktop;
#endif
};



#ifdef MINTGGGAMEENGINE_PORT_ARDUINO

#define MINTGGGAMEENGINE_STARTUP_CODE() \
        void setup()                    \
        {                               \
            EngineSetup();              \
        }                               \
        void loop()                     \
        {                               \
            if (!EngineLoop()) {        \
                EngineShutdown();       \
                for (;;);               \
            }                           \
        }

#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)

#define MINTGGGAMEENGINE_STARTUP_CODE() \
        extern "C" {                    \
        void app_main()                 \
        {                               \
            EngineSetup();              \
            while (EngineLoop());       \
            EngineShutdown();           \
        }                               \
        }

#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)

#define MINTGGGAMEENGINE_STARTUP_CODE() \
        extern "C" {                    \
        int main(int, char**)           \
        {                               \
            EngineSetup();              \
            while (EngineLoop());       \
            EngineShutdown();           \
            return 0;                   \
        }                               \
        }

#endif


}
