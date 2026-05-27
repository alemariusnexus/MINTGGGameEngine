#pragma once

#include "../Globals.h"

#include <string>

#include "util/Log.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#include <hal/spi_types.h>
#include <nvs_flash.h>
#endif

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
//#include <Preferences.h>
#include <SD.h>
#include <SPI.h>
#endif


namespace MINTGGGameEngine
{

class Game;


class StorageEngine
{
public:
    StorageEngine();

    bool begin(Game& game);

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    bool mountSDCard (
        const char* mountPoint = "/sdcard",
        spi_host_device_t spiHost = SPI2_HOST,
        gpionum_t csPin = -1,
        uint32_t clkFreq = 20000000
        );

    bool mountSPIFFS (
        const char* mountPoint = "/spiffs"
        );
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    bool mountSDCard (
        const char* mountPoint = "/sdcard",
        SPIClass& spi = SPI,
        gpionum_t csPin = -1
        );

    bool checkSDFilePath(const std::string& path, std::string* outRelPath);
#endif

    template <typename ValueT>
    bool writeValue(const std::string_view& key, ValueT value);

    template <typename ValueT>
    ValueT readValue(const std::string_view& key, ValueT defaultValue = {}, bool* ok = nullptr);

    bool hasValue(const std::string_view& key);

private:
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    bool commitNVS();
#endif

private:
    Game* game;
    std::string sdMountPath;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    nvs_handle_t nvsHandle;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)

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
#else
    // TODO: Implement
    if (ok) *ok = false;
    return {};
#endif
}


}
