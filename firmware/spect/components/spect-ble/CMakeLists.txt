idf_component_register(SRCS "spect-ble.c" "gatt_svr.c" "misc.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "bt led_strip spect-config")
