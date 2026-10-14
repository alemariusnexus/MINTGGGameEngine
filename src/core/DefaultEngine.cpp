#include "DefaultEngine.h"

#include <cassert>

#include "../graphics/Font.h"
#include "../util/Log.h"
#include "../util/Util.h"
#include "../graphics/screen/drivers/ScreenNull.h"
#include "../graphics/screen/drivers/mipidcs/ScreenILI9341.h"
#include "../graphics/screen/drivers/mipidcs/ScreenST7735.h"
#include "../addons/mipidsi/ScreenRPiTC358762.h"
#include "../platform/PlatformManager.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#include "esp_heap_caps.h"
#include "esp_idf_version.h"
#include "nvs_flash.h"
#endif

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
#include <QApplication>
#include <QFileInfo>
#include <QtVersion>

#include "../platform/desktop/MainWindow.h"
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


void DefaultEngine::initConfig(SetupConfig& cfg)
{
    cfg.game = nullptr;

    cfg.appID = MGG_APP_ID;
    cfg.appName = MGG_APP_NAME;

    cfg.sdCardMountPoint = MGG_STORAGE_SDCARD_MOUNT_POINT;
    cfg.internalStorageMountPoint = MGG_STORAGE_INTERNAL_MOUNT_POINT;

    cfg.pins.sdcard.miso = MGG_STORAGE_SDCARD_PIN_MISO;
    cfg.pins.sdcard.mosi = MGG_STORAGE_SDCARD_PIN_MOSI;
    cfg.pins.sdcard.sck = MGG_STORAGE_SDCARD_PIN_SCK;
    cfg.pins.sdcard.cs = MGG_STORAGE_SDCARD_PIN_CS;
    cfg.pins.audio.speaker = MGG_AUDIO_SPEAKER_PIN;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    cfg.pfESPIDF.spiHost = SPI2_HOST;
    cfg.pfESPIDF.spiMaxTransferSize = 4092;
#endif
}


DefaultEngine::DefaultEngine()
    : earlySetupDone(false), printFrameStats(false)
{
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    pfDesktop.qapp = nullptr;
#endif
}

bool DefaultEngine::earlySetup()
{
    DelayTaskMs(500);

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    initSerial();
#endif

    DelayTaskMs(500);

    LogInfo("********** BEGIN ENGINE SETUP **********");
    LogInfo("MINTGGGameEngine version: %s", MINTGGGAMEENGINE_VERSION_STRING);

    earlySetupDone = true;

    return true;
}

bool DefaultEngine::setup(SetupConfig* cfg, int argc, char** argv)
{
    if (!earlySetupDone) {
        if (!earlySetup()) {
            return false;
        }
        LogWarning("You forgot to call earlySetup() before setup()!");
    }

    setupCfg = *cfg;

    game = cfg->game;
    screen = nullptr;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    LogInfo("Platform: ESP-IDF (version %s, via Arduino)", esp_get_idf_version());
#else
    LogInfo("Platform: ESP-IDF (version %s)", esp_get_idf_version());
#endif
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    LogInfo("Platform: Arduino");
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    LogInfo("Platform: Desktop (using Qt %s, compiled for %s)", qVersion(), QT_VERSION_STR);
#endif

#if MGG_ENABLE_COPYLEFT_ADDONS
    LogWarning("Engine compiled with copyleft addons. Make sure you follow their licensing terms!");
#else
    LogInfo("Engine compiled without copyleft addons.");
#endif

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    pfDesktop.qapp = new QApplication(argc, argv);

    pfDesktop.qapp->setOrganizationName("MINTGGGameEngine");
    pfDesktop.qapp->setApplicationName(QString::fromStdString(game->getApplicationID()));
#endif

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    heap_caps_register_failed_alloc_callback(&HeapCapsAllocFailedHook);

    pfESPIDF.spiHost = cfg->pfESPIDF.spiHost;
    pfESPIDF.spiMaxTransferSize = cfg->pfESPIDF.spiMaxTransferSize;
#endif

    TimerInit();

    game->setApplicationID(cfg->appID);
    game->setApplicationName(cfg->appName);

    LogInfo("Application: %s (%s)", game->getApplicationName().c_str(), game->getApplicationID().c_str());

    LogInfo("Initializing platform...");
    initPlatform(cfg);

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

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    MainWindow::setup(game);
#endif

    LogInfo("********** END ENGINE SETUP **********");

    return true;
}

bool DefaultEngine::doFrame(void (*gameLoopFunc)(float), void (*postDrawFunc)(float))
{
    game->beginFrame();

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    pfDesktop.qapp->processEvents();
#endif

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

    return !game->isQuitRequested();
}

void DefaultEngine::shutdown()
{
    // TODO: Implement proper shutdown

    LogInfo("********** BEGIN ENGINE SHUTDOWN **********");

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    MainWindow::shutdown(game);
#endif

    unmountSDCard();

    unmountInternalStorage();

    LogInfo("Shutting down network...");
    shutdownNetwork();

    LogInfo("Shutting down input...");
    shutdownInput();

    LogInfo("Shutting down audio...");
    shutdownAudio();

    LogInfo("Shutting down screen...");
    shutdownScreen();

    LogInfo("Shutting down storage...");
    shutdownStorage();

    LogInfo("Shutting down SPI bus...");
    shutdownSPI();

    LogInfo("Shutting down platform...");
    shutdownPlatform();

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    pfDesktop.qapp->quit();
#endif

    LogInfo("********** END ENGINE SHUTDOWN **********");
}


void DefaultEngine::initStorage(SetupConfig* cfg)
{
    game->storage().begin(*game);

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    game->storage().setAppRootDirectory(QFileInfo(pfDesktop.appDir).absolutePath().toStdString());
#endif
}

void DefaultEngine::shutdownStorage()
{
    game->storage().shutdown();
}

void DefaultEngine::initPlatform(SetupConfig* cfg)
{
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    pfDesktop.cliParser.setApplicationDescription(QString("%1 (%2)")
        .arg(QString::fromStdString(game->getApplicationName()))
        .arg(QString::fromStdString(game->getApplicationID())));
    pfDesktop.cliParser.addHelpOption();

    QCommandLineOption appDirOpt (
        QStringList() << "d" << "appdir",
        "Path to app directory.",
        "APPDIR",
        qApp->applicationDirPath()
        );
    pfDesktop.cliParser.addOption(appDirOpt);

    pfDesktop.cliParser.process(*qApp);

    pfDesktop.appDir = pfDesktop.cliParser.value(appDirOpt);
#endif

    PlatformManager::getInstance().setup();
}

void DefaultEngine::shutdownPlatform()
{
    PlatformManager::getInstance().shutdown();
}

void DefaultEngine::initSPI(SetupConfig* cfg)
{
    if (cfg->pins.sdcard.mosi >= 0  ||  cfg->pins.sdcard.miso >= 0  ||  cfg->pins.sdcard.sck >= 0) {
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
        spi_bus_config_t busCfg = {
            .mosi_io_num = cfg->pins.sdcard.mosi,
            .miso_io_num = cfg->pins.sdcard.miso,
            .sclk_io_num = cfg->pins.sdcard.sck,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = static_cast<int>(cfg->pfESPIDF.spiMaxTransferSize),
            .flags = 0
        };

        esp_err_t res = spi_bus_initialize(cfg->pfESPIDF.spiHost, &busCfg, SPI_DMA_CH_AUTO);
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

void DefaultEngine::shutdownSPI()
{
}

void DefaultEngine::initAudio(SetupConfig* cfg)
{
    game->audio().begin(cfg->pins.audio.speaker);
}

void DefaultEngine::shutdownAudio()
{
    game->audio().shutdown();
}

void DefaultEngine::initInput(SetupConfig* cfg)
{
    game->input().begin();
}

void DefaultEngine::shutdownInput()
{
    game->input().shutdown();
}

void DefaultEngine::initNetwork(SetupConfig* cfg)
{
    game->network().begin();
}

void DefaultEngine::shutdownNetwork()
{
    game->network().shutdown();
}

void DefaultEngine::initScreen(SetupConfig* cfg)
{
    screen = nullptr;

#if MGG_SCREEN_MAIN_TYPE == MGG_SCREEN_TYPE_NULL
    screen = new ScreenNull;
#elif MGG_SCREEN_MAIN_TYPE == MGG_SCREEN_TYPE_ST7735_SPI
    ScreenST7735::Config screenCfg;
    ScreenST7735::initConfig(screenCfg);
    screenCfg.engine = this;
    screen = new ScreenST7735(screenCfg);
#elif MGG_SCREEN_MAIN_TYPE == MGG_SCREEN_TYPE_ILI9341_8080
    ScreenILI9341::Config screenCfg;
    ScreenILI9341::initConfig(screenCfg);
    screenCfg.engine = this;
    screen = new ScreenILI9341(screenCfg);
#elif MGG_SCREEN_MAIN_TYPE == MGG_SCREEN_TYPE_RPI_TC358762
#   if MGG_ENABLE_COPYLEFT_ADDONS
    ScreenRPiTC358762::Config screenCfg;
    ScreenRPiTC358762::initConfig(screenCfg);
    screenCfg.dsi.engine = this;
    screen = new ScreenRPiTC358762(screenCfg);
#   else
#       error "Can't use RPI_TC358762 screen without MGG_ENABLE_COPYLEFT_ADDONS!"
#   endif
#endif

    Font::loadDefaultFonts();

    if (!screen) {
        LogWarning("No screen configured. Using ScreenNull.");
        screen = new ScreenNull;
    }

    assert(screen);

    if (!screen->init()) {
        const auto screenWidth = screen->getWidth();
        const auto screenHeight = screen->getHeight();

        LogError("Error initializing screen. Falling back to ScreenNull.");
        delete screen;

        screen = new ScreenNull(screenWidth, screenHeight);
        if (!screen->init()) {
            LogError("Error initializing fallback screen as well. Giving up.");
        }
    }

    if (screen) {
        // Fill the screen once to avoid uninitialized data if Game::setBackgroundSkipped(true) is used.
        screen->fill(Color::BLACK, false);

        game->begin(*screen);
    }
}

void DefaultEngine::shutdownScreen()
{
    screen->shutdown();
    delete screen;
    screen = nullptr;
}

bool DefaultEngine::mountInternalStorage(SetupConfig* cfg)
{
    LogInfo("Mounting internal storage...");

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    return game->storage().mountSPIFFS(cfg->internalStorageMountPoint);
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    return game->storage().mountStandardPath(cfg->internalStorageMountPoint, "data");
#endif

    return false;
}

void DefaultEngine::unmountInternalStorage()
{
    LogInfo("Unmounting internal storage...");
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    game->storage().unmountSPIFFS(setupCfg.internalStorageMountPoint);
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    game->storage().unmountPath(setupCfg.internalStorageMountPoint);
#endif
}

bool DefaultEngine::mountSDCard(SetupConfig* cfg)
{
    if (cfg->pins.sdcard.cs >= 0) {
        LogInfo("Mounting SD card...");
        bool sdMountOk = false;
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
        sdMountOk = game->storage().mountSDCard (
            cfg->sdCardMountPoint,
            cfg->pfESPIDF.spiHost,
            cfg->pins.sdcard.cs
            );
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
        sdMountOk = game->storage().mountSDCard (
            cfg->sdCardMountPoint,
            *spi,
            cfg->pins.sdcard.cs
            );
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
        sdMountOk = game->storage().mountStandardPath(cfg->sdCardMountPoint, "sdcard");
#endif
        if (!sdMountOk) {
            LogError("Error mounting SD card. Trying to continue anyway...");
        }

        return sdMountOk;
    }

    return false;
}

void DefaultEngine::unmountSDCard()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    game->storage().unmountSDCard(setupCfg.sdCardMountPoint);
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    game->storage().unmountSDCard(setupCfg.sdCardMountPoint);
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    game->storage().unmountPath(setupCfg.sdCardMountPoint);
#endif
}

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF

spi_host_device_t DefaultEngine::getESPSPIHostDevice() const
{
    return pfESPIDF.spiHost;
}

size_t DefaultEngine::getESPSPIMaxTransferSize() const
{
    return pfESPIDF.spiMaxTransferSize;
}

#endif


#ifdef MINTGGGAMEENGINE_PORT_ARDUINO

void DefaultEngine::initSerial()
{
    Serial.begin(115200);
}

#endif

}
