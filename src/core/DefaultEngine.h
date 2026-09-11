#pragma once

#include "../Globals.h"

#include "Game.h"


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
class DefaultEngine
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
         * @see Game::setApplicationID()
         */
        const char* appID;

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

        spi_host_device_t spiHost;
        size_t spiMaxTransferSize;

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
    };

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
     */
    virtual void doFrame(void (*gameLoopFunc)(float), void (*postDrawFunc)(float));

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

protected:
    virtual void initSPI(SetupConfig* cfg);
    virtual void initAudio(SetupConfig* cfg);
    virtual void initInput(SetupConfig* cfg);
    virtual void initNetwork(SetupConfig* cfg);
    virtual void initStorage(SetupConfig* cfg);
    virtual void initScreen(SetupConfig* cfg);

    virtual bool mountInternalStorage(SetupConfig* cfg);
    virtual bool mountSDCard(SetupConfig* cfg);

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    virtual void initSerial();
#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)
#endif

protected:
    bool earlySetupDone;
    Game* game;
    Screen* screen;

    bool printFrameStats;

/*#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    SPIClass* spi;
    Adafruit_ST7735* tft;
#endif*/
};



#ifdef MINTGGGAMEENGINE_PORT_ARDUINO

#define MINTGGGAMEENGINE_STARTUP_CODE() \
        void setup()                    \
        {                               \
            EngineSetup();              \
        }                               \
        void loop()                     \
        {                               \
            EngineLoop();               \
        }

#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)

#define MINTGGGAMEENGINE_STARTUP_CODE() \
        extern "C" {                    \
        void app_main()                 \
        {                               \
            EngineSetup();              \
            for (;;) {                  \
                EngineLoop();           \
            }                           \
        }                               \
        }

#endif


}
