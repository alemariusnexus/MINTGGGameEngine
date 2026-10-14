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

#include "../Globals.h"

#include "File.h"


namespace MINTGGGameEngine
{


class Reader
{
public:
    virtual size_t read(void* out, size_t size) = 0;
    virtual size_t skip(size_t size) = 0;
    virtual bool seek(ssize_t offset, File::SeekMode mode = File::SeekSet) = 0;
    virtual ssize_t tell() = 0;
};


}