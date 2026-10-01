#pragma once

#include "../Globals.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <driver/spi_master.h>
#endif


namespace MINTGGGameEngine
{

class Engine
{
public:


#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    virtual spi_host_device_t getESPSPIHostDevice() const = 0;
    virtual size_t getESPSPIMaxTransferSize() const = 0;
#endif
};

}
