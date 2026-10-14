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

#include "File.h"
#include "Reader.h"


namespace MINTGGGameEngine
{


class FileReader : public Reader
{
public:
    FileReader(const File& file);
    FileReader(const std::string_view& path);
    virtual ~FileReader();

    bool open(const char** outErrmsg = nullptr);
    void close();
    bool isOpen() const;

    size_t read(void* out, size_t size) override;
    size_t skip(size_t size) override;
    bool seek(ssize_t offset, File::SeekMode mode = File::SeekSet) override;
    ssize_t tell() override;

private:
    File file;
};


}