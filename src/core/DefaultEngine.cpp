#include "DefaultEngine.h"

#include "../util/Log.h"
#include "../util/Util.h"
#include "graphics/ScreenHAGL.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#include "esp_heap_caps.h"
#include "nvs_flash.h"

#include <hagl.h>
#include <hagl_hal.h>
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
    : screen(nullptr), printFrameStats(false)
{
}

bool DefaultEngine::setup(SetupConfig* cfg)
{
    if (!cfg->internalStorageMountPoint) {
        cfg->internalStorageMountPoint = "/internal";
    }
    if (!cfg->sdCardMountPoint) {
        cfg->sdCardMountPoint = "/sdcard";
    }

    game = cfg->game;

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    initSerial(cfg);
#endif

    LogInfo("*** BEGIN ENGINE SETUP ***");

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    LogInfo("Platform: ESP-IDF");
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    LogInfo("Platform: Arduino");
#endif

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    heap_caps_register_failed_alloc_callback(&HeapCapsAllocFailedHook);
#endif

    TimerInit();

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    initNVS(cfg);
#endif

    LogInfo("Initializing screen...");
    initScreen(cfg);

    LogInfo("Initializing audio...");
    initAudio(cfg);

    LogInfo("Initializing input...");
    initInput(cfg);

    LogInfo("Initializing network...");
    initNetwork(cfg);

    LogInfo("Initializing storage...");
    initStorage(cfg);

    mountInternalStorage(cfg);

	mountSDCard(cfg);

    LogInfo("*** END ENGINE SETUP ***");

    return true;
}

void DefaultEngine::doFrame(void (*gameLoopFunc)(float))
{
    game->beginFrame();

    float dt = game->getFrameTime() * 1e-3f;

    timer_ustick_t gameLoopTime = TimerGetTickcountUs();
    if (gameLoopFunc) {
        gameLoopFunc(dt);
    }

    timer_ustick_t checkCollTime = TimerGetTickcountUs();
    game->checkCollisions(); // Kollisionsprüfung

    timer_ustick_t drawTime = TimerGetTickcountUs();
    Game::DrawStats drawStats;
    game->draw(&drawStats); // GameObjects zeichnen

    timer_ustick_t endTime = TimerGetTickcountUs();

    if (printFrameStats) {
        LogInfo(
            "Frame Stats   -   total: %uus   -   gameLoop: %uus, checkCollisions: %uus, draw: %uus   -   "
            "fill: %uus, objs: %uus, colls: %uus, rays: %uus, texts: %uus, comm: %uus",

            (uint32_t) (endTime-gameLoopTime),

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

void DefaultEngine::initStorage(SetupConfig* cfg)
{
    StorageEngine::getInstance().begin();
}

void DefaultEngine::initScreen(SetupConfig* cfg)
{
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    spi = new SPIClass(*cfg->spiBase);
    spi->begin(cfg->pins.spiSCK, cfg->pins.spiMISO, cfg->pins.spiMOSI, cfg->pins.screenCS);

    tft = new Adafruit_ST7735(spi, cfg->pins.screenCS, cfg->pins.screenDC, cfg->pins.screenRST);
    ScreenST7735* stScreen = new ScreenST7735(*tft);
    stScreen->begin();
    screen = stScreen;
#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)
    ScreenHAGL* haglScreen = new ScreenHAGL;

    hagl_hal_custom_config_t haglCfg = {
        .width = 160,
        .height = 128,
        .offsetX = 0,
        .offsetY = 0,
        .clockFreqHz = 40000000,

        .invertColors = false,
        .blActiveLevel = 1,
        .blPwmDutyCycle = -1,

        .pins = {
            .miso = cfg->pins.spiMISO,
            .mosi = cfg->pins.spiMOSI,
            .sck = cfg->pins.spiSCK,
            .cs = cfg->pins.screenCS,
            .dc = cfg->pins.screenDC,
            .rst = cfg->pins.screenRST,
            .bl = -1
        }
    };
    hagl_hal_custom_config(&haglCfg);

    haglScreen->begin();

    screen = haglScreen;
#else
    screen = new ScreenNull;
#endif

    assert(screen);

    if (screen) {
        game->begin(*screen);
    }
}

bool DefaultEngine::mountInternalStorage(SetupConfig* cfg)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#ifdef MOUNT_SPIFFS
    LogInfo("Mounting SPIFFS...");
    StorageEngine::getInstance().mountSPIFFS(cfg->internalStorageMountPoint);
    return true;
#endif
#endif

    return false;
}

bool DefaultEngine::mountSDCard(SetupConfig* cfg)
{
    if (cfg->pins.sdCardCS >= 0) {
        LogInfo("Mounting SD card...");
        bool sdMountOk = false;
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
        sdMountOk = StorageEngine::getInstance().mountSDCard (
            cfg->sdCardMountPoint,
            static_cast<spi_host_device_t>(CONFIG_MIPI_DISPLAY_SPI_HOST),
            cfg->pins.sdCardCS
            );
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
        sdMountOk = StorageEngine::getInstance().mountSDCard (
            cfg->sdCardMountPoint,
            *spi,
            cfg->pins.sdCardCS
            );
#endif
        if (sdMountOk) {
            LogError("Error mounting SD card. Trying to continue anyway...");
        }

        return sdMountOk;
    }

    return false;
}


#ifdef MINTGGGAMEENGINE_PORT_ARDUINO

void DefaultEngine::initSerial(SetupConfig* cfg)
{
    Serial.begin(115200);
}

#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)

void DefaultEngine::initNVS(SetupConfig* cfg)
{
    esp_err_t nvsRes = nvs_flash_init();
    if (nvsRes == ESP_ERR_NVS_NO_FREE_PAGES || nvsRes == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvsRes = nvs_flash_init();
    }
    if (nvsRes != ESP_OK) {
        LogError("Error initializing NVS: %s", esp_err_to_name(nvsRes));
    }
}

#endif


}
