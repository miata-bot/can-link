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
#include "esp_vfs_fat.h"
#include <esp_system.h>

#include "tusb.h"
#include "msc.h"
#include "hal/usb_hal.h"
#include "soc/usb_periph.h"
#include "esp32s2/rom/gpio.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_idf_version.h"
#include "esp_system.h"

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <soc/gpio_struct.h>

#include "ini.h"

#include "ble.h"
#include "can.h"
#include "console.h"
#include "motor.h"
#include "pico.h"
#include "regulator.h"
#include "SX1231.h"
#include "usb_acm.h"

#include "msc.h"
#include "hal/usb_hal.h"
#include "soc/usb_periph.h"
#include "util.h"

#include "pins.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
#include "esp_mac.h"
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#include "esp_private/periph_ctrl.h"
#else
#include "driver/periph_ctrl.h"
#endif

static const char *TAG = "CONEPROJ";

SX1231_t* sx1231;
pico_t* pico;

reg_t*   motor_reg;
motor_t* motor1;
motor_t* motor2;

static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
const char *base_path = "/flash";

typedef struct
{
    int version;
    uint8_t node_id;
    uint8_t network_id;

} configuration;

#define EPNUM_CDC       2
#define EPNUM_VENDOR    3
#define EPNUM_MSC       4

#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_VENDOR_DESC_LEN + TUD_MSC_DESC_LEN)

enum {
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_VENDOR,
    ITF_NUM_MSC,
    ITF_NUM_TOTAL
};

static const tusb_desc_device_t descriptor_config = {
    .bLength = sizeof(descriptor_config),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
#ifdef CFG_TUD_ENDPOINT0_SIZE
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
#else  // earlier versions have a typo in the name
    .bMaxPacketSize0 = CFG_TUD_ENDOINT0_SIZE,
#endif
    .idVendor = CONFIG_CONEPROJ_USB_VID,
    .idProduct = CONFIG_CONEPROJ_USB_PID,
    .bcdDevice = 0x100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

/*
    ESP usb builtin jtag subclass and protocol is 0xFF and 0x01 respectively.
    However, Tinyusb default values are 0x00.
    In order to use same protocol without tinyusb customization we are re-defining
    vendor descriptor here.
*/
// Interface number, string index, EP Out & IN address, EP size
#define TUD_VENDOR_EUB_DESCRIPTOR(_itfnum, _stridx, _epout, _epin, _epsize) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 2, TUSB_CLASS_VENDOR_SPECIFIC, 0xFF, 0x01, _stridx,\
  /* Endpoint Out */\
  7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_BULK, U16_TO_U8S_LE(_epsize), 0,\
  /* Endpoint In */\
  7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_BULK, U16_TO_U8S_LE(_epsize), 0

static uint8_t const desc_configuration[] = {
    // config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, 0x81, 8, EPNUM_CDC, 0x80 | EPNUM_CDC, TUD_OPT_HIGH_SPEED ? 512 : 64),

    // Interface number, string index, EP Out & IN address, EP size
    TUD_VENDOR_EUB_DESCRIPTOR(ITF_NUM_VENDOR, 5, EPNUM_VENDOR, 0x80 | EPNUM_VENDOR, 64),

    // Interface number, string index, EP Out & EP In address, EP size
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 6, EPNUM_MSC, 0x80 | EPNUM_MSC, 64),
};

#define MAC_BYTES       6

static char serial_descriptor[MAC_BYTES * 2 + 1] = {'\0'}; // 2 chars per hexnumber + '\0'

static char const *string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },   // 0: is supported language is English (0x0409)
    CONFIG_CONEPROJ_MANUFACTURER,    // 1: Manufacturer
    CONFIG_CONEPROJ_PRODUCT_NAME,    // 2: Product
    serial_descriptor,               // 3: Serials
    "CDC",
    "MSC",
};

static uint16_t _desc_str[32];

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    return desc_configuration;
}

uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *) &descriptor_config;
}

static void serial_number_init(void)
{
    uint8_t m[MAC_BYTES] = {0};
    esp_err_t ret = esp_efuse_mac_get_default(m);

    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "Cannot read MAC address and set the device serial number");
        eub_abort();
    }

    snprintf(serial_descriptor, sizeof(serial_descriptor),
             "%02X%02X%02X%02X%02X%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
}

uint16_t const *tud_descriptor_string_cb(const uint8_t index, const uint16_t langid)
{
    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        // Convert ASCII string into UTF-16

        if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) {
            return NULL;
        }

        const char *str = string_desc_arr[index];

        // Cap at max char
        chr_count = strlen(str);
        if (chr_count > 31) {
            chr_count = 31;
        }

        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2 * chr_count + 2);

    return _desc_str;
}

static void usb_configure_pins(usb_hal_context_t *usb)
{
    /* usb_periph_iopins currently configures USB_OTG as USB Device.
     * Introduce additional parameters in usb_hal_context_t when adding support
     * for USB Host.
     */
    for (const usb_iopin_dsc_t *iopin = usb_periph_iopins; iopin->pin != -1; ++iopin) {
        if ((usb->use_external_phy) || (iopin->ext_phy_only == 0)) {
            gpio_pad_select_gpio(iopin->pin);
            if (iopin->is_output) {
                gpio_matrix_out(iopin->pin, iopin->func, false, false);
            } else {
                gpio_matrix_in(iopin->pin, iopin->func, false);
                gpio_pad_input_enable(iopin->pin);
            }
            gpio_pad_unhold(iopin->pin);
        }
    }
    if (!usb->use_external_phy) {
        gpio_set_drive_capability(USBPHY_DM_NUM, GPIO_DRIVE_CAP_3);
        gpio_set_drive_capability(USBPHY_DP_NUM, GPIO_DRIVE_CAP_3);
    }
}

static void tusb_device_task(void *pvParameters)
{
    while (1) {
        tud_task();
    }
    vTaskDelete(NULL);
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
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    switch (ret)
    {
    case ESP_OK:
        break;
    case ESP_FAIL:
        ESP_LOGE(TAG, "Failed to initialize HSPI Host");
        eub_abort();
        break;
    case ESP_ERR_NOT_FOUND:
        ESP_LOGE(TAG, "Failed to initialize HSPI Host: not found");
        eub_abort();
        break;
    default:
        ESP_LOGE(TAG, "Failed to initialize HSPI Host (%s)", esp_err_to_name(ret));
        eub_abort();
    }
}
static void spi_deinit() {}

void radio_init(configuration* config)
{
    SX1231_config_t cfg = {
        .gpio_cs = PIN_NUM_RADIO_CS,
        .gpio_int = PIN_NUM_RADIO_IRQ,
        .gpio_reset = PIN_NUM_RADIO_RESET,
        .freqBand = RF69_915MHZ,
        .nodeID = config->node_id,
        .networkID = config->network_id,
        .isRFM69HW_HCW = true,
        .host = SPI2_HOST
    };
    esp_err_t err = sx1231_initialize(&cfg,  &sx1231);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize radio (%s)", esp_err_to_name(err));
        eub_abort();
    }
    ESP_LOGI(TAG, "Initalized radio");
}
static void radio_deinit() {}

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
        eub_abort();
    }
    ESP_LOGI(TAG, "Initalized pico");
    pico_ping(pico);
}

static void flash_init()
{
    ESP_LOGI(TAG, "Mounting FAT filesystem");
    // To mount device we need name of device partition, define base_path
    // and allow format partition in case if it is new one and was not formatted before
    const esp_vfs_fat_mount_config_t mount_config = {
        .max_files = 4,
        .format_if_mount_failed = true,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(base_path, "storage", &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        eub_abort();
    }
}

static void flash_deinit() {}

static void usb_init()
{
    periph_module_reset(PERIPH_USB_MODULE);
    periph_module_enable(PERIPH_USB_MODULE);

    usb_hal_context_t hal = {
        .use_external_phy = false
    };

    usb_hal_init(&hal);
    usb_configure_pins(&hal);

    if(!tusb_init()) {
        ESP_LOGE(TAG, "Failed to initialize USB");
        eub_abort();
    }

    xTaskCreate(tusb_device_task, "tusb_device_task", 4 * 1024, NULL, 5, NULL);
    xTaskCreate(msc_task, "msc_task", 4 * 1024, NULL, 5, NULL);
    acm_init();
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
        eub_abort();
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
    //     eub_abort();
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
        eub_abort();
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
        eub_abort();
    }
    ESP_LOGI(TAG, "initialized console");
}

static int lua_set_color(lua_State *L)
{
    double index = luaL_checknumber(L, 1);
    double r = luaL_checknumber(L, 2);
    double g = luaL_checknumber(L, 3);
    double b = luaL_checknumber(L, 4);
    ESP_LOGI(TAG, "lua_set_color(%d, %02X, %02X, %02x)", (uint8_t)index, (uint8_t)r, (uint8_t)g, (uint8_t)b);
    pico_set_color(pico, (uint8_t)index, (uint8_t)r, (uint8_t)g, (uint8_t)b);
    return 0;
}

static int lua_set_brightness(lua_State *L)
{
    double index = luaL_checknumber(L, 1);
    double brightness = luaL_checknumber(L, 2);
    ESP_LOGI(TAG, "lua_set_brightness(%d, %02X)", (uint8_t)index, (uint8_t)brightness);
    pico_set_brightness(pico, (uint8_t)index, (uint8_t)brightness);
    return 0;
}

static void report(lua_State *L, int status)
{
    if (status == LUA_OK)
        return;

    const char *msg = lua_tostring(L, -1);
    printf("%s\n", msg);
    lua_pop(L, 1);
}

static int config_ini_handler(void* user, const char* section, const char* name, const char* value)
{
    configuration* pconfig = (configuration*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("protocol", "version")) {
        pconfig->version = atoi(value);
    } else if (MATCH("node", "id")) {
        pconfig->node_id = atoi(value);
    } else if (MATCH("node", "network")) {
        pconfig->network_id = atoi(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
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

    serial_number_init();
    usb_init();

    configuration config;
    if (ini_parse("/flash/config.ini", config_ini_handler, &config) < 0) {
        ESP_LOGE(TAG, "Can't load 'config.ini'\n");
        eub_abort();
    }
    ESP_LOGI(TAG, "Config loaded from 'config.ini': version=%d nodeid=%d networkid=%d\n", config.version, config.node_id, config.network_id);

    spi_init();

    radio_init(&config);

    pico_init();
    motor_init();
    reg_init();
    ble_init();
    console_init();

    ESP_LOGI(TAG, "Preparing to run MAIN.LUA");
    vTaskDelay(pdMS_TO_TICKS(1500));
    pico_set_color(pico, COMMAND_RGB_INDEX_0, 0xff, 0, 0xff);
    pico_set_brightness(pico, COMMAND_RGB_INDEX_0, 0xff);

    lua_State *L = luaL_newstate();
    ESP_ERROR_CHECK(L ? ESP_OK : ESP_FAIL);

    luaL_openlibs(L);

    //Expose the rgb function to the lua environment
    lua_pushcfunction(L, lua_set_color);
    lua_setglobal(L, "set_color");

    lua_pushcfunction(L, lua_set_brightness);
    lua_setglobal(L, "set_brightness");

    int r = luaL_loadfilex(L, "/flash/main.lua", NULL);
    if (r != LUA_OK)
        printf("Failed to execute main.lua\n");
    else {
        r = lua_pcall(L, 0, LUA_MULTRET, 0);
    }

    report(L, r);
    lua_close(L);

    printf("State closed, heap: %d\n", xPortGetFreeHeapSize());

    while (1)
    {
        // if(sx1231_sendWithRetry(sx1231, 2, "ABCD", 4, 3, 10)) {
        //     ESP_LOGI("RADIO", "got ack");
        // }
        if(sx1231_receiveDone(sx1231)) {
            ESP_LOGI("RADIO", "SENDER=%d RSSI=%d dbm rx_data={%.*s}", sx1231->SENDERID, sx1231->RSSI, sx1231->DATALEN, sx1231->DATA);
            struct __attribute__((__packed__)) {
                uint8_t event;
                uint8_t senderID;
                uint8_t targetID;
                uint8_t ackReq;
                uint8_t ackRecv;
                int16_t rssi;
                uint8_t dataLength;
            }
            payload;

            payload.event      = 0x10;
            payload.senderID   = sx1231->SENDERID;
            payload.targetID   = sx1231->TARGETID;
            payload.ackReq     = sx1231->ACK_REQUESTED;
            payload.ackRecv    = sx1231->ACK_RECEIVED;
            payload.rssi       = sx1231->RSSI;
            payload.dataLength = sx1231->DATALEN;

            tinyusb_cdcacm_write_queue(0, (void*)&payload, 8);
            tinyusb_cdcacm_write_queue(0, sx1231->DATA, sx1231->DATALEN);
            tinyusb_cdcacm_write_flush(0, 0);
        }
        vTaskDelay(10);
    }

    // Deinit peripherals in reverse of initialization
    ble_deinit();
    flash_deinit();
    twai_deinit();
    motor_deinit();
    radio_deinit();
    spi_deinit();
    eub_abort();
}