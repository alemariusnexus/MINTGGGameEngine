/*
 * This file is part of the TC358762 display driver addon.
 *
 * A driver addon for Raspberry Pi compatible displays based on the
 * TC358762 DSI-to-DPI bridge.
 *
 * Copyright 2026 David Lerch
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

// Large parts of the following code are adapted from the ESP-IDF component esp_lcd_rpi_dsi by Kuro, published under
// the GPLv2. That is why this code is released under GPLv2 as well.
//
//      https://github.com/kurozz/esp_lcd_rpi_dsi

#include "ScreenRPiTC358762.h"


#if MGG_ENABLE_STRONG_COPYLEFT_ADDONS

#include <cassert>

#include "../../../util/Log.h"
#include "../../../util/Util.h"

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
#   include <driver/i2c_master.h>
#   include <esp_err.h>
#   include <esp_lcd_panel_dev.h>
#   include <esp_lcd_panel_interface.h>
#   include <esp_lcd_panel_io.h>
#   include <esp_lcd_panel_ops.h>
#endif


// Register map of the ATtiny that's installed along the TC358762
#define ATTINY_REG_ADDR_ID              0x80    // For ATtiny firmware version
#define ATTINY_REG_ADDR_PORTA           0x81    // E.g. horizontal/vertical flip. We currently don't use the ATtiny for
                                                // this, since we also want swapXY support, which it doesn't have. Could
                                                // try this if swapXY=false to save some more frame time though.
#define ATTINY_REG_ADDR_PORTB           0x82
#define ATTINY_REG_ADDR_PORTC           0x83
#define ATTINY_REG_ADDR_POWERON         0x85
#define ATTINY_REG_ADDR_PWM             0x86    // For backlight brightness (0-255)
#define ATTINY_REG_ADDR_ADDR_L          0x8C
#define ATTINY_REG_ADDR_ADDR_H          0x8D
#define ATTINY_REG_ADDR_WRITE_DATA_H    0x90
#define ATTINY_REG_ADDR_WRITE_DATA_L    0x91

// Register map of the TC358762 itself
#define BRIDGE_REG_ADDR_PPI_STARTPPI            0x0104
#define BRIDGE_REG_ADDR_PPI_LPTXTIMECNT         0x0114
#define BRIDGE_REG_ADDR_PPI_D0S_ATMR            0x0144
#define BRIDGE_REG_ADDR_PPI_D1S_ATMR            0x0148
#define BRIDGE_REG_ADDR_PPI_D0S_CLRSIPOCOUNT    0x0164
#define BRIDGE_REG_ADDR_PPI_D1S_CLRSIPOCOUNT    0x0168
#define BRIDGE_REG_ADDR_DSI_STARTDSI            0x0204
#define BRIDGE_REG_ADDR_DSI_LANEENABLE          0x0210
#define BRIDGE_REG_ADDR_LCDCTRL                 0x0420
#define BRIDGE_REG_ADDR_SPICMR                  0x0450
#define BRIDGE_REG_ADDR_SYSCTRL                 0x0464


LOG_USE_TAG("ScreenRPiTC358762")


namespace MINTGGGameEngine
{

void ScreenRPiTC358762::initConfig(Config& cfg, bool useMainScreenConfig)
{
    // TODO: We could actually support BGR565 and RGB888 on ESP32-P4 using the PPA's color conversion.
    //  AbstractMIPIDSIScreen already contains a path that uses the PPA with its "final transform pass". We could
    //  hijack that and add the necessary color conversion. But we would have to make sure that the final transform
    //  pass is force-enabled in that case, which slows down the image transfer.
    cfg.dsi = {
        .engine = nullptr,

        .width = 800,
        .height = 480,

        .pixelFormat = PixelFormat::RGB565,     // Only BGR888 and RGB565 are supported, NOT their component-swapped
                                                // counterparts. We use RGB565 by default to lower the amount of data
                                                // transferred each frame, which dramatically decreases frame time
                                                // (especially since the renderer has to share the PSRAM with the DSI
                                                // hardware itself).
        .byteOrder = ByteOrder::LittleEndian,   // Only little-endian is supported

        .mirrorX = false,
        .mirrorY = false,
        .swapXY = false
    };

    if (!useMainScreenConfig) {
        return;
    }

#ifdef MGG_SCREEN_MAIN_WIDTH
    cfg.dsi.width = MGG_SCREEN_MAIN_WIDTH;
#endif
#ifdef MGG_SCREEN_MAIN_HEIGHT
    cfg.dsi.height = MGG_SCREEN_MAIN_HEIGHT;
#endif

#ifdef MGG_SCREEN_MAIN_PIXEL_FORMAT
    assert (
            MGG_SCREEN_MAIN_PIXEL_FORMAT == PixelFormat::RGB565
        ||  MGG_SCREEN_MAIN_PIXEL_FORMAT == PixelFormat::BGR888);
    cfg.dsi.pixelFormat = MGG_SCREEN_MAIN_PIXEL_FORMAT;
#endif

#ifdef MGG_SCREEN_MAIN_MIRROR_X
    cfg.dsi.mirrorX = static_cast<bool>(MGG_SCREEN_MAIN_MIRROR_X);
#endif
#ifdef MGG_SCREEN_MAIN_MIRROR_Y
    cfg.dsi.mirrorY = static_cast<bool>(MGG_SCREEN_MAIN_MIRROR_Y);
#endif
#ifdef MGG_SCREEN_MAIN_SWAP_XY
    cfg.dsi.swapXY = static_cast<bool>(MGG_SCREEN_MAIN_SWAP_XY);
#endif
}


ScreenRPiTC358762::ScreenRPiTC358762(const Config& cfg)
    : AbstractMIPIDSIScreen(cfg.dsi),
      cfg(cfg)
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
      , ldoChanHandle(nullptr),
        i2cBusHandle(nullptr), i2cAttinyHandle(nullptr),
        dsiBusHandle(nullptr), panelIOHandle(nullptr),
        i2cAttinyTimeout(100),
        dsiHALCtx(nullptr)
#endif
{
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    plfData.panelHandle = nullptr;
#endif
}

bool ScreenRPiTC358762::initPanel(uint8_t numFramebufs)
{
    LogInfo("Initializing RPi TC358762 screen.");

    // See comment further below for why only these pixel formats are supported
    if (cfg.dsi.pixelFormat != PixelFormat::BGR888  &&  cfg.dsi.pixelFormat != PixelFormat::RGB565) {
        LogError("TC358762 only supports pixel formats BGR888 and RGB565!");
        return false;
    }
    if (cfg.dsi.byteOrder != ByteOrder::LittleEndian) {
        LogError("TC358762 only supports little-endian byte order!");
        return false;
    }

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    // The following was adapted from: https://github.com/kurozz/esp_lcd_rpi_dsi (GNU GPL v2)

    // Enable power to the DSI PHY
    if (!initLDO()) {
        return false;
    }

    // Initialize I2C for the ATtiny that controls the display.
    if (!initI2C()) {
        return false;
    }

    // Initialize general DSI bus and DBI interface
    if (!initDSIDBI()) {
        return false;
    }

    if (!initLCDDriver(numFramebufs)) {
        return false;
    }

    esp_err_t err;

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
    err = esp_lcd_dpi_panel_enable_dma2d(plfData.panelHandle);
    if (err != ESP_OK) {
        LogError("Error enabling DMA2D: %s", esp_err_to_name(err));
        return false;
    }
#endif

    err = esp_lcd_panel_reset(plfData.panelHandle);
    if (err != ESP_OK) {
        LogError("Error resetting panel: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_lcd_panel_init(plfData.panelHandle);
    if (err != ESP_OK) {
        LogError("Error initializing panel: %s", esp_err_to_name(err));
        return false;
    }

    // These are NOT supported by the driver. The ATmega used with the TC358762 does seem to have a register to enable
    // mirroring, but the driver doesn't expose this, and who knows if it actually works with the various clones of
    // the ATmega and its firmware. In any way, there does not seem to be a (known) way to swap X/Y in hardware, so
    // we'll just fall back to the mirror/swap implementation of AbstractMIPIDSIScreen.
    /*err = esp_lcd_panel_mirror(plfData.panelHandle, cfg.dsi.mirrorX, cfg.dsi.mirrorY);
    if (err != ESP_OK) {
        LogError("Error mirroring panel: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_lcd_panel_swap_xy(plfData.panelHandle, true);
    if (err != ESP_OK) {
        LogError("Error swapping panel axes: %s", esp_err_to_name(err));
        return false;
    }*/

    /*if (config->flags.enable_touch) {
        // Non-fatal: a display with a dead or absent touch controller is still a display
        if (rpi_dsi_touch_start(disp, config->timing.h_size, config->timing.v_size) != ESP_OK) {
            ESP_LOGW(TAG, "no touch controller found, continuing without it");
            disp->touch = NULL;
        }
    }*/

    return true;
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    // Nothing to do.
    return true;
#else
    LogWarning("MIPI DSI displays are not supported on this platform.");
    return false;
#endif
}

void ScreenRPiTC358762::shutdownPanel()
{
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    /*if (handle->touch) {
        esp_lcd_touch_del(handle->touch);
    }
    if (handle->touch_io) {
        esp_lcd_panel_io_del(handle->touch_io);
    }*/
    shutdownLCDDriver();
    shutdownDSIDBI();
    shutdownI2C();
    shutdownLDO();
#endif
}



#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED

bool ScreenRPiTC358762::initLDO()
{
    // Configure LDO channel that powers the VDD_MIPI_DPHY pin. Most development board use LDO channel 3 for it.
    esp_ldo_channel_config_t ldoConfig = {
        .chan_id = 3, // TODO: Make configurable
        .voltage_mv = 2500,
    };
    esp_err_t err = esp_ldo_acquire_channel(&ldoConfig, &ldoChanHandle);
    if (err != ESP_OK) {
        LogError("Error powering the MIPI DSI PHY: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

void ScreenRPiTC358762::shutdownLDO()
{
    if (ldoChanHandle) {
        esp_ldo_release_channel(ldoChanHandle);
        ldoChanHandle = nullptr;
    }
}

bool ScreenRPiTC358762::initI2C()
{
    i2c_master_bus_config_t i2cConfig = {
        .i2c_port = -1,
        .sda_io_num = static_cast<gpio_num_t>(7),
        .scl_io_num = static_cast<gpio_num_t>(8),
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .trans_queue_depth = 0,
        .flags {
            .enable_internal_pullup = true,
            .allow_pd = false
        }
    };
    esp_err_t err = i2c_new_master_bus(&i2cConfig, &i2cBusHandle);
    if (err != ESP_OK) {
        LogError("Error creating I2C master bus: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

void ScreenRPiTC358762::shutdownI2C()
{
    if (i2cBusHandle) {
        i2c_del_master_bus(i2cBusHandle);
        i2cBusHandle = nullptr;
    }
}

bool ScreenRPiTC358762::initDSIDBI()
{
    esp_lcd_dsi_bus_config_t busConfig = {
        .bus_id = 0,
        .num_data_lanes = 1,
        .phy_clk_src = static_cast<mipi_dsi_phy_pllref_clock_source_t>(0),  // Let the driver choose (default differs
                                                                            // based on ESP32P4 revision)
        .lane_bit_rate_mbps = 700,
        .flags = {
            .clock_lane_force_hs = 1 // MUST be enabled because the TC358762 derives the display clock from the DSI clock
        }
    };
    esp_err_t err = esp_lcd_new_dsi_bus(&busConfig, &dsiBusHandle);
    if (err != ESP_OK) {
        LogError("Error creating DSI bus: %s", esp_err_to_name(err));
        return false;
    }

    // Doesn't really matter since the display doesn't use DCS at all.
    dbiVirtualChannel = 0;
    esp_lcd_dbi_io_config_t dbiConfig = {
        .virtual_channel = dbiVirtualChannel,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    err = esp_lcd_new_panel_io_dbi(dsiBusHandle, &dbiConfig, &panelIOHandle);
    if (err != ESP_OK) {
        LogError("Error creating DBI IO: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

void ScreenRPiTC358762::shutdownDSIDBI()
{
    if (panelIOHandle) {
        esp_lcd_panel_io_del(panelIOHandle);
        panelIOHandle = nullptr;
    }
    if (dsiBusHandle) {
        esp_lcd_del_dsi_bus(dsiBusHandle);
        dsiBusHandle = nullptr;
    }
}

bool ScreenRPiTC358762::initLCDDriver(uint8_t numFramebufs)
{
    lcd_color_format_t lcdColorFormat;

    // IMPORTANT: No, RGB888 and BGR565 are NOT currently supported. What MIPI DSI calls "RGB888" is actually
    //  transferred in B, G, R order, so it corresponds to our definition of BGR888 instead. RGB565 is supported if
    //  the correct value is written into the LCDCTRL register at initialization.
    //  The TC358762 does NOT appear to support swapping the channels, so RGB888 and BGR565 aren't directly supported.
    //  Supposedly, the ESP32-P4's DSI peripheral DOES have a way to swap component order in hardware before
    //  transferring it, but there doesn't currently seem to be an exposed high-level API in ESP-IDF to configure it
    //  to do that. rgb_ele_order below does NOT currently make a difference. And for now, I'd like to avoid directly
    //  tinkering with register values while also using the high-level ESP-IDF drivers.
    //  On a related note, it also seems like neither the TC358762 nor the P4 peripheral have a way to swap endianness,
    //  so only little-endian is supported for now.
    // TODO: Maybe get this to work if the situation described above changes at some point.
    switch (cfg.dsi.pixelFormat) {
    case PixelFormat::BGR888:
        lcdColorFormat = LCD_COLOR_FMT_RGB888;
        break;
    case PixelFormat::RGB565:
        lcdColorFormat = LCD_COLOR_FMT_RGB565;
        break;
    default:
        LogError("Unsupported pixel format for display.");
        return false;
    }

    // HACK: We need access to the mipi_dsi_hal_context_t to be able to send raw commands to the display controller.
    //  unfortunately there seems to be no way in ESP-IDF to get access to it, so we have to access internal driver
    //  structures here. That is extremely ugly, but apparently the only way right now.
    // TODO: If any of this code crashes, this is a likely candidate for why.
    struct private_esp_lcd_dsi_bus_t
    {
        int bus_id;
        mipi_dsi_hal_context_t hal;
    };
    dsiHALCtx = &reinterpret_cast<private_esp_lcd_dsi_bus_t*>(dsiBusHandle)->hal;

    // Timing below is the known-good one from the Linux driver panel-raspberrypi-touchscreen.c.
    esp_lcd_dpi_panel_config_t dpiConfig = {
        .virtual_channel = 0,
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz = 26,
        .in_color_format = lcdColorFormat,
        .out_color_format = lcdColorFormat,
        .num_fbs = numFramebufs,
        .video_timing = {
            .h_size = cfg.dsi.swapXY ? cfg.dsi.height : cfg.dsi.width,
            .v_size = cfg.dsi.swapXY ? cfg.dsi.width : cfg.dsi.height,
            .hsync_pulse_width = 2,
            .hsync_back_porch = 46,
            .hsync_front_porch = 1,
            .vsync_pulse_width = 2,
            .vsync_back_porch = 21,
            .vsync_front_porch = 7
        },
        .flags {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(6, 0, 0)
            .use_dma2d = 1,
#endif
            .disable_lp = 0
        }
    };

    i2c_device_config_t attinyCfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x45,
        .scl_speed_hz = 100000,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = 1
        }
    };
    esp_err_t err = i2c_master_bus_add_device(i2cBusHandle, &attinyCfg, &i2cAttinyHandle);
    if (err != ESP_OK) {
        LogError("Error adding ATtiny I2C device: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_lcd_new_panel_dpi(dsiBusHandle, &dpiConfig, &plfData.panelHandle);
    if (err != ESP_OK) {
        LogError("Error creating DPI panel: %s", esp_err_to_name(err));
        i2c_master_bus_rm_device(i2cAttinyHandle);
        return false;
    }

    panelOrigInit = plfData.panelHandle->init;
    panelOrigDel = plfData.panelHandle->del;

    plfData.panelHandle->reset = [](esp_lcd_panel_t* p)
            { return static_cast<ScreenRPiTC358762*>(p->user_data)->lcdPanelReset(p); };
    plfData.panelHandle->init = [](esp_lcd_panel_t* p)
            { return static_cast<ScreenRPiTC358762*>(p->user_data)->lcdPanelInit(p); };
    plfData.panelHandle->del = [](esp_lcd_panel_t* p)
            { return static_cast<ScreenRPiTC358762*>(p->user_data)->lcdPanelDel(p); };
    plfData.panelHandle->disp_on_off = [](esp_lcd_panel_t* p, bool onOff)
            { return static_cast<ScreenRPiTC358762*>(p->user_data)->lcdPanelDispOnOff(p, onOff); };
    plfData.panelHandle->user_data = this;

    return true;
}

void ScreenRPiTC358762::shutdownLCDDriver()
{
    if (plfData.panelHandle) {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
        esp_lcd_dpi_panel_disable_dma2d(plfData.panelHandle);
#endif
        esp_lcd_panel_del(plfData.panelHandle);
        plfData.panelHandle = nullptr;
    }
}

esp_err_t ScreenRPiTC358762::lcdPanelReset(esp_lcd_panel_t* panel)
{
    // Backlight off
    attinyWrite(ATTINY_REG_ADDR_PWM, 0);
    DelayTaskMs(10);

    // Configure horizontal/vertical flip as disabled for now
    attinyWrite(ATTINY_REG_ADDR_PORTA, 0);
    DelayTaskMs(10);

    // ???
    attinyWrite(ATTINY_REG_ADDR_PORTB, 0x02);
    DelayTaskMs(10);

    // Reset LCD and touch controller
    attinyWrite(ATTINY_REG_ADDR_PORTC, 0);
    DelayTaskMs(30);

    return ESP_OK;
}

esp_err_t ScreenRPiTC358762::lcdPanelInit(esp_lcd_panel_t* panel)
{
    // ********** ATTINY INIT SEQUENCE **********

    // Keep LCD and touch controller in reset mode
    esp_err_t err = attinyWrite(ATTINY_REG_ADDR_PORTC, 0);
    if (err != ESP_OK) {
        LogError("Error in LCD init (hold reset): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(10);

    // Configure horizontal flip
    err = attinyWrite(ATTINY_REG_ADDR_PORTA, 0x04);
    if (err != ESP_OK) {
        LogError("Error in LCD init (config flip): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(10);

    // Enable display power
    err = attinyWrite(ATTINY_REG_ADDR_PORTB, 0x80);
    if (err != ESP_OK) {
        LogError("Error in LCD init (enable power): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(10);

    // Enable backlight
    err = attinyWrite(ATTINY_REG_ADDR_PORTC, 0x01);
    if (err != ESP_OK) {
        LogError("Error in LCD init (enable backlight): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(80);

    // Release LCD and touch controller reset mode
    err = attinyWrite(ATTINY_REG_ADDR_PORTC, 0x0F);
    if (err != ESP_OK) {
        LogError("Error in LCD init (release reset): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(8);

    // This effectively writes 0 to the TC358762's register 0x047C (yes, a register on the bridge instead of the
    // ATtiny). What does this register mean and why do we write 0 to it? God only knows.
    err = attinyWrite(ATTINY_REG_ADDR_ADDR_H, 0x04);
    if (err != ESP_OK) {
        LogError("Error in LCD init (addr_h): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(8);

    err = attinyWrite(ATTINY_REG_ADDR_ADDR_L, 0x7c);
    if (err != ESP_OK) {
        LogError("Error in LCD init (addr_l): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(8);

    err = attinyWrite(ATTINY_REG_ADDR_WRITE_DATA_H, 0x00);
    if (err != ESP_OK) {
        LogError("Error in LCD init (write_data_h): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(8);

    err = attinyWrite(ATTINY_REG_ADDR_WRITE_DATA_L, 0x00);
    if (err != ESP_OK) {
        LogError("Error in LCD init (write_data_l): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(100);


    // ********** BRIDGE INIT SEQUENCE **********

    // This is adapted from the init sequence of the Linux tc358762 driver.

    // Enable clock and first DSI lane (we use single-lane mode)
    bridgeWrite(BRIDGE_REG_ADDR_DSI_LANEENABLE, 0x03);

    // Timers for the DSI lanes
    bridgeWrite(BRIDGE_REG_ADDR_PPI_D0S_CLRSIPOCOUNT, 0x05);
    bridgeWrite(BRIDGE_REG_ADDR_PPI_D1S_CLRSIPOCOUNT, 0x05);

    // ???
    bridgeWrite(BRIDGE_REG_ADDR_PPI_D0S_ATMR, 0x00);
    bridgeWrite(BRIDGE_REG_ADDR_PPI_D1S_ATMR, 0x00);

    bridgeWrite(BRIDGE_REG_ADDR_PPI_LPTXTIMECNT, 0x03);

    bridgeWrite(BRIDGE_REG_ADDR_SPICMR, 0x00);

    if (cfg.dsi.pixelFormat == PixelFormat::RGB565) {
        bridgeWrite(BRIDGE_REG_ADDR_LCDCTRL, 0x00100120);
    } else if (cfg.dsi.pixelFormat == PixelFormat::BGR888) {
        bridgeWrite(BRIDGE_REG_ADDR_LCDCTRL, 0x00100150);
    }

    bridgeWrite(BRIDGE_REG_ADDR_SYSCTRL, 0x040F);
    DelayTaskMs(100);

    // And finally start the interfaces
    bridgeWrite(BRIDGE_REG_ADDR_PPI_STARTPPI, 0x01);
    bridgeWrite(BRIDGE_REG_ADDR_DSI_STARTDSI, 0x01);
    DelayTaskMs(100);

    err = panelOrigInit(panel);
    if (err != ESP_OK) {
        LogError("Error in LCD init (original panel init): %s", esp_err_to_name(err));
        return err;
    }

    // Turn on backlight
    err = attinyWrite(ATTINY_REG_ADDR_PWM, 0xFF);
    if (err != ESP_OK) {
        LogError("Error in LCD init (turn on backlight): %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

esp_err_t ScreenRPiTC358762::lcdPanelDel(esp_lcd_panel_t* panel)
{
    // Turn off backlight
    esp_err_t err = attinyWrite(ATTINY_REG_ADDR_PWM, 0);
    if (err != ESP_OK) {
        LogError("Error in LCD deletion (turn off backlight): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(10);

    err = attinyWrite(ATTINY_REG_ADDR_PORTA, 0);
    if (err != ESP_OK) {
        LogError("Error in LCD deletion (PORTA): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(10);

    err = attinyWrite(ATTINY_REG_ADDR_PORTB, 0x02);
    if (err != ESP_OK) {
        LogError("Error in LCD deletion (PORTB): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(10);

    err = attinyWrite(ATTINY_REG_ADDR_PORTC, 0);
    if (err != ESP_OK) {
        LogError("Error in LCD deletion (PORTC): %s", esp_err_to_name(err));
        return err;
    }
    DelayTaskMs(30);

    return panelOrigDel(panel);
}

esp_err_t ScreenRPiTC358762::lcdPanelDispOnOff(esp_lcd_panel_t* panel, bool onOff)
{
    esp_err_t err = attinyWrite(ATTINY_REG_ADDR_PWM, onOff ? 0xFF : 0x00);
    if (err != ESP_OK) {
        LogError("Error turning on display %s: %s", onOff ? "on" : "off", esp_err_to_name(err));
    }

    return err;
}

esp_err_t ScreenRPiTC358762::attinyWrite(uint8_t addr, uint8_t value)
{
    const uint8_t data[] = {addr, value};
    return i2c_master_transmit(i2cAttinyHandle, data, 2, i2cAttinyTimeout);
}

esp_err_t ScreenRPiTC358762::attinyRead(uint8_t addr, uint8_t* value)
{
    // Write register address
    esp_err_t err = i2c_master_transmit(i2cAttinyHandle, &addr, 1, i2cAttinyTimeout);
    if (err != ESP_OK) {
        return err;
    }
    TimerDelaySpinUs(200);

    // Read response
    return i2c_master_receive(i2cAttinyHandle, value, 1, i2cAttinyTimeout);
}

void ScreenRPiTC358762::bridgeWrite(uint16_t addr, uint32_t value)
{
    // Address and value, little-endian
    const uint8_t data[] = {
        static_cast<uint8_t>(addr & 0xFF),
        static_cast<uint8_t>((addr >> 8) & 0xFF),

        static_cast<uint8_t>(value & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>((value >> 16) & 0xFF),
        static_cast<uint8_t>((value >> 24) & 0xFF),
    };
    mipi_dsi_hal_host_gen_write_long_packet (
        dsiHALCtx,
        dbiVirtualChannel,
        MIPI_DSI_DT_GENERIC_LONG_WRITE,
        data,
        sizeof(data)
        );
}

#endif

} // MINTGGGameEngine

#endif
