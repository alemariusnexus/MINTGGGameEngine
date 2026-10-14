#pragma once

#include "ConfigDefaults.h"


#ifndef MGG_SCREEN_MAIN_TYPE
#   define MGG_SCREEN_MAIN_TYPE         MGG_SCREEN_TYPE_NULL
#endif


#if MGG_SCREEN_MAIN_TYPE == MGG_SCREEN_TYPE_NULL


#   ifndef      MGG_SCREEN_MAIN_WIDTH
#       define  MGG_SCREEN_MAIN_WIDTH        160
#   endif

#   ifndef      MGG_SCREEN_MAIN_HEIGHT
#       define  MGG_SCREEN_MAIN_HEIGHT       128
#   endif


#elif MGG_SCREEN_MAIN_TYPE == MGG_SCREEN_TYPE_ST7735_SPI

#   ifndef      MGG_SCREEN_MAIN_WIDTH
#       define  MGG_SCREEN_MAIN_WIDTH       160
#   endif

#   ifndef      MGG_SCREEN_MAIN_HEIGHT
#       define  MGG_SCREEN_MAIN_HEIGHT      128
#   endif

#   ifndef      MGG_SCREEN_MAIN_OFFSET_X
#       define  MGG_SCREEN_MAIN_OFFSET_X    0
#   endif

#   ifndef      MGG_SCREEN_MAIN_OFFSET_Y
#       define  MGG_SCREEN_MAIN_OFFSET_Y    0
#   endif

#   ifndef      MGG_SCREEN_MAIN_MIRROR_X
#       define  MGG_SCREEN_MAIN_MIRROR_X    1
#   endif

#   ifndef      MGG_SCREEN_MAIN_MIRROR_Y
#       define  MGG_SCREEN_MAIN_MIRROR_Y    0
#   endif

#   ifndef      MGG_SCREEN_MAIN_SWAP_XY
#       define  MGG_SCREEN_MAIN_SWAP_XY     1
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIXEL_FORMAT
#       define  MGG_SCREEN_MAIN_PIXEL_FORMAT    PixelFormat::RGB565
#   endif

#   ifndef      MGG_SCREEN_MAIN_SPI_CLOCK
#       define  MGG_SCREEN_MAIN_SPI_CLOCK   40000000
#   endif

#   ifndef      MGG_SCREEN_MAIN_SPI_BUS
#       define  MGG_SCREEN_MAIN_SPI_BUS     MGG_BUS_SPI_GP0
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_MISO
#       define  MGG_SCREEN_MAIN_PIN_MISO    -1
#   endif

#   ifndef MGG_SCREEN_MAIN_PIN_MOSI
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_MOSI
#   endif

#   ifndef MGG_SCREEN_MAIN_PIN_SCK
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_SCK
#   endif

#   ifndef MGG_SCREEN_MAIN_PIN_CS
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_CS
#   endif

#   ifndef MGG_SCREEN_MAIN_PIN_DC
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_DC
#   endif

#   ifndef MGG_SCREEN_MAIN_PIN_RST
#       define MGG_SCREEN_MAIN_PIN_RST      -1
#   endif


#elif MGG_SCREEN_MAIN_TYPE == MGG_SCREEN_TYPE_ILI9341_8080


#   ifndef      MGG_SCREEN_MAIN_WIDTH
#       define  MGG_SCREEN_MAIN_WIDTH       320
#   endif

#   ifndef      MGG_SCREEN_MAIN_HEIGHT
#       define  MGG_SCREEN_MAIN_HEIGHT      240
#   endif

#   ifndef      MGG_SCREEN_MAIN_OFFSET_X
#       define  MGG_SCREEN_MAIN_OFFSET_X    0
#   endif

#   ifndef      MGG_SCREEN_MAIN_OFFSET_Y
#       define  MGG_SCREEN_MAIN_OFFSET_Y    0
#   endif

#   ifndef      MGG_SCREEN_MAIN_MIRROR_X
#       define  MGG_SCREEN_MAIN_MIRROR_X    0
#   endif

#   ifndef      MGG_SCREEN_MAIN_MIRROR_Y
#       define  MGG_SCREEN_MAIN_MIRROR_Y    0
#   endif

#   ifndef      MGG_SCREEN_MAIN_SWAP_XY
#       define  MGG_SCREEN_MAIN_SWAP_XY     1
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIXEL_FORMAT
#       define  MGG_SCREEN_MAIN_PIXEL_FORMAT    PixelFormat::RGB565
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIXEL_CLOCK
#       define  MGG_SCREEN_MAIN_PIXEL_CLOCK       10000000
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_CS
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_CS
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_DC
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_DC
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_RST
#       define  MGG_SCREEN_MAIN_PIN_RST     -1
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_WR
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_WR
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_RD
#       define  MGG_SCREEN_MAIN_PIN_RD      -1
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_DAT0
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_DAT0
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_DAT1
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_DAT1
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_DAT2
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_DAT2
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_DAT3
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_DAT3
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_DAT4
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_DAT4
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_DAT5
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_DAT5
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_DAT6
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_DAT6
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIN_DAT7
#       error Missing config option in mgg_config.h: MGG_SCREEN_MAIN_PIN_DAT7
#   endif


#elif MGG_SCREEN_MAIN_TYPE == MGG_SCREEN_TYPE_RPI_TC358762


#   ifndef      MGG_SCREEN_MAIN_WIDTH
#       define  MGG_SCREEN_MAIN_WIDTH       800
#   endif

#   ifndef      MGG_SCREEN_MAIN_HEIGHT
#       define  MGG_SCREEN_MAIN_HEIGHT      480
#   endif

#   ifndef      MGG_SCREEN_MAIN_PIXEL_FORMAT
#       define  MGG_SCREEN_MAIN_PIXEL_FORMAT    PixelFormat::RGB565
#   endif

#   ifndef      MGG_SCREEN_MAIN_MIRROR_X
#       define  MGG_SCREEN_MAIN_MIRROR_X        0
#   endif

#   ifndef      MGG_SCREEN_MAIN_MIRROR_Y
#       define  MGG_SCREEN_MAIN_MIRROR_Y        0
#   endif

#   ifndef      MGG_SCREEN_MAIN_SWAP_XY
#       define  MGG_SCREEN_MAIN_SWAP_XY         0
#   endif


#else

#   error Invalid value for MGG_SCREEN_MAIN_TYPE!

#endif
