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

#include "ScreenNull.h"

#include "../../../platform/PlatformManager.h"
#include "../../../util/Util.h"

LOG_USE_TAG("ScreenNull")


namespace MINTGGGameEngine
{


ScreenNull::ScreenNull(uint16_t width, uint16_t height)
    : BufferedScreen (
        PixelFormat::RGB565,
        ByteOrder::Native,
        AdvancedAlignedAlloc(16, width * height * sizeof(uint16_t),
            ADVANCED_ALLOC_FLAG_UNALIGNED_ACCESS | ADVANCED_ALLOC_FLAG_DMA_ACCESSIBLE
                | (PlatformManager::getInstance().hasExternalRAM()
                    ? ADVANCED_ALLOC_FLAG_EXTERNAL_RAM
                    : ADVANCED_ALLOC_FLAG_INTERNAL_RAM)),
        width,
        height,
        &AdvancedFree
        )
{
}

ScreenNull::ScreenNull()
    : ScreenNull (
#ifdef MGG_SCREEN_MAIN_WIDTH
        MGG_SCREEN_MAIN_WIDTH,
#else
        160,
#endif
#ifdef MGG_SCREEN_MAIN_HEIGHT
        MGG_SCREEN_MAIN_HEIGHT
#else
        128
#endif
        )
{
}

bool ScreenNull::init()
{
    LogWarning (
        "Initializing null screen (%ux%u). This is a dummy screen, so no image will be visible!",
        getWidth(), getHeight()
        );

    return BufferedScreen::init();
}


}
