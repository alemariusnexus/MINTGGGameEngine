#include "ScreenST7735.h"

#include <algorithm>

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <driver/gpio.h>
#endif

#include "../../../util/Util.h"


LOG_USE_TAG("ScreenST7735")


namespace MINTGGGameEngine
{


void ScreenST7735::initConfig(Config& cfg)
{
    cfg.engine = nullptr;

    cfg.width = 160;
    cfg.height = 128;

    cfg.offsetX = 0;
    cfg.offsetY = 0;

    cfg.staticBuffer = nullptr;

    cfg.mirrorX = true;
    cfg.mirrorY = false;
    cfg.swapXY = true;
    cfg.bgrColors = false;

    cfg.clockFreqHz = 40000000;

    cfg.pins.cs = -1;
    cfg.pins.dc = -1;
    cfg.pins.rst = -1;
}


ScreenST7735::ScreenST7735(const Config& cfg)
    : AbstractMIPIScreen (
        cfg.width, cfg.height,
        cfg.offsetX, cfg.offsetY,
        cfg.mirrorX, cfg.mirrorY, cfg.swapXY,
        cfg.bgrColors,
#ifdef MINTGGGAMEENGINE_LITTLE_ENDIAN
        true, // ST7735 expects big-endian pixel data -> swap endianness
#else
        false,
#endif
        cfg.staticBuffer
        ),
      cfg(cfg)
{
}

ScreenST7735::~ScreenST7735()
{
    // TODO: Cleanup bus
}

void ScreenST7735::commit()
{
    BufferedScreen::commit();

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    const auto w = getWidth();
    const auto h = getHeight();

    if (w == 0  ||  h == 0) {
        return;
    }

    if (!mipiSetAddress(0, 0, w-1, h-1)) {
        return;
    }
    mipiWriteCommandPlusData(MIPI_DCS_WRITE_MEMORY_START, fb.getBuffer(), w * h * sizeof(uint16_t));
#endif
}

bool ScreenST7735::mipiInitBus()
{
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    if (cfg.pins.cs >= 0) {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << cfg.pins.cs,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        esp_err_t res = gpio_config(&gpioCfg);
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
        esp_err_t res = gpio_config(&gpioCfg);
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
        esp_err_t res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring RST pin: %s", esp_err_to_name(res));
            return false;
        }
        gpio_set_level(static_cast<gpio_num_t>(cfg.pins.rst), 1);
    }

    spi_device_interface_config_t devCfg = {
        .mode = 0,
        .clock_speed_hz = static_cast<int>(cfg.clockFreqHz),
        .spics_io_num = cfg.pins.cs,
        .flags = SPI_DEVICE_NO_DUMMY,
        .queue_size = 8
    };
    esp_err_t res = spi_bus_add_device(cfg.engine->getESPSPIHostDevice(), &devCfg, &spiDev);
    if (res != ESP_OK) {
        LogError("Error adding SPI device: %s", esp_err_to_name(res));
        return false;
    }

    return true;
#else
    return false;
#endif
}

bool ScreenST7735::mipiWriteCommandPlusData(uint8_t command, const uint8_t* data, size_t length)
{
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    spi_transaction_t transaction = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 8,
        .tx_data = {command},
    };

    // Set DC -> command
    gpio_set_level(static_cast<gpio_num_t>(cfg.pins.dc), 0);
    esp_err_t res = spi_device_polling_transmit(spiDev, &transaction);
    if (res != ESP_OK) {
        LogError("Error sending MIPI command %02X: %s", command, esp_err_to_name(res));
        return false;
    }

    const size_t maxTransferSize = cfg.engine->getESPSPIMaxTransferSize();

    if (data  &&  length != 0) {
        // Set DC -> data
        gpio_set_level(static_cast<gpio_num_t>(cfg.pins.dc), 1);
        for (size_t i = 0; i < length; i += maxTransferSize) {
            size_t chunk = std::min(maxTransferSize, length - i);

            spi_transaction_t datTransaction = {
                .flags = 0,
                .length = chunk * 8,
                .tx_buffer = data + i,
                .rx_buffer = NULL
            };

            res = spi_device_polling_transmit(spiDev, &datTransaction);
            if (res != ESP_OK) {
                LogError("Error sending MIPI command %02X data: %s", command, esp_err_to_name(res));
                return false;
            }
        }
    }

    return true;
#else
    return false;
#endif
}

bool ScreenST7735::doHWReset()
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
