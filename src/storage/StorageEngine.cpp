/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

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
    : game(nullptr), appRootDir("")
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
    sync();
}

void StorageEngine::setAppRootDirectory(const std::string& rootDir)
{
    if (appRootDir == rootDir) {
        return;
    }

    LogInfo("Set app root directory: %s", rootDir.c_str());

    appRootDir = rootDir;
}

std::string StorageEngine::getAppRootDirectory() const
{
    return appRootDir;
}

std::string StorageEngine::getConfigDirectory() const
{
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    auto configDir = appRootDir + "/config";

    if (QFileInfo(QString::fromStdString(configDir)).isDir()) {
        return configDir;
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

    auto cardSizeMB = static_cast<uint32_t>(
        (static_cast<uint64_t>(sdcard->csd.capacity) * sdcard->csd.sector_size) / (1024*1024));
    LogInfo("SD card mounted successfuly. Mount Point: %s, Name: %s, Size: %luMiB",
        mountPoint,
        sdcard->cid.name,
        static_cast<long unsigned>(cardSizeMB)
        );

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
        .partition_label = nullptr,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t res = esp_vfs_spiffs_register(&conf);
    if (res != ESP_OK) {
        LogError("Error registering SPIFFS: %s", esp_err_to_name(res));
        return false;
    }

    size_t totalSize, usedBytes;
    res = esp_spiffs_info(nullptr, &totalSize, &usedBytes);
    if (res != ESP_OK) {
        LogError("Error fetching SPIFFS info: %s", esp_err_to_name(res));
        return false;
    }

    LogInfo("SPIFFS mounted successfully. Mount Point: %s, Size: %uKiB, Used: %uKiB (%.1f%%)",
        mountPoint,
        static_cast<unsigned int>(totalSize / 1024),
        static_cast<unsigned int>(usedBytes / 1024),
        (static_cast<float>(usedBytes) / static_cast<float>(totalSize)) * 100.0f
        );

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

void StorageEngine::sync()
{
    commitNVS();
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

void StorageEngine::sync()
{
}

#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)

bool StorageEngine::mountPath(const char* mountPoint, const char* targetPath)
{
    if (mountPoint[0] != '/') {
        return false;
    }

    std::string mountPointStr(mountPoint);
    while (!mountPointStr.empty()  &&  mountPointStr.ends_with('/')) {
        mountPointStr.erase(mountPointStr.length()-1);
    }

    if (!QFileInfo(targetPath).isDir()) {
        LogError("Failed to mount path '%s' as '%s': No such file or directory", targetPath, mountPoint);
        return false;
    }

    mountPoints[mountPointStr] = targetPath;

    LogInfo("Mounted path '%s' as '%s' successfully.", targetPath, mountPoint);

    return true;
}

bool StorageEngine::mountStandardPath(const char* mountPoint, const char* targetDirName)
{
    using namespace std::string_literals;
    std::string targetPath = appRootDir + "/"s + targetDirName;
    return mountPath(mountPoint, targetPath.c_str());
}

void StorageEngine::unmountPath(const char* mountPoint)
{
    mountPoints.erase(mountPoint);
}

bool StorageEngine::hasValue(const std::string_view& key)
{
    return settings.contains(QString::fromUtf8(key.data(), key.length()));
}

void StorageEngine::sync()
{
    settings.sync();
}

#endif

std::string StorageEngine::resolvePath(const std::string_view& path)
{
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    using namespace std::string_literals;

    for (const auto& sdMountPoint : mountPoints) {
        if (path.starts_with(sdMountPoint.first)) {
            return sdMountPoint.second + std::string(path.substr(sdMountPoint.first.length()));
        }
    }
#endif
    return std::string(path);
}


}
