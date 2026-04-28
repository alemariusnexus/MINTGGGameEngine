#include "Log.h"


namespace MINTGGGameEngine
{


#ifndef MINTGGGAMEENGINE_PORT_ESPIDF

bool LogMessageBegin(const char* tag, int level)
{
    const char* levelStr;
    switch (level) {
    case LOG_LEVEL_ERROR: levelStr = "ERR"; break;
    case LOG_LEVEL_WARNING: levelStr = "WRN"; break;
    case LOG_LEVEL_INFO: levelStr = "INF"; break;
    case LOG_LEVEL_DEBUG: levelStr = "DBG"; break;
    case LOG_LEVEL_VERBOSE: levelStr = "VRB"; break;
    }
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    Serial.print("[");
    Serial.print(levelStr);
    Serial.print("] ");
    Serial.print(tag);
    Serial.print(" - ");
#else
    printf("[%s] %s - ", levelStr, tag);
#endif
}

#endif


}
