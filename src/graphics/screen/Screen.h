#pragma once

#include "../../Globals.h"

#include "../surface/DrawSurface.h"


namespace MINTGGGameEngine
{

/**
 * \brief Abstract base class for screens that the engine can draw onto.
 *
 * Most of the abstract screen functionality is described in the DrawSurface class that this class inherits from. This
 * class only adds the following:
 *
 *  - init(): A method that is called once at startup to initialize the screen hardware.
 *  - commit(): A method to transfer the current frame data to the screen. If the screen uses double-buffering, this may
 *    cause the frame data to be transferred to the screen. If it doesn't, this may do nothing.
 *
 * Most screens that use double-buffering should not use this class directly, but inherit from BufferedScreen instead.
 *
 * \see BufferedScreen
 * \see DrawSurface
 */
class Screen : public DrawSurface
{
public:
    /**
     * \brief Initialize the screen.
     *
     * This should do any necessary interface initialization, bootup code, and other hardware-specific display setup.
     *
     * @return true if successful, false if anything went wrong (which will disable the screen).
     */
    virtual bool init() = 0;

    /**
     * \brief Shuts down the screen.
     */
    virtual void shutdown() = 0;

    /**
     * \brief Transfer any data to the actual screen if it hasn't been transferred yet.
     */
    virtual void commit() = 0;
};

}
