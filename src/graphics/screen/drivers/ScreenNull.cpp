#include "ScreenNull.h"


namespace MINTGGGameEngine
{


ScreenNull::ScreenNull(uint16_t width, uint16_t height)
    : BufferedScreen (
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
        static_cast<uint8_t*>(heap_caps_malloc(width * height * sizeof(uint16_t), MALLOC_CAP_32BIT | MALLOC_CAP_8BIT)),
#else
        static_cast<uint8_t*>(malloc(width * height * sizeof(uint16_t))),
#endif
        width,
        height,
        &ScreenNull::_defaultDelete
        )
{
}


}
