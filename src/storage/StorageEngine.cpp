#include "StorageEngine.h"

#include "../util/Log.h"

#ifdef ESP_PLATFORM
#include <esp_err.h>
#include <esp_vfs_fat.h>
#endif


LOG_USE_TAG("StorageEngine")


namespace MINTGGGameEngine
{


StorageEngine::StorageEngine()
{
}

bool StorageEngine::begin()
{
    return true;
}

bool StorageEngine::mountSDCard (
    const char* mountPoint,
    spi_host_device_t spiHost,
    int csPin,
    uint32_t clkFreq
) {
    if (!mountPoint) {
        return false;
    }
    if (csPin < 0) {
        return false;
    }

#ifdef ESP_PLATFORM
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = spiHost;
    host.max_freq_khz = static_cast<int>(clkFreq/1000);

    sdspi_device_config_t devCfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    devCfg.host_id = static_cast<spi_host_device_t>(host.slot);
    devCfg.gpio_cs = static_cast<gpio_num_t>(csPin);

    esp_vfs_fat_mount_config_t mountCfg = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 2048,
        .disk_status_check_enable = false,
        .use_one_fat = false
    };

    sdmmc_card_t* sdcard;
    esp_err_t res = esp_vfs_fat_sdspi_mount (
        mountPoint,
        &host,
        &devCfg,
        &mountCfg,
        &sdcard
        );
    if (res != ESP_OK) {
        LogError(TAG, "Error mounting SD card: %s", esp_err_to_name(res));
        return false;
    }

    return true;
#else
    LogError(TAG, "SD card mounting not supported on platform.");
    return false;
#endif
}


}
