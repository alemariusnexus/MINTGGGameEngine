#include "DefaultEngine.h"

#include "../graphics/Font.h"
#include "../util/Log.h"
#include "../util/Util.h"
#include "../graphics/screen/drivers/ScreenNull.h"
#include "../graphics/screen/drivers/ScreenST7735.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#include "esp_heap_caps.h"
#include "nvs_flash.h"
#endif


LOG_USE_TAG("DefaultEngine")



namespace MINTGGGameEngine
{


#ifdef MINTGGGAMEENGINE_PORT_ESPIDF

void HeapCapsAllocFailedHook (
    size_t reqSize,
    uint32_t caps,
    const char *funcName
) {
    LogError("Failed to allocate %u bytes with 0x%X caps (%s)",
        static_cast<unsigned int>(reqSize), caps, funcName);
}

#endif


DefaultEngine::DefaultEngine()
    : earlySetupDone(false), printFrameStats(false)
{
}

bool DefaultEngine::earlySetup()
{
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    initSerial();
#endif

    LogInfo("*** BEGIN ENGINE SETUP ***");

    earlySetupDone = true;

    return true;
}

bool DefaultEngine::setup(SetupConfig* cfg)
{
    if (!earlySetupDone) {
        if (!earlySetup()) {
            return false;
        }
        LogWarning("You forgot to call earlySetup() before setup()!");
    }

    if (!cfg->internalStorageMountPoint) {
        cfg->internalStorageMountPoint = "/storage";
    }
    if (!cfg->sdCardMountPoint) {
        cfg->sdCardMountPoint = "/sdcard";
    }

    game = cfg->game;
    screen = cfg->screen;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    LogInfo("Platform: ESP-IDF");
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    LogInfo("Platform: Arduino");
#endif

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    heap_caps_register_failed_alloc_callback(&HeapCapsAllocFailedHook);
#endif

    TimerInit();

    game->setApplicationID(cfg->appID ? cfg->appID : "mygame");

    LogInfo("Initializing SPI bus...");
    initSPI(cfg);

    LogInfo("Initializing storage...");
    initStorage(cfg);

    LogInfo("Initializing screen...");
    initScreen(cfg);

    LogInfo("Initializing audio...");
    initAudio(cfg);

    LogInfo("Initializing input...");
    initInput(cfg);

    LogInfo("Initializing network...");
    initNetwork(cfg);

    mountInternalStorage(cfg);

	mountSDCard(cfg);

    LogInfo("*** END ENGINE SETUP ***");

    return true;
}

void DefaultEngine::doFrame(void (*gameLoopFunc)(float), void (*postDrawFunc)(float))
{
    game->beginFrame();

    const float dt = game->getFrameTime() * 1e-3f;

    const timer_ustick_t gameLoopTime = TimerGetTickcountUs();
    if (gameLoopFunc) {
        gameLoopFunc(dt);
    }

    const timer_ustick_t checkCollTime = TimerGetTickcountUs();
    game->checkCollisions(); // Kollisionsprüfung

    const timer_ustick_t drawTime = TimerGetTickcountUs();
    Game::DrawStats drawStats;

    // GameObjects zeichnen
    game->drawBegin(&drawStats);
    if (postDrawFunc) {
        postDrawFunc(dt);
    }
    game->drawFinish(&drawStats);

    const timer_ustick_t endTime = TimerGetTickcountUs();

    const float fps = 1e6f / (endTime - gameLoopTime);

    if (printFrameStats) {
        LogInfo(
            "Frame Stats   -   total: %uus (~%.2f FPS)   -   gameLoop: %uus, checkCollisions: %uus, draw: %uus   -   "
            "fill: %uus, objs: %uus, colls: %uus, rays: %uus, texts: %uus, comm: %uus",

            (uint32_t) (endTime-gameLoopTime),
            fps,

            (uint32_t) (checkCollTime-gameLoopTime),
            (uint32_t) (drawTime-checkCollTime),
            (uint32_t) (endTime-drawTime),

            drawStats.timeFillUs,
            drawStats.timeObjectsUs,
            drawStats.timeCollidersUs,
            drawStats.timeRaysUs,
            drawStats.timeTextsUs,
            drawStats.timeCommitUs
            );
    }

    game->endFrame();

    game->sleepNextFrame(); // Warten bis zum nächsten Frame
}


void DefaultEngine::initStorage(SetupConfig* cfg)
{
    game->storage().begin(*game);
}

void DefaultEngine::initSPI(SetupConfig* cfg)
{
    if (cfg->pins.spiMOSI >= 0  ||  cfg->pins.spiMISO >= 0  ||  cfg->pins.spiSCK >= 0) {
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
        spi_bus_config_t busCfg = {
            .mosi_io_num = cfg->pins.spiMOSI,
            .miso_io_num = cfg->pins.spiMISO,
            .sclk_io_num = cfg->pins.spiSCK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = static_cast<int>(cfg->spiMaxTransferSize),
            .flags = 0
        };

        esp_err_t res = spi_bus_initialize(cfg->spiHost, &busCfg, SPI_DMA_CH_AUTO);
        if (res != ESP_OK) {
            LogError("Error initializing SPI bus: %s", esp_err_to_name(res));
        }
#else
        LogWarning("SPI bus not currently implemented on this platform.");
#endif
    } else {
        LogWarning("SPI bus is disabled since not all pins are configured.");
    }
}

void DefaultEngine::initAudio(SetupConfig* cfg)
{
    game->audio().begin(cfg->pins.speaker);
}

void DefaultEngine::initInput(SetupConfig* cfg)
{
    game->input().begin();
}

void DefaultEngine::initNetwork(SetupConfig* cfg)
{
    game->network().begin();
}

void DefaultEngine::initScreen(SetupConfig* cfg)
{
    Font::loadDefaultFonts();

    if (!screen) {
        LogWarning("No screen configured. Using ScreenNull.");
        screen = new ScreenNull;
    }

    assert(screen);

    if (!screen->init()) {
        LogError("Error initializing screen. Falling back to ScreenNull.AAA");
        delete screen;

        screen = new ScreenNull;
        if (!screen->init()) {
            LogError("Error initializing fallback screen as well. Giving up.");
        }
    }

    if (screen) {
        game->begin(*screen);
    }
}

bool DefaultEngine::mountInternalStorage(SetupConfig* cfg)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    LogInfo("Mounting SPIFFS...");
    game->storage().mountSPIFFS(cfg->internalStorageMountPoint);
    return true;
#endif

    return false;
}

bool DefaultEngine::mountSDCard(SetupConfig* cfg)
{
    if (cfg->pins.sdCardCS >= 0) {
        LogInfo("Mounting SD card...");
        bool sdMountOk = false;
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
        sdMountOk = game->storage().mountSDCard (
            cfg->sdCardMountPoint,
            cfg->spiHost,
            cfg->pins.sdCardCS
            );
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
        sdMountOk = game->storage().mountSDCard (
            cfg->sdCardMountPoint,
            *spi,
            cfg->pins.sdCardCS
            );
#endif
        if (!sdMountOk) {
            LogError("Error mounting SD card. Trying to continue anyway...");
        }

        return sdMountOk;
    }

    return false;
}


#ifdef MINTGGGAMEENGINE_PORT_ARDUINO

void DefaultEngine::initSerial()
{
    Serial.begin(115200);
}

#endif


}
