/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "ScreenST7701.h"

#include "../../../../util/Log.h"

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
#   include <esp_lcd_panel_dev.h>
#   include <esp_lcd_panel_io.h>
#   include <esp_lcd_panel_ops.h>
#endif

LOG_USE_TAG("ScreenST7701")


namespace MINTGGGameEngine
{

void ScreenST7701::initConfig(Config& cfg, bool useMainScreenConfig)
{
    cfg.dsi = {
        .engine = nullptr,

        .width = 480,
        .height = 800,

        .pixelFormat = PixelFormat::RGB565,     // Supports RGB565, BGR565, RGB888, BGR888
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


ScreenST7701::ScreenST7701(const Config& cfg)
    : AbstractMIPIDSIScreen(cfg.dsi),
      cfg(cfg)
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
      , ldoChanHandle(nullptr), dsiBusHandle(nullptr),
      panelIOHandle(nullptr)
#endif
{
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    plfData.panelHandle = nullptr;
#endif
}

bool ScreenST7701::initPanel(uint8_t numFramebufs)
{
    LogInfo("Initializing ST7701 screen.");

    if (    cfg.dsi.pixelFormat != PixelFormat::RGB888
        &&  cfg.dsi.pixelFormat != PixelFormat::BGR888
        &&  cfg.dsi.pixelFormat != PixelFormat::RGB565
        &&  cfg.dsi.pixelFormat != PixelFormat::BGR565
    ) {
        LogError("Unsupported pixel format!");
        return false;
    }
    if (cfg.dsi.byteOrder != ByteOrder::LittleEndian) {
        LogError("Unsupported byte order!");
        return false;
    }

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    if (!initLDO()) {
        return false;
    }
    if (!initCustomPanelHW()) {
        return false;
    }
    if (!initDSIDBI()) {
        return false;
    }
    if (!initLCDDriver(numFramebufs)) {
        return false;
    }

    esp_err_t err = esp_lcd_panel_reset(plfData.panelHandle);
    if (err != ESP_OK) {
        LogError("Error resetting panel: %s", esp_err_to_name(err));
        return false;
    }

    // Disabled for now. See comment in supportsHardwareTransforms() for why.
    /*if (shouldDoHardwareTransforms()  &&  (cfg.dsi.mirrorX || cfg.dsi.mirrorY)) {
        err = esp_lcd_panel_mirror(plfData.panelHandle, cfg.dsi.mirrorX, cfg.dsi.mirrorY);
        if (err != ESP_OK) {
            LogError("Error applying mirroring: %s", esp_err_to_name(err));
            return false;
        }
    }*/

    err = esp_lcd_panel_init(plfData.panelHandle);
    if (err != ESP_OK) {
        LogError("Error initializing panel: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_lcd_panel_disp_on_off(plfData.panelHandle, true);
    if (err != ESP_OK) {
        LogError("Error turning on panel: %s", esp_err_to_name(err));
        return false;
    }

    return true;
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    // Nothing to do.
    return true;
#else
    LogWarning("MIPI DSI displays are not supported on this platform.");
    return false;
#endif
}

void ScreenST7701::shutdownPanel()
{
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    shutdownLCDDriver();
    shutdownDSIDBI();
    shutdownCustomPanelHW();
    shutdownLDO();
#endif
}

bool ScreenST7701::supportsHardwareTransforms(bool mirrorX, bool mirrorY, bool swapXY) const
{
    // ST7701 supports mirroring along both axes, but NOT swapping the axes (unfortunately)
    if (AbstractMIPIDSIScreen::supportsHardwareTransforms(mirrorX, mirrorY, swapXY)) {
        return true;
    }
    // The ST7701 does actually support both horizontal and vertical mirroring (through MADCTL and SDIR registers), and
    // the esp_lcd_st7701 driver implements this. Unfortunately, I have only been getting corrupted data on the screen
    // with mirrorY=true so far, and I don't see why. mirrorX=true works, and we could support that, but that alone
    // isn't that useful, so we won't for now. In any case, swapXY does NOT seem to be supported by the chip at all.
    // TODO: Maybe figure out why mirrorY=true doesn't work and fix it.
    //return !swapXY;
    return false;
}


#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED

bool ScreenST7701::initLDO()
{
    esp_ldo_channel_config_t ldoCfg = {
        .chan_id = 3,
        .voltage_mv = 2500
    };
    esp_err_t err = esp_ldo_acquire_channel(&ldoCfg, &ldoChanHandle);
    if (err != ESP_OK) {
        LogError("Error powering the MIPI DSI PHY: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

void ScreenST7701::shutdownLDO()
{
    if (ldoChanHandle) {
        esp_ldo_release_channel(ldoChanHandle);
        ldoChanHandle = nullptr;
    }
}

bool ScreenST7701::initDSIDBI()
{
    esp_lcd_dsi_bus_config_t busConfig = {
        .bus_id = 0,
        .num_data_lanes = 2,
        .phy_clk_src = static_cast<mipi_dsi_phy_pllref_clock_source_t>(0),  // Let the driver choose (default differs
                                                                            // based on ESP32P4 revision)
        .lane_bit_rate_mbps = 500
    };
    esp_err_t err = esp_lcd_new_dsi_bus(&busConfig, &dsiBusHandle);
    if (err != ESP_OK) {
        LogError("Error creating DSI bus: %s", esp_err_to_name(err));
        return false;
    }

    esp_lcd_dbi_io_config_t dbiConfig = ST7701_PANEL_IO_DBI_CONFIG();
    err = esp_lcd_new_panel_io_dbi(dsiBusHandle, &dbiConfig, &panelIOHandle);
    if (err != ESP_OK) {
        LogError("Error creating DBI IO: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

void ScreenST7701::shutdownDSIDBI()
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

bool ScreenST7701::initLCDDriver(uint8_t numFramebufs)
{
    lcd_color_format_t lcdColorFormat;
    lcd_rgb_element_order_t lcdRgbElemOrder;

    // NOTE: For MIPI DSI, RGB888 is actually transferred in the order of B, G, R, which is why RGB888 and BGR888 are
    // reversed below. The same is NOT true for RGB565/BGR565 though.
    switch (cfg.dsi.pixelFormat) {
    case PixelFormat::RGB888:
        lcdColorFormat = LCD_COLOR_FMT_RGB888;
        lcdRgbElemOrder = LCD_RGB_ELEMENT_ORDER_BGR;
        break;
    case PixelFormat::BGR888:
        lcdColorFormat = LCD_COLOR_FMT_RGB888;
        lcdRgbElemOrder = LCD_RGB_ELEMENT_ORDER_RGB;
        break;
    case PixelFormat::RGB565:
        lcdColorFormat = LCD_COLOR_FMT_RGB565;
        lcdRgbElemOrder = LCD_RGB_ELEMENT_ORDER_RGB;
        break;
    case PixelFormat::BGR565:
        lcdColorFormat = LCD_COLOR_FMT_RGB565;
        lcdRgbElemOrder = LCD_RGB_ELEMENT_ORDER_BGR;
        break;
    default:
        LogError("Unsupported pixel format for display.");
        return false;
    }

    esp_lcd_dpi_panel_config_t dpiCfg = {
        .virtual_channel = 0,
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz = 15,
        .in_color_format = lcdColorFormat,
        .out_color_format = lcdColorFormat,
        .num_fbs = numFramebufs,
        .video_timing = {
            .h_size = cfg.dsi.swapXY ? cfg.dsi.height : cfg.dsi.width,
            .v_size = cfg.dsi.swapXY ? cfg.dsi.width : cfg.dsi.height,
            .hsync_pulse_width = 10,
            .hsync_back_porch = 10,
            .hsync_front_porch = 20,
            .vsync_pulse_width = 10,
            .vsync_back_porch = 10,
            .vsync_front_porch = 10,
        },
    };

    const st7701_lcd_init_cmd_t* initCmds = nullptr;
    const size_t numInitCmds = getCustomPanelInitSequence(&initCmds);

    st7701_vendor_config_t vendorCfg = {
        .init_cmds = initCmds,
        .init_cmds_size = static_cast<uint16_t>(numInitCmds),
        .mipi_config = {
            .dsi_bus = dsiBusHandle,
            .dpi_config = &dpiCfg
        },
        .flags = {
            .use_mipi_interface = 1
        }
    };

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
    esp_lcd_panel_dev_config_t panelCfg = {
        .rgb_ele_order = lcdRgbElemOrder,
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        .bits_per_pixel = static_cast<uint32_t>(CalcPixelFormatBytesPerPixel(cfg.dsi.pixelFormat) * 8),
        .reset_gpio_num = static_cast<gpio_num_t>(-1),
        .vendor_config = &vendorCfg
    };
#else
    esp_lcd_panel_dev_config_t panelCfg = {
        .reset_gpio_num = static_cast<gpio_num_t>(-1),
        .rgb_ele_order = lcdRgbElemOrder,
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        .bits_per_pixel = static_cast<uint32_t>(CalcPixelFormatBytesPerPixel(cfg.dsi.pixelFormat) * 8),
        .vendor_config = &vendorCfg
    };
#endif

    esp_err_t err = esp_lcd_new_panel_st7701(panelIOHandle, &panelCfg, &plfData.panelHandle);
    if (err != ESP_OK) {
        LogError("Error initializing ST7701 panel: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

void ScreenST7701::shutdownLCDDriver()
{
    if (plfData.panelHandle) {
        esp_lcd_panel_del(plfData.panelHandle);
        plfData.panelHandle = nullptr;
    }
}

#endif

}
