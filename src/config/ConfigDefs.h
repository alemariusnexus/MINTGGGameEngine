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

// ********** BUILD **********

#define MGG_STARTUP_CODE_TYPE_NONE          0
#define MGG_STARTUP_CODE_TYPE_PLATFORM      1
#define MGG_STARTUP_CODE_TYPE_FULL          1


// ********** BUS **********

#define MGG_BUS_I2C_GP0     0
#define MGG_BUS_I2C_GP1     1
#define MGG_BUS_I2C_GP2     2
#define MGG_BUS_I2C_GP3     3

#define MGG_BUS_SPI_GP0     0
#define MGG_BUS_SPI_GP1     1
#define MGG_BUS_SPI_GP2     2
#define MGG_BUS_SPI_GP3     3


// ********** SCREEN **********

#define MGG_SCREEN_TYPE_NULL            0
#define MGG_SCREEN_TYPE_ST7735_SPI      1
#define MGG_SCREEN_TYPE_ILI9341_8080    2
#define MGG_SCREEN_TYPE_RPI_TC358762    3
