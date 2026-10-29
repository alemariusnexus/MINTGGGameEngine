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

#ifndef     MGG_STORAGE_SDCARD_MOUNT_POINT
#   define  MGG_STORAGE_SDCARD_MOUNT_POINT      "/sdcard"
#endif

#ifndef     MGG_STORAGE_INTERNAL_MOUNT_POINT
#   define  MGG_STORAGE_INTERNAL_MOUNT_POINT    "/data"
#endif

#ifndef     MGG_STORAGE_SDCARD_SPI_BUS
#   define  MGG_STORAGE_SDCARD_SPI_BUS          MGG_BUS_SPI_DEFAULT
#endif

#ifndef     MGG_STORAGE_SDCARD_PIN_CS
#   ifdef MINTGGGAMEENGINE_PORT_DESKTOP
        // For desktop, just use anything positive to enable the SD card. We don't care about pin numbers otherwise.
#       define  MGG_STORAGE_SDCARD_PIN_CS       0
#   else
#       define  MGG_STORAGE_SDCARD_PIN_CS       -1
#endif
#endif
