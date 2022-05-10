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
#include <esp_chip_info.h>
#include <esp_spi_flash.h>
#include <esp_system.h>

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <driver/twai.h>
#include <soc/gpio_struct.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>

#include "ble.h"

static void halt();

static const char *TAG = "CONEPROJ";

#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK 14

// SD Config
#define PIN_NUM_SD_CS 15
const char SD_MOUNT_POINT[] = "/sdcard";
sdmmc_card_t SDCARD;

static void spi_init()
{
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    // Initialize the SPI bus in HSPI mode. DMA channel might need changing later?
    esp_err_t ret = spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);

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

static void spi_deinit()
{
    // causes panic for some reason
    // spi_bus_free(HSPI_HOST);
}

static void sdcard_init()
{
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    ESP_LOGI(TAG, "Mounting SD filesystem");

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_SD_CS;
    slot_config.host_id = host.slot;
    sdmmc_card_t *card = &SDCARD;

    // Mount the filesystem
    esp_err_t ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    switch (ret)
    {
    case ESP_OK:
        ESP_LOGI(TAG, "Filesystem mounted");
        break;
    case ESP_FAIL:
        ESP_LOGE(TAG, "Failed to mount filesystem.");
        return;
    default:
        ESP_LOGE(TAG, "Failed to initialize the card (%s). ", esp_err_to_name(ret));
        return;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
}

static void sdcard_deinit()
{
    // unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, &SDCARD);
}

static void radio_init()
{
    // spi_bus_add_device
}

static void radio_deinit()
{
    // spi_bus_remove_device()
}

static void motor_init()
{
}

static void motor_deinit()
{
}

static void twai_init()
{
    // //Initialize configuration structures using macro initializers
    // twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
    // twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    // twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // //Install TWAI driver
    // if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    //     printf("Driver installed\n");
    // } else {
    //     printf("Failed to install driver\n");
    //     return;
    // }

    // //Start TWAI driver
    // if (twai_start() == ESP_OK) {
    //     printf("Driver started\n");
    // } else {
    //     printf("Failed to start driver\n");
    //     return;
    // }
}

static void twai_deinit()
{
    // //Stop the TWAI driver
    // if (twai_stop() == ESP_OK) {
    //     printf("Driver stopped\n");
    // } else {
    //     printf("Failed to stop driver\n");
    //     return;
    // }

    // //Uninstall the TWAI driver
    // if (twai_driver_uninstall() == ESP_OK) {
    //     printf("Driver uninstalled\n");
    // } else {
    //     printf("Failed to uninstall driver\n");
    //     return;
    // }
}

static void spiffs_init()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};

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
static void spiffs_deinit()
{
}



static void report(lua_State *L, int status)
{
    if (status == LUA_OK)
        return;

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

void mainTask(void *arg)
{
    lua_State *L = luaL_newstate();
    ESP_ERROR_CHECK(L ? ESP_OK : ESP_FAIL);

    luaL_openlibs(L);

    int r = luaL_loadfilex(L, "/sdcard/main.lua", NULL);
    if (r != LUA_OK)
        printf("Failed to execute main.lua\n");
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

    // Deinit peripherals in reverse of initialization
    ble_deinit();
    spiffs_deinit();
    twai_deinit();
    motor_deinit();
    radio_deinit();
    sdcard_deinit();
    spi_deinit();
    halt();
}

void app_main()
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    spi_init();
    sdcard_init();
    radio_init();
    motor_init();
    twai_init();
    spiffs_init();
    ble_init();

    xTaskCreate(mainTask, "mainTask", 0x10000, NULL, 5, NULL);
}