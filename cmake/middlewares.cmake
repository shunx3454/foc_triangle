file(GLOB_RECURSE LVGL_SRC )

file(GLOB MIDDLEWARES_SRC
    # FreeRTOS
    middlewares/freertos/source/croutine.c
    middlewares/freertos/source/event_groups.c
    middlewares/freertos/source/list.c
    middlewares/freertos/source/queue.c
    middlewares/freertos/source/stream_buffer.c
    middlewares/freertos/source/tasks.c
    middlewares/freertos/source/timers.c
    middlewares/freertos/source/portable/GCC/ARM_CM4F/port.c
    middlewares/freertos/source/portable/memmang/heap_4.c
    # CherryUSB 
    # middlewares/cherryusb/core/usbd_core.c
    # middlewares/cherryusb/port/dwc2/usb_dc_dwc2.c
    # middlewares/cherryusb/port/dwc2/usb_glue_at.c
    # middlewares/cherryusb/class/cdc/usbd_cdc_acm.c
    # middlewares/cherryusb/class/audio/usbd_audio.c
    # middlewares/cherryusb/class/msc/usbd_msc.c
    # middlewares/cherryusb/osal/usb_osal_freertos.c
    # SFUD
    #middlewares/sfud/src/*.c
    #middlewares/sfud/port/*.c
    # libmad
    #middlewares/libmad/*.c
    # AT32 i2c application
    middlewares/i2c_application_library/*.c
    # fatfs
    #middlewares/fatfs/*.c
    # Mahony filter
    middlewares/MahonyAHRS/src/MahonyAHRS.cpp
)

set(MIDDLEWARES_SRC ${MIDDLEWARES_SRC} ${LVGL_SRC})

set(MIDDLEWARES_INC
    # FreeRTOS
    middlewares/freertos/source/include
    middlewares/freertos/source/portable/GCC/ARM_CM4F
    # CherryUSB
    #middlewares/cherryusb/core
    #middlewares/cherryusb/common
    #middlewares/cherryusb/osal
    #middlewares/cherryusb/port/dwc2
    #middlewares/cherryusb/class/audio
    #middlewares/cherryusb/class/cdc
    #middlewares/cherryusb/class/msc
    # SFUD
    #middlewares/sfud/inc
    # libmad
    #middlewares/libmad
    #middlewares/libmad/msvc++
    # AT32 i2c application
    middlewares/i2c_application_library
    # fatfs
    #middlewares/fatfs/
    # Mahony filter
    middlewares/MahonyAHRS/src/
)