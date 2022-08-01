idf_component_register(SRCS "spect-radio.c" "sx1231.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "esp_timer spect-config" "spect-rgb-channel")
