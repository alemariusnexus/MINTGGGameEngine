#pragma once

#ifndef     MGG_STORAGE_SDCARD_MOUNT_POINT
#   define  MGG_STORAGE_SDCARD_MOUNT_POINT      "/sdcard"
#endif

#ifndef     MGG_STORAGE_INTERNAL_MOUNT_POINT
#   define  MGG_STORAGE_INTERNAL_MOUNT_POINT    "/data"
#endif

#ifndef     MGG_STORAGE_SDCARD_SPI_BUS
#   define  MGG_STORAGE_SDCARD_SPI_BUS          MGG_BUS_SPI_GP0
#endif

#ifndef     MGG_STORAGE_SDCARD_PIN_MISO
#   define  MGG_STORAGE_SDCARD_PIN_MISO         -1
#endif

#ifndef     MGG_STORAGE_SDCARD_PIN_MOSI
#   define  MGG_STORAGE_SDCARD_PIN_MOSI         -1
#endif

#ifndef     MGG_STORAGE_SDCARD_PIN_SCK
#   define  MGG_STORAGE_SDCARD_PIN_SCK          -1
#endif

#ifndef     MGG_STORAGE_SDCARD_PIN_CS
#   ifdef MINTGGGAMEENGINE_PORT_DESKTOP
        // For desktop, just use anything positive to enable the SD card. We don't care about pin numbers otherwise.
#       define  MGG_STORAGE_SDCARD_PIN_CS       0
#   else
#       define  MGG_STORAGE_SDCARD_PIN_CS       -1
#endif
#endif
