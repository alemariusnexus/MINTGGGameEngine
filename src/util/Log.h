#pragma once

#include "../Globals.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
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

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF

#define LogError(format, ...) ESP_LOGE(TAG, format, ## __VA_ARGS__)
#define LogWarning(format, ...) ESP_LOGW(TAG, format, ## __VA_ARGS__)
#define LogInfo(format, ...) ESP_LOGI(TAG, format, ## __VA_ARGS__)
#define LogDebug(format, ...) ESP_LOGD(TAG, format, ## __VA_ARGS__)
#define LogVerbose(format, ...) ESP_LOGV(TAG, format, ## __VA_ARGS__)

#else

bool LogMessageBegin(const char* tag, int level);

#define LogMessage(tag, level, format, ...) do {        \
        if (LogMessageBegin((tag), (level))) {          \
            Serial.printf((format), ## __VA_ARGS__);    \
            Serial.println();                           \
        }                                               \
    } while (false)

#define LogError(format, ...) LogMessage(TAG, LOG_LEVEL_ERROR, format, ## __VA_ARGS__)
#define LogWarning(format, ...) LogMessage(TAG, LOG_LEVEL_WARNING, format, ## __VA_ARGS__)
#define LogInfo(format, ...) LogMessage(TAG, LOG_LEVEL_INFO, format, ## __VA_ARGS__)
#define LogDebug(format, ...) LogMessage(TAG, LOG_LEVEL_DEBUG, format, ## __VA_ARGS__)
#define LogVerbose(format, ...) LogMessage(TAG, LOG_LEVEL_VERBOSE, format, ## __VA_ARGS__)

#endif

}
