#pragma once

#include "../../../../Globals.h"

#include "../../../../core/Engine.h"
#include "../../BufferedScreen.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <freertos/FreeRTOS.h>
#endif

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   ifdef MINTGGGAMEENGINE_CHIP_ESP32P4
#       define MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
#       define MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_ESP_PPA_SUPPORTED
#   endif
#endif

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
#   include <esp_lcd_mipi_dsi.h>
#endif

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_ESP_PPA_SUPPORTED
#   include <driver/ppa.h>
#endif


namespace MINTGGGameEngine
{

class AbstractMIPIDSIScreen : public BufferedScreen
{
public:
    struct Config
    {
        Engine* engine;

        uint16_t width;
        uint16_t height;

        PixelFormat pixelFormat;
        ByteOrder byteOrder;

        bool mirrorX;
        bool mirrorY;
        bool swapXY;
    };

protected:
    struct PlatformData
    {
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
        esp_lcd_panel_handle_t panelHandle;
#endif
    };

private:
    // You can't just change this number. The current algorithm implements triple buffering, which relies on having
    // exactly three buffers available.
    static constexpr size_t numFramebufs = 3;
    static_assert(numFramebufs == 3);

    struct Framebuf
    {
        void* buf;
    };

public:
    AbstractMIPIDSIScreen(const Config& cfg);
    ~AbstractMIPIDSIScreen() override;

    bool init() override;

    void shutdown() override;

    void commit() override;

protected:
    virtual bool initPanel(uint8_t numFramebufs) = 0;
    virtual void shutdownPanel() = 0;

    virtual PlatformData& getPlatformData() = 0;

    virtual bool supportsHardwareMirrorAndSwapXY() const { return false; }

private:
    bool initFinalTransformPass();
    void shutdownFinalTransformPass();

    bool initTripleBuffering();
    void shutdownTripleBuffering();

    bool useFinalTransformPass() const
            { return !supportsHardwareMirrorAndSwapXY()  &&  (cfg.mirrorX  ||  cfg.mirrorY  ||  cfg.swapXY); }
    bool mightUseHWForFinalTransformPass() const { return false; }
    void performFinalTransformPass(void* outBuf, bool* outputTouchedByCPU);

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_ESP_PPA_SUPPORTED
    bool performFinalTransformPassESPPPA(void* outBuf, Rotation rotation, FlipDir mirror);
#endif

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    static bool espOnFrameBufComplete (
        esp_lcd_panel_handle_t panel,
        esp_lcd_dpi_panel_event_data_t* edata,
        void* userCtx
        );
#endif

private:
    Config cfg;

    Framebuf framebufs[numFramebufs];
    void* additionalFramebuf;

    // The framebuf that the DSI peripheral is currently drawing to the screen from.
    volatile uint8_t displayingFramebufIdx;

    // The framebuf that the DSI peripheral will start drawing its next frame from. This may or may not be equal to
    // displayingFramebufIdx. It will be different if we have a "newer" frame to move on to, but still waiting for
    // the DSU peripheral to finish displaying the older frame. If there's no newer frame available, it will be equal
    // to displayingFramebufIdx.
    volatile uint8_t committedFramebufIdx;

    // The framebuf that the renderer task is currently writing to. This is the buffer that's actually used by all the
    // drawing routines in BufferedScreen.
    volatile uint8_t renderingFramebufIdx;

    // The framebuf that's currently neither used by the renderer nor the display, if any. A value of UINT8_MAX means
    // that no framebuf is currently free (which can happen when displayingFramebufIdx != committedFramebufIdx, meaning
    // the display currently holds two buffers, and the renderer holds the third).
    volatile uint8_t freeFramebufIdx; // May be UINT8_MAX if none is currently free

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    portMUX_TYPE framebufsSpinlock;
    BaseType_t rendererCoreID;
    volatile BaseType_t isrCoreID;
#endif

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_ESP_PPA_SUPPORTED
    ppa_client_handle_t ppaSRMClient;
#endif
};

} // MINTGGGameEngine
