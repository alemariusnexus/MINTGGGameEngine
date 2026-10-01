#include "ScreenILI9341.h"

#include <algorithm>

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <driver/gpio.h>
#endif

#include "../../../util/Util.h"


LOG_USE_TAG("ScreenILI9341")


#ifndef MINTGGGAMEENGINE_CHIP_ESP32C3
#   define LCD_I80_DRIVER_SUPPORTED
#endif

#define LCD_PERIPH_MAX_TRANSFER_BYTES   (10*320*2+1)


namespace MINTGGGameEngine
{


void ScreenILI9341::initConfig(Config& cfg)
{
    cfg.width = 320;
    cfg.height = 240;

    cfg.offsetX = 0;
    cfg.offsetY = 0;

    cfg.staticBuffer = nullptr;

    cfg.mirrorX = false;
    cfg.mirrorY = false;
    cfg.swapXY = true;
    cfg.bgrColors = true;

    cfg.clockFreqHz = 10000000;

    cfg.pins.cs     = -1;
    cfg.pins.dc     = -1;
    cfg.pins.rst    = -1;
    cfg.pins.wr     = -1;
    cfg.pins.rd     = -1;
    cfg.pins.dat[0] = -1;
    cfg.pins.dat[1] = -1;
    cfg.pins.dat[2] = -1;
    cfg.pins.dat[3] = -1;
    cfg.pins.dat[4] = -1;
    cfg.pins.dat[5] = -1;
    cfg.pins.dat[6] = -1;
    cfg.pins.dat[7] = -1;
}


ScreenILI9341::ScreenILI9341(const Config& cfg)
    : AbstractMIPIScreen(
        cfg.width, cfg.height,
        cfg.offsetX, cfg.offsetY,
        cfg.mirrorX, cfg.mirrorY, cfg.swapXY,
        cfg.bgrColors,
#ifdef MINTGGGAMEENGINE_LITTLE_ENDIAN
        true, // ILI9341 expects big-endian pixel data -> swap endianness
#else
        false, // Don't swap endianness
#endif
        cfg.staticBuffer
        ),
      cfg(cfg)
{
}

ScreenILI9341::~ScreenILI9341()
{
    // TODO: Cleanup bus
}

void ScreenILI9341::commit()
{
    BufferedScreen::commit();

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    auto w = getWidth();
    auto h = getHeight();

    if (w == 0  ||  h == 0) {
        return;
    }

    const size_t singleLineSize = w * sizeof(uint16_t);
    const size_t maxLinesPerTransfer = LCD_PERIPH_MAX_TRANSFER_BYTES / singleLineSize;

    uint16_t x1 = 0;
    uint16_t y1 = 0;

    uint8_t* buffer = fb.getBuffer();

    while (h != 0) {
        size_t singleTrfH = std::min(static_cast<size_t>(h), maxLinesPerTransfer);
        const uint16_t x2 = x1 + w - 1;
        const uint16_t y2 = y1 + singleTrfH - 1;
        const size_t size = w * singleTrfH * sizeof(uint16_t);

        if (!mipiSetAddress(x1, y1, x2, y2)) {
            return;
        }
        if (!mipiWriteCommandPlusData(MIPI_DCS_WRITE_MEMORY_START, buffer, size)) {
            return;
        }

        buffer += size;
        y1 += singleTrfH;
        h -= singleTrfH;
    }
#endif
}

bool ScreenILI9341::mipiInitBus()
{
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
#ifdef LCD_I80_DRIVER_SUPPORTED
    // ********** GPIO SETUP **********

    esp_err_t res;

    if (cfg.pins.cs >= 0) {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << cfg.pins.cs,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring CS pin: %s", esp_err_to_name(res));
            return false;
        }
        gpio_set_level(static_cast<gpio_num_t>(cfg.pins.cs), 1);
    }

    {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << cfg.pins.dc,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring DC pin: %s", esp_err_to_name(res));
            return false;
        }
    }

    if (cfg.pins.rst >= 0) {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << cfg.pins.rst,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring RST pin: %s", esp_err_to_name(res));
            return false;
        }
        gpio_set_level(static_cast<gpio_num_t>(cfg.pins.rst), 1);
    }

    {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << cfg.pins.wr,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring WR pin: %s", esp_err_to_name(res));
            return false;
        }
        gpio_set_level(static_cast<gpio_num_t>(cfg.pins.wr), 1);
    }

    if (cfg.pins.rd >= 0) {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << cfg.pins.rd,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring RD pin: %s", esp_err_to_name(res));
            return false;
        }
        gpio_set_level(static_cast<gpio_num_t>(cfg.pins.wr), 1);
    }

    unsigned int datPinIdx = 0;
    for (int datPin : cfg.pins.dat) {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << datPin,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring DAT[%u] pin: %s", datPinIdx, esp_err_to_name(res));
            return false;
        }
        gpio_set_level(static_cast<gpio_num_t>(datPin), 1);
        datPinIdx++;
    }


    // ********** LCD DRIVER SETUP **********

    esp_lcd_i80_bus_config_t busCfg = {
        .dc_gpio_num = static_cast<gpio_num_t>(cfg.pins.dc),
        .wr_gpio_num = static_cast<gpio_num_t>(cfg.pins.wr),
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .data_gpio_nums = {
            static_cast<gpio_num_t>(cfg.pins.dat[0]),
            static_cast<gpio_num_t>(cfg.pins.dat[1]),
            static_cast<gpio_num_t>(cfg.pins.dat[2]),
            static_cast<gpio_num_t>(cfg.pins.dat[3]),
            static_cast<gpio_num_t>(cfg.pins.dat[4]),
            static_cast<gpio_num_t>(cfg.pins.dat[5]),
            static_cast<gpio_num_t>(cfg.pins.dat[6]),
            static_cast<gpio_num_t>(cfg.pins.dat[7])
        },
        .bus_width = 8,
        .max_transfer_bytes = LCD_PERIPH_MAX_TRANSFER_BYTES,
        .dma_burst_size = 64
    };
    res = esp_lcd_new_i80_bus(&busCfg, &busHandle);
    if (res != ESP_OK) {
        LogError("Error creating LCD I80 bus: %s", esp_err_to_name(res));
        return false;
    }

    esp_lcd_panel_io_i80_config_t ioCfg = {
        .cs_gpio_num = static_cast<gpio_num_t>(cfg.pins.cs),
        .pclk_hz = cfg.clockFreqHz,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1
        }
    };
    res = esp_lcd_new_panel_io_i80(busHandle, &ioCfg, &ioHandle);
    if (res != ESP_OK) {
        LogError("Error creating I80 IO handle: %s", esp_err_to_name(res));
        return false;
    }


    return true;
#else
    // We could bit-bang the protocol, but that's too slow to give a smooth frame experience at the screen sizes this
    // chip is typically used for. The chips without the I80 driver also probably don't even have enough pins for the
    // parallel interface.
    LogError("ILI9341 with parallel interface is not supported on this chip!");
    return false;
#endif
#else
    return false;
#endif
}

bool ScreenILI9341::mipiWriteCommandPlusData(uint8_t command, const uint8_t* data, size_t length)
{
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    esp_err_t res;
    if (length <= 1) {
        res = esp_lcd_panel_io_tx_param(ioHandle, command, data, length);
    } else {
        res = esp_lcd_panel_io_tx_color(ioHandle, command, data, length);
    }
    if (res != ESP_OK) {
        LogError("Error sending MIPI command %02X: %s", command, esp_err_to_name(res));
        return false;
    }
    return true;
#else
    return false;
#endif
}

bool ScreenILI9341::doHWReset()
{
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    if (cfg.pins.rst < 0) {
        return false;
    }

    gpio_set_level(static_cast<gpio_num_t>(cfg.pins.rst), 0);
    DelayTaskMs(100);
    gpio_set_level(static_cast<gpio_num_t>(cfg.pins.rst), 1);
    DelayTaskMs(100);

    return true;
#else
    return false;
#endif
}


}
