set(srcs "main.c"
        "gatt_svr.c"
        "misc.c"
        "scli.c"
        "rainbow.c"
        "SX1231.c"
        )

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")
