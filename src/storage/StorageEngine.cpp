#include "StorageEngine.h"

#include "../core/Game.h"
#include "../util/Log.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#include <esp_err.h>
#include <esp_spiffs.h>
#include <esp_vfs_fat.h>
#endif


LOG_USE_TAG("StorageEngine")


namespace MINTGGGameEngine
{


StorageEngine::StorageEngine()
    : game(nullptr)
{
}

bool StorageEngine::begin(Game& game)
{
    this->game = &game;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    esp_err_t nvsRes = nvs_flash_init();
    if (nvsRes == ESP_ERR_NVS_NO_FREE_PAGES || nvsRes == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvsRes = nvs_flash_init();
    }
    if (nvsRes != ESP_OK) {
        LogError("Error initializing NVS: %s", esp_err_to_name(nvsRes));
    }

    std::string nsName = game.getApplicationID().substr(0, NVS_KEY_NAME_MAX_SIZE-1);
    LogInfo("Using NVS namespace: %s", nsName.data());

    nvsRes = nvs_open(nsName.data(), NVS_READWRITE, &nvsHandle);
    if (nvsRes != ESP_OK) {
        LogError("Error opening NVS namespace: %s", esp_err_to_name(nvsRes));
        return false;
    }


#endif

    return true;
}


#ifdef MINTGGGAMEENGINE_PORT_ESPIDF

bool StorageEngine::mountSDCard (
    const char* mountPoint,
    spi_host_device_t spiHost,
    gpionum_t csPin,
    uint32_t clkFreq
) {
    if (!mountPoint) {
        return false;
    }
    if (csPin < 0) {
        return false;
    }

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
        LogError("Error mounting SD card: %s", esp_err_to_name(res));
        return false;
    }

    return true;
}

bool StorageEngine::mountSPIFFS (
    const char* mountPoint
) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = mountPoint,
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t res = esp_vfs_spiffs_register(&conf);
    if (res != ESP_OK) {
        LogError("Error registering SPIFFS: %s", esp_err_to_name(res));
        return false;
    }

    return true;
}

bool StorageEngine::hasValue(const std::string_view& key)
{
    return nvs_find_key(nvsHandle, key.data(), nullptr) == ESP_OK;
}

bool StorageEngine::commitNVS()
{
    esp_err_t res = nvs_commit(nvsHandle);
    if (res != ESP_OK) {
        LogError("Error committing NVS: %s", esp_err_to_name(res));
        return false;
    }
    return true;
}

#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)

bool StorageEngine::mountSDCard (
    const char* mountPoint,
    SPIClass& spi,
    gpionum_t csPin
) {
    sdMountPath = mountPoint;

    // Remove trailing slashes
    while (sdMountPath.ends_with('/')) {
        sdMountPath.pop_back();
    }

    if (!SD.begin(csPin, spi)) {
        LogError("Error initializing SD card.");
        return false;
    }

    return true;
}

bool StorageEngine::checkSDFilePath(const std::string& path, std::string* outRelPath)
{
    if (!path.starts_with(sdMountPath)) {
        return false;
    }
    const size_t sdMountPathLen = sdMountPath.length();
    if (path.length() == sdMountPathLen) {
        // Exact mount point
        if (outRelPath) {
            *outRelPath = "/";
        }
        return true;
    } else if (path[sdMountPathLen] != '/') {
        // Not actually a path below the mount path (just starts the same)
        return false;
    }

    if (outRelPath) {
        *outRelPath = path.substr(sdMountPathLen);
    }

    return true;
}

bool StorageEngine::hasValue(const std::string_view& key)
{
    // TODO: Implement
    return false;
}

#endif


}
