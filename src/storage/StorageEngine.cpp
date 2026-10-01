#include "StorageEngine.h"

#include "../core/Game.h"
#include "../util/Log.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <esp_err.h>
#   include <esp_spiffs.h>
#   include <esp_vfs_fat.h>
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
#   include <QApplication>
#   include <QFileInfo>
#   include <QString>
#endif


LOG_USE_TAG("StorageEngine")


namespace MINTGGGameEngine
{


StorageEngine& StorageEngine::getInstance()
{
    static StorageEngine inst;
    return inst;
}


StorageEngine::StorageEngine()
    : game(nullptr)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    sdcard = nullptr;
#endif
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
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    LogInfo("Value storage path: %s", settings.fileName().toUtf8().constData());
#else
    LogWarning("StorageEngine is not currently supported on this platform!");
#endif

    return true;
}

void StorageEngine::shutdown()
{
}

std::string StorageEngine::getConfigDirectory() const
{
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    const QString cfgDirPathCandidates[] = {
        QString("%1/config").arg(qApp->applicationDirPath())
    };

    for (const auto& cfgDirPath : cfgDirPathCandidates) {
        if (QFileInfo(cfgDirPath).isDir()) {
            return cfgDirPath.toStdString();
        }
    }

    return {};
#else
    // TODO: Establish a config directory for other ports
    return {};
#endif
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

void StorageEngine::unmountSDCard(const char* mountPoint)
{
    if (!mountPoint) {
        return;
    }
    if (!sdcard) {
        return;
    }

    esp_err_t res = esp_vfs_fat_sdcard_unmount(mountPoint, sdcard);
    if (res != ESP_OK) {
        LogError("Error unmounting SD card: %s", esp_err_to_name(res));
    }
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

void StorageEngine::unmountSPIFFS(const char* mountPoint)
{
    esp_err_t res = esp_vfs_spiffs_unregister(nullptr);
    if (res != ESP_OK) {
        LogError("Error unregistering SPIFFS: %s", esp_err_to_name(res));
    }
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

void StorageEngine::unmountSDCard(const char* mountPoint)
{
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

#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)

bool StorageEngine::mountSDCard (
        const char* mountPoint
) {
    if (mountPoint[0] != '/') {
        return false;
    }

    std::string mountPointStr(mountPoint);
    while (!mountPointStr.empty()  &&  mountPointStr.ends_with('/')) {
        mountPointStr.erase(mountPointStr.length()-1);
    }

    const QString mountDirCands[] = {
        QString("%1%2").arg(qApp->applicationDirPath()).arg(QString::fromStdString(mountPointStr))
    };

    for (const auto& mountDir : mountDirCands) {
        if (QFileInfo(mountDir).isDir()) {
            sdMountPoints[mountPointStr] = mountDir.toStdString();
            return true;
        }
    }

    return false;
}

void StorageEngine::unmountSDCard(const char* mountPoint)
{
    sdMountPoints.erase(mountPoint);
}

bool StorageEngine::hasValue(const std::string_view& key)
{
    return settings.contains(QString::fromUtf8(key.data(), key.length()));
}

#endif

std::string StorageEngine::resolvePath(const std::string& path)
{
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    using namespace std::string_literals;

    for (const auto& sdMountPoint : sdMountPoints) {
        if (path.starts_with(sdMountPoint.first)) {
            return sdMountPoint.second + path.substr(sdMountPoint.first.length());
        }
    }
#endif
    return path;
}


}
