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
#include <soc/gpio_struct.h>

#include "ble.h"
#include "sdcard.h"
#include "motor.h"
#include "can.h"
#include "SX1231.h"

static void halt();

static const char *TAG = "CONEPROJ";

#define PIN_NUM_MISO           37
#define PIN_NUM_MOSI           35
#define PIN_NUM_CLK            36

#define PIN_NUM_RADIO_CS       9
#define PIN_NUM_RADIO_RESET    2
#define PIN_NUM_RADIO_IRQ      1

#define PIN_NUM_RP2040_EN 29
#define PIN_NUM_RP2040_CSN 13

SX1231_t* sx1231;

esp_err_t rp2040_reset()
{
  gpio_set_direction(PIN_NUM_RP2040_EN, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_NUM_RP2040_EN, 0);
  vTaskDelay(10 / portTICK_PERIOD_MS);

  gpio_set_direction(PIN_NUM_RP2040_CSN, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_NUM_RP2040_CSN, 1);

//   while(true) {
//     gpio_set_level(PIN_NUM_RP2040_CSN, 0);
//     vTaskDelay(250 / portTICK_PERIOD_MS);
//     gpio_set_level(PIN_NUM_RP2040_CSN, 1);
//     vTaskDelay(250 / portTICK_PERIOD_MS);
//   }


  esp_err_t err = ESP_OK;
  spi_device_handle_t spi;

  spi_device_interface_config_t devcfg={
    .clock_speed_hz = APB_CLK_FREQ/800,
    // .clock_speed_hz = SPI_MASTER_FREQ_10M,
    .mode = 0,  //SPI mode 0
    // .spics_io_num = PIN_NUM_RP2040_CSN,
    .spics_io_num = -1,
    .queue_size = 8,
    // .command_bits = 8,
    .input_delay_ns=10,
    .flags = SPI_DEVICE_NO_DUMMY
  };
  ESP_LOGI(TAG, "add device");
//Attach the pico to the SPI bus
  err = spi_bus_add_device(SPI3_HOST, &devcfg, &spi);
  ESP_LOGI(TAG, "add device ok");

  if (err != ESP_OK)  {
    ESP_LOGE(TAG, "Could not create SPI device");
    ESP_ERROR_CHECK(err);
    return err;
  }

//   gpio_set_level(PIN_NUM_RP2040_EN, 1);
//   vTaskDelay(10 / portTICK_PERIOD_MS);

  uint8_t buffer[10];
  memset(buffer, 0, 10);

    while(true) {
        gpio_set_level(PIN_NUM_RP2040_CSN, 0);
        ESP_LOGI(TAG, "select");
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        err = spi_device_acquire_bus(spi, portMAX_DELAY);
        ESP_ERROR_CHECK(err);
        spi_transaction_t t = {
            // .cmd = 0x69,
            .length = 8,
            .flags = SPI_TRANS_USE_TXDATA,
            .tx_data = {0xab},
            .rx_buffer = &buffer,
            .user = NULL,
        };
        err = spi_device_polling_transmit(spi, &t);
        ESP_ERROR_CHECK(err);
        ESP_LOGI(TAG, "SPI XFER %d", buffer[0]);

        vTaskDelay(1000 / portTICK_PERIOD_MS);

        spi_device_release_bus(spi);
        gpio_set_level(PIN_NUM_RP2040_CSN, 1);
        ESP_LOGI(TAG, "unselect\n");

        vTaskDelay(5000 / portTICK_PERIOD_MS);

    }


  return err;
}

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
    esp_err_t ret = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);

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
        .host = SPI3_HOST
    };
    esp_err_t err = sx1231_initialize(&cfg,  &sx1231);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize radio (%s)", esp_err_to_name(err));
        halt();
    }
    ESP_LOGI(TAG, "Initalized radio");
}

static void spi_deinit()
{
    // causes panic for some reason
    // spi_bus_free(HSPI_HOST);
}

void radio_deinit()
{
}

static void spiffs_init()
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
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");


    
    spi_init();
    // sdcard_init();
    radio_init();
    if(rp2040_reset() != ESP_OK) {
        ESP_LOGE(TAG, "RP2040 reset fail");
        halt();
    }
    // ESP_LOGI(TAG, "RP2040 reset");
    // motor_init();
    // twai_init();
    // spiffs_init();
    // ble_init();

    // xTaskCreate(mainTask, "mainTask", 0x10000, NULL, 5, NULL);

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