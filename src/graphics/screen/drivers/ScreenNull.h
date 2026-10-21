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

#include "../../../Globals.h"

#include "../BufferedScreen.h"


namespace MINTGGGameEngine
{

/**
 * \brief A dummy screen driver that does not display its content anywhere.
 *
 * This can be useful for headless applications that don't actually have a screen. It is also used when no screen is
 * found or when another screen's initialization fails for some reason.
 *
 * Although no actual external screen is used, the pixel data is still drawn properly to a framebuffer, and it can be
 * read back normally. This means that even such a null screen will take up memory for the framebuffer, which can
 * be avoided by making the screen a smaller size.
 */
class ScreenNull : public BufferedScreen
{
public:
    ScreenNull(uint16_t width, uint16_t height);
    ScreenNull();

    bool init() override;

    std::string getModelID() override { return "null"; }
};

}
