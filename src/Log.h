#pragma once

#include "Globals.h"

#include <string>

#ifdef ESP_PLATFORM
#include <esp_log.h>
#endif


namespace MINTGGGameEngine
{


#define LOG_USE_TAG(tag) const static char* TAG = tag;


enum LogLevel
{
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_VERBOSE
};

#ifdef ESP_PLATFORM

#define LogError(tag, format, ...) ESP_LOGE(tag, format, ## __VA_ARGS__)
#define LogWarning(tag, format, ...) ESP_LOGW(tag, format, ## __VA_ARGS__)
#define LogInfo(tag, format, ...) ESP_LOGI(tag, format, ## __VA_ARGS__)
#define LogDebug(tag, format, ...) ESP_LOGD(tag, format, ## __VA_ARGS__)
#define LogVerbose(tag, format, ...) ESP_LOGV(tag, format, ## __VA_ARGS__)

#else

bool LogMessageBegin(const char* tag, int level);

void LogMessage(const char* tag, int level, const char* format, ...);

#define LogMessage(tag, level, format, ...) do {        \
        if (LogMessageBegin((tag), (level))) {          \
            Serial.printf((format), __VA_ARGS__);       \
            Serial.println();                           \
        }                                               \
    } while (false)

#define LogError(tag, format, ...) LogMessage((tag), LOG_LEVEL_ERROR, ## __VA_ARGS__)
#define LogWarning(tag, format, ...) LogMessage((tag), LOG_LEVEL_WARNING, ## __VA_ARGS__)
#define LogInfo(tag, format, ...) LogMessage((tag), LOG_LEVEL_INFO, ## __VA_ARGS__)
#define LogDebug(tag, format, ...) LogMessage((tag), LOG_LEVEL_DEBUG, ## __VA_ARGS__)
#define LogVerbose(tag, format, ...) LogMessage((tag), LOG_LEVEL_VERBOSE, ## __VA_ARGS__)

#endif

}
