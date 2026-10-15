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

#pragma once

#include "../Globals.h"

#include <string>

#include "../util/Log.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <hal/spi_types.h>
#   include <nvs_flash.h>
#   include <esp_vfs_fat.h>
#endif

#ifndef MINTGGGAMEENGINE_PORT_ESPIDF
#   ifdef MINTGGGAMEENGINE_PORT_ARDUINO
#       include <SD.h>
#       include <SPI.h>
#   endif
#endif

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <unordered_map>
#   include <QSettings>
#endif


namespace MINTGGGameEngine
{

class Game;


class StorageEngine
{
public:
    static StorageEngine& getInstance();

public:
    bool begin(Game& game);

    void shutdown();

    void setAppRootDirectory(const std::string& rootDir);

    std::string getAppRootDirectory() const;
    std::string getConfigDirectory() const;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    bool mountSDCard (
        const char* mountPoint,
        spi_host_device_t spiHost = SPI2_HOST,
        gpionum_t csPin = -1,
        uint32_t clkFreq = 20000000
        );
    void unmountSDCard(const char* mountPoint);

    bool mountSPIFFS(const char* mountPoint);
    void unmountSPIFFS(const char* mountPoint);
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    bool mountSDCard (
        const char* mountPoint,
        SPIClass& spi = SPI,
        gpionum_t csPin = -1
        );
    void unmountSDCard(const char* mountPoint);

    bool checkSDFilePath(const std::string& path, std::string* outRelPath);
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    bool mountPath(const char* mountPoint, const char* targetPath);
    bool mountStandardPath(const char* mountPoint, const char* targetDirName);
    void unmountPath(const char* mountPoint);
#endif

    template <typename ValueT>
    bool writeValue(const std::string_view& key, ValueT value);

    template <typename ValueT>
    ValueT readValue(const std::string_view& key, ValueT defaultValue = {}, bool* ok = nullptr);

    bool hasValue(const std::string_view& key);

    void sync();

    std::string resolvePath(const std::string& path);

private:
    StorageEngine();

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    bool commitNVS();
#endif

private:
    Game* game;
    std::string appRootDir;
    std::string sdMountPath;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    nvs_handle_t nvsHandle;
    sdmmc_card_t* sdcard;
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    QSettings settings;
    std::unordered_map<std::string, std::string> mountPoints;
#endif
};


template <typename ValueT>
bool StorageEngine::writeValue(const std::string_view& key, ValueT value)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    esp_err_t res;

    if constexpr (std::is_signed_v<ValueT>  &&  sizeof(ValueT) == 8) {
        res = nvs_set_i64(nvsHandle, key.data(), static_cast<int64_t>(value));
    } else if constexpr (std::is_signed_v<ValueT>  &&  sizeof(ValueT) == 4) {
        res = nvs_set_i32(nvsHandle, key.data(), static_cast<int32_t>(value));
    } else if constexpr (std::is_signed_v<ValueT>  &&  sizeof(ValueT) == 2) {
        res = nvs_set_i16(nvsHandle, key.data(), static_cast<int16_t>(value));
    } else if constexpr (std::is_signed_v<ValueT>  &&  sizeof(ValueT) == 1) {
        res = nvs_set_i8(nvsHandle, key.data(), static_cast<int8_t>(value));
    } else if constexpr (std::is_unsigned_v<ValueT>  &&  sizeof(ValueT) == 8) {
        res = nvs_set_u64(nvsHandle, key.data(), static_cast<uint64_t>(value));
    } else if constexpr (std::is_unsigned_v<ValueT>  &&  sizeof(ValueT) == 4) {
        res = nvs_set_u32(nvsHandle, key.data(), static_cast<uint32_t>(value));
    } else if constexpr (std::is_unsigned_v<ValueT>  &&  sizeof(ValueT) == 2) {
        res = nvs_set_u16(nvsHandle, key.data(), static_cast<uint16_t>(value));
    } else if constexpr (std::is_unsigned_v<ValueT>  &&  sizeof(ValueT) == 1) {
        res = nvs_set_u8(nvsHandle, key.data(), static_cast<uint8_t>(value));
    } else if constexpr (std::is_floating_point_v<ValueT>  &&  sizeof(ValueT) == 4) {
        return writeValue(key, *reinterpret_cast<uint32_t*>(&value));
    } else if constexpr (std::is_floating_point_v<ValueT>  &&  sizeof(ValueT) == 8) {
        return writeValue(key, *reinterpret_cast<uint64_t*>(&value));
    } else if constexpr (std::is_same_v<ValueT, bool>) {
        res = nvs_set_u8(nvsHandle, key.data(), value ? 1 : 0);
    } else if constexpr (
            std::is_same_v<ValueT, std::string>
        ||  std::is_same_v<ValueT, std::string_view>
    ) {
        res = nvs_set_str(nvsHandle, key.data(), value.data());
    } else if constexpr (std::is_same_v<ValueT, const char*>) {
        res = nvs_set_str(nvsHandle, key.data(), value);
    } else {
        static_assert(false);
        res = ESP_OK;
    }
    if (res != ESP_OK) {
        ESP_LOGE("StorageEngine", "Error writing NVS value: %s", esp_err_to_name(res));
        return false;
    }
    return commitNVS();
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    auto qkey = QString::fromUtf8(key.data(), static_cast<qsizetype>(key.length()));
    if constexpr (
            std::is_same_v<ValueT, std::string>
        ||  std::is_same_v<ValueT, std::string_view>
    ) {
        settings.setValue(qkey, QVariant::fromValue(QByteArray::fromStdString(value)));
    } else {
        settings.setValue(qkey, QVariant::fromValue<ValueT>(value));
    }
    return true;
#else
    // TODO: Implement
    return false;
#endif
}

template <typename ValueT>
ValueT StorageEngine::readValue(const std::string_view& key, ValueT defaultValue, bool* ok)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    esp_err_t res;
    ValueT value = defaultValue;

    if constexpr (std::is_signed_v<ValueT>  &&  sizeof(ValueT) == 8) {
        res = nvs_get_i64(nvsHandle, key.data(), reinterpret_cast<int64_t*>(&value));
    } else if constexpr (std::is_signed_v<ValueT>  &&  sizeof(ValueT) == 4) {
        res = nvs_get_i32(nvsHandle, key.data(), reinterpret_cast<int32_t*>(&value));
    } else if constexpr (std::is_signed_v<ValueT>  &&  sizeof(ValueT) == 2) {
        res = nvs_get_i16(nvsHandle, key.data(), reinterpret_cast<int16_t*>(&value));
    } else if constexpr (std::is_signed_v<ValueT>  &&  sizeof(ValueT) == 1) {
        res = nvs_get_i8(nvsHandle, key.data(), reinterpret_cast<int8_t*>(&value));
    } else if constexpr (std::is_unsigned_v<ValueT>  &&  sizeof(ValueT) == 8) {
        res = nvs_get_u64(nvsHandle, key.data(), reinterpret_cast<uint64_t*>(&value));
    } else if constexpr (std::is_unsigned_v<ValueT>  &&  sizeof(ValueT) == 4) {
        res = nvs_get_u32(nvsHandle, key.data(), reinterpret_cast<uint32_t*>(&value));
    } else if constexpr (std::is_unsigned_v<ValueT>  &&  sizeof(ValueT) == 2) {
        res = nvs_get_u16(nvsHandle, key.data(), reinterpret_cast<uint16_t*>(&value));
    } else if constexpr (std::is_unsigned_v<ValueT>  &&  sizeof(ValueT) == 1) {
        res = nvs_get_u8(nvsHandle, key.data(), reinterpret_cast<uint8_t*>(&value));
    } else if constexpr (std::is_floating_point_v<ValueT>  &&  sizeof(ValueT) == 4) {
        uint32_t ivalue;
        res = nvs_get_u32(nvsHandle, key.data(), &ivalue);
        if (res == ESP_OK) {
            value = *reinterpret_cast<ValueT*>(&ivalue);
        }
    } else if constexpr (std::is_floating_point_v<ValueT>  &&  sizeof(ValueT) == 8) {
        uint64_t ivalue;
        res = nvs_get_u64(nvsHandle, key.data(), &ivalue);
        if (res == ESP_OK) {
            value = *reinterpret_cast<ValueT*>(&ivalue);
        }
    } else if constexpr (std::is_same_v<ValueT, bool>) {
        uint8_t ivalue;
        res = nvs_get_u8(nvsHandle, key.data(), &ivalue);
        if (res == ESP_OK) {
            value = (ivalue != 0);
        }
    } else if constexpr (
            std::is_same_v<ValueT, std::string>
        ||  std::is_same_v<ValueT, std::string_view>
    ) {
        // TODO: Handle this
        static_assert(false);
        res = ESP_FAIL;
    } else if constexpr (std::is_same_v<ValueT, const char*>) {
        // TODO: Handle this
        static_assert(false);
        res = ESP_FAIL;
    } else {
        static_assert(false);
        res = ESP_FAIL;
    }
    if (res != ESP_OK) {
        ESP_LOGE("StorageEngine", "Error reading NVS value: %s", esp_err_to_name(res));
        if (ok) *ok = false;
    } else {
        if (ok) *ok = true;
    }
    return value;
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    auto qkey = QString::fromUtf8(key.data(), key.length());
    if constexpr (
            std::is_same_v<ValueT, std::string>
        ||  std::is_same_v<ValueT, std::string_view>
    ) {
        QVariant value = settings.value(qkey, QVariant::fromValue(QByteArray::fromStdString(defaultValue)));
        if (ok) *ok = true;
        return value.value<QByteArray>().toStdString();
    } else {
        QVariant value = settings.value(qkey, QVariant::fromValue<ValueT>(defaultValue));
        if (ok) *ok = true;
        return value.value<ValueT>();
    }
#else
    // TODO: Implement
    if (ok) *ok = false;
    return {};
#endif
}


}
