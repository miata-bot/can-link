#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>

#include <esp_spiffs.h>
#include <esp_err.h>
#include <esp_log.h>

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <driver/twai.h>
#include <soc/gpio_struct.h>

static const char *TAG = "CONEPROJ";

void spi_init()
{   
    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1
    };

    //Initialize the SPI bus
    esp_err_t ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    switch (ret)
    {
        case ESP_OK:
            break;
        case ESP_FAIL:
            ESP_LOGE(TAG, "Failed to initialize HSPI Host");
            halt();
            break;
        case ESP_ERR_NOT_FOUND:
            ESP_LOGE(TAG, "Failed to initialize HSPI Host: not found");
            halt();
            break;
        default:
            ESP_LOGE(TAG, "Failed to initialize HSPI Host (%s)", esp_err_to_name(ret));
            halt();
    }
}

void spi_deinit()
{
    // spi_bus_free();
}

void sdcard_init()
{
    esp_err_t ret;
    ret = sdspi_host_init();
    sdspi_device_config_t config;
    sdspi_dev_handle_t handle;
    sdspi_host_init_device(config, handle);
}

void sdcard_deinit()
{
    sdspi_host_remove_device();
    sdspi_host_deinit();
}

void radio_init()
{
    // spi_bus_add_device
}

void radio_deinit()
{
    // spi_bus_remove_device()
}

void motor_init()
{
    
}

void motor_deinit()
{

}

void twai_init()
{
    //Initialize configuration structures using macro initializers
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    //Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("Driver installed\n");
    } else {
        printf("Failed to install driver\n");
        return;
    }

    //Start TWAI driver
    if (twai_start() == ESP_OK) {
        printf("Driver started\n");
    } else {
        printf("Failed to start driver\n");
        return;
    }
}

void twai_deinit()
{
    //Stop the TWAI driver
    if (twai_stop() == ESP_OK) {
        printf("Driver stopped\n");
    } else {
        printf("Failed to stop driver\n");
        return;
    }

    //Uninstall the TWAI driver
    if (twai_driver_uninstall() == ESP_OK) {
        printf("Driver uninstalled\n");
    } else {
        printf("Failed to uninstall driver\n");
        return;
    }
}

void ble_init()
{

}

void ble_deinit()
{
    
}

static void report(lua_State *L, int status)
{
    if (status == LUA_OK) return;

    const char *msg = lua_tostring(L, -1);
    printf("%s\n", msg);
    lua_pop(L, 1);
}

static void halt()
{
    ESP_LOGE(TAG, "System halted");
    while (1)
        vTaskDelay(1000);
}

static void mount_spiffs()
{
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/lua",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = false
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    switch (ret)
    {
        case ESP_OK:
            break;
        case ESP_FAIL:
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
            halt();
            break;
        case ESP_ERR_NOT_FOUND:
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
            halt();
            break;
        default:
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
            halt();
    }
    size_t total = 0, used = 0;
    ESP_ERROR_CHECK(esp_spiffs_info(NULL, &total, &used));
    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
}


void mainTask(void *arg)
{
    lua_State *L = luaL_newstate();
    ESP_ERROR_CHECK(L ? ESP_OK : ESP_FAIL);

    luaL_openlibs(L);

    int r = luaL_loadfilex(L, "/lua/main.lua", NULL);
    if (r != LUA_OK)
        printf("Failed to execute /lua/main.lua\n");
    else
        r = lua_pcall(L, 0, LUA_MULTRET, 0);

    report(L, r);
    lua_close(L);

    printf("State closed, heap: %d\n", xPortGetFreeHeapSize());

    while (1)
    {
        printf(".");
        fflush(stdout);
        vTaskDelay(100);
    }
    motor_deinit();
    radio_deinit();
    sdcard_deinit();
    spi_deinit();
    twai_deinit();
    ble_deinit();
}

void app_main()
{
    spi_init();
    sdcard_init();
    radio_init();
    motor_init();
    twai_init();
    ble_init();

    mount_spiffs();

    xTaskCreate(mainTask, "mainTask", 0x10000, NULL, 5, NULL);
}