#pragma once

#include "../Globals.h"

#ifndef MINTGGGAMEENGINE_PORT_ARDUINO
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#include <esp_log.h>
#endif
#endif


namespace MINTGGGameEngine
{


#define LOG_USE_TAG(tag) [[maybe_unused]] const static char* TAG = tag;


enum LogLevel
{
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_VERBOSE
};


#if defined(MINTGGGAMEENGINE_PORT_ESPIDF)  &&  !defined(MINTGGGAMEENGINE_PORT_ARDUINO)

#define LogError(format, ...) ESP_LOGE(TAG, format, ## __VA_ARGS__)
#define LogWarning(format, ...) ESP_LOGW(TAG, format, ## __VA_ARGS__)
#define LogInfo(format, ...) ESP_LOGI(TAG, format, ## __VA_ARGS__)
#define LogDebug(format, ...) ESP_LOGD(TAG, format, ## __VA_ARGS__)
#define LogVerbose(format, ...) ESP_LOGV(TAG, format, ## __VA_ARGS__)

#else

bool LogMessageBegin(const char* tag, int level);

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
#   define LogPrintf(...) Serial.printf(__VA_ARGS__)
#   define LogPrintln() Serial.println()
#else
#   define LogPrintf(...) printf(__VA_ARGS__)
#   define LogPrintln() printf("\n")
#endif

#define LogMessage(tag, level, format, ...) do {        \
        if (LogMessageBegin((tag), (level))) {          \
            LogPrintf((format), ## __VA_ARGS__);        \
            LogPrintln();                               \
        }                                               \
    } while (false)

#define LogError(format, ...) LogMessage(TAG, LOG_LEVEL_ERROR, format, ## __VA_ARGS__)
#define LogWarning(format, ...) LogMessage(TAG, LOG_LEVEL_WARNING, format, ## __VA_ARGS__)
#define LogInfo(format, ...) LogMessage(TAG, LOG_LEVEL_INFO, format, ## __VA_ARGS__)
#define LogDebug(format, ...) LogMessage(TAG, LOG_LEVEL_DEBUG, format, ## __VA_ARGS__)
#define LogVerbose(format, ...) LogMessage(TAG, LOG_LEVEL_VERBOSE, format, ## __VA_ARGS__)

#endif

}
