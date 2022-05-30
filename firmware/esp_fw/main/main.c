#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>

#include <esp_chip_info.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_spi_flash.h>
#include <esp_spiffs.h>
#include <esp_system.h>

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <soc/gpio_struct.h>

#include "ble.h"
#include "can.h"
#include "console.h"
#include "motor.h"
#include "pico.h"
#include "regulator.h"
#include "sdcard.h"
#include "SX1231.h"

#include "pins.h"

static void halt();

static const char *TAG = "CONEPROJ";

SX1231_t* sx1231;
pico_t* pico;

reg_t*   motor_reg;
motor_t* motor1;
motor_t* motor2;

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
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

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

void radio_init()
{
    SX1231_config_t cfg = {
        .gpio_cs = PIN_NUM_RADIO_CS,
        .gpio_int = PIN_NUM_RADIO_IRQ,
        .gpio_reset = PIN_NUM_RADIO_RESET,
        .freqBand = RF69_915MHZ,
        .nodeID = 5, 
        .networkID = 100,
        .isRFM69HW_HCW = true,
        .host = SPI2_HOST
    };
    esp_err_t err = sx1231_initialize(&cfg,  &sx1231);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize radio (%s)", esp_err_to_name(err));
        halt();
    }
    ESP_LOGI(TAG, "Initalized radio");
}

static void pico_init()
{
    pico_config_t cfg = {
        .gpio_en = PIN_NUM_RP2040_EN,
        .gpio_cs = PIN_NUM_RP2040_CSN,
        .host = SPI2_HOST
    };
    esp_err_t err = pico_initialize(&cfg, &pico);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize pico (%s)", esp_err_to_name(err));
        halt();
    }
    ESP_LOGI(TAG, "Initalized pico");
    pico_ping(pico);
}

static void spi_deinit()
{
    // causes panic for some reason
    // spi_bus_free(HSPI_HOST);
}

void radio_deinit()
{
}

static void flash_init()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
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

static void spiffs_deinit()
{
}

void motor_init()
{
    esp_err_t err;
    motor_config_t cfg1 = {
        .gpio_enA = PIN_NUM_MCPWM_EN1,
        .gpio_enB = PIN_NUM_MCPWM_EN2,

        .gpio_inA = PIN_NUM_MCPWM_IN1,
        .gpio_inB = PIN_NUM_MCPWM_IN2,

        .unit = BDC_MCPWM_UNIT,
        .timer = BDC_MCPWM_TIMER,
        .frequency = BDC_MCPWM_FREQ_HZ
    };
    err = motor_initialize(&cfg1, &motor1);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize motor1");
        halt();
    }
    ESP_LOGI(TAG, "initialized motor 1");

    // motor_config_t cfg2 = {
    //     .gpio_enA = PIN_NUM_MCPWM_EN1,
    //     .gpio_inA = PIN_NUM_MCPWM_IN1,
    //     .gpio_enB = PIN_NUM_MCPWM_EN2,
    //     .gpio_inB = PIN_NUM_MCPWM_IN2,
    //     .unit = 0,
    //     .timer = 0,
    //     .frequency = 15000
    // };
    // err = motor_initialize(&cfg2, &motor2);
    // if(err != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to initialize motor1");
    //     halt();
    // }
    // ESP_LOGI(TAG, "initialized motor 2");
}

void reg_init()
{
    reg_config_t config = {
        .gpio_enable  = PIN_NUM_MCPWM_REG_EN,
        .enable_value = REG_HIGH
    };
    esp_err_t err = reg_initialize(&config, &motor_reg);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize motor regulator");
        halt();
    }
    ESP_LOGI(TAG, "initialized motor regulator");
    // reg_disable(motor_reg);
    reg_enable(motor_reg);
}

void console_init()
{
    esp_err_t err = console_initialize();
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize console");
        halt();
    }
    ESP_LOGI(TAG, "initialized console");
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

void app_main()
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "This is %s chip with %d CPU core(s), WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    flash_init();
    spi_init();
    sdcard_init();
    radio_init();
    pico_init();
    motor_init();
    reg_init();
    ble_init();
    console_init();

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
        // sx1231_send(sx1231, 2, "test", 4, false);
        if(sx1231_sendWithRetry(sx1231, 2, "ABCD", 4, 3, 10)) {
            ESP_LOGI("RADIO", "got ack");
        }
        // printf(".");
        fflush(stdout);
        vTaskDelay(100);
        if(sx1231_receiveDone(sx1231)) {
            ESP_LOGI("RADIO", "SENDER=%d RSSI=%d dbm rx_data={%.*s}", sx1231->SENDERID, sx1231->RSSI, sx1231->DATALEN, sx1231->DATA);
        }
        // sx1231_receiveDone(sx1231);
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