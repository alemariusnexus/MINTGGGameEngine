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

#include <string>

#include "Reader.h"


namespace MINTGGGameEngine
{


class BufferedReader : public Reader
{
public:
    BufferedReader(Reader& reader, void* buf, size_t bufSize);
    BufferedReader(Reader& reader, size_t bufSize);
    BufferedReader(Reader& reader);
    virtual ~BufferedReader();

    size_t read(void* out, size_t size) override;
    size_t skip(size_t size) override;
    bool seek(ssize_t offset, File::SeekMode mode = File::SeekSet) override;
    ssize_t tell() override;

private:
    size_t readFromBuffer(uint8_t* out, size_t size);
    size_t readDirect(uint8_t* out, size_t size);
    size_t readThroughBuffer(uint8_t* out, size_t size);

private:
    Reader* reader;

    uint8_t* buf;
    size_t bufSize;
    bool bufOwned;

    size_t bufOffset;
    size_t bufNumLeft;
};


}
