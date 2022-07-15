idf_component_register(SRCS "spect-config.c"
                    INCLUDE_DIRS "include"
                    REQUIRES spect-rgb-channel esp-idf-sqlite3 driver esp_system vfs fatfs)
