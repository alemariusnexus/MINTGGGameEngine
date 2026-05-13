#pragma once

#include "../Globals.h"

#include "Game.h"


namespace MINTGGGameEngine
{


class DefaultEngine
{
public:
    struct SetupConfig
    {
        Game* game;

        const char* sdCardMountPoint;
        const char* internalStorageMountPoint;

        struct {
            int spiMISO;
            int spiMOSI;
            int spiSCK;

            int screenCS;
            int screenDC;
            int screenRST;

            int sdCardCS;

            int speaker;
        } pins;
    };

public:
    DefaultEngine();

    virtual bool setup(SetupConfig* cfg);

    virtual void doFrame(void (*gameLoopFunc)(float));

    Game* getGame() { return game; }
    Screen* getScreen() { return screen; }

    void setPrintFrameStatistics(bool print) { printFrameStats = print; }

protected:
    virtual void initAudio(SetupConfig* cfg);
    virtual void initInput(SetupConfig* cfg);
    virtual void initNetwork(SetupConfig* cfg);
    virtual void initStorage(SetupConfig* cfg);
    virtual void initScreen(SetupConfig* cfg);

    virtual bool mountInternalStorage(SetupConfig* cfg);
    virtual bool mountSDCard(SetupConfig* cfg);

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    virtual void initSerial(SetupConfig* cfg);
#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)
    virtual void initNVS(SetupConfig* cfg);
#endif

protected:
    Game* game;
    Screen* screen;

    bool printFrameStats;

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    SPIClass* spi;
    Adafruit_ST7735 tft;
#endif
};


}
