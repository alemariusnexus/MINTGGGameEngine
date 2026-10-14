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
