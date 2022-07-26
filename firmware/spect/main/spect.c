#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "tusb.h"
#include "msc.h"
#include "hal/usb_hal.h"
#include "soc/usb_periph.h"

#include "esp_chip_info.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "driver/spi_master.h"

#include "esp32s2/rom/gpio.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_idf_version.h"
#include "esp_system.h"

#include "pins.h"
#include "util.h"
#include "usb_acm.h"

#include "filesystem.h"
#include "spect-config.h"
#include "spect-ble.h"

#include "spect-rgb-channel.h"
#include "rainbow.h"

#include "spect-radio.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
#include "esp_mac.h"
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#include "esp_private/periph_ctrl.h"
#else
#include "driver/periph_ctrl.h"
#endif

static const char *TAG = "SPECT";

spect_config_context_t* config_ctx;
spect_rgb_t* channel0;

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
    while (1) tud_task();
    vTaskDelete(NULL);
}


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

void app_main(void)
{
  ESP_LOGI(TAG, "app boot");
  ESP_LOGI(TAG, "free memory=%d", esp_get_minimum_free_heap_size());

  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  serial_number_init();
  usb_init();

  esp_err_t err;

  ESP_LOGI(TAG, "SPI init");
  spi_bus_config_t buscfg = {
      .miso_io_num = GPIO_NUM_SPI2_MISO,
      .mosi_io_num = GPIO_NUM_SPI2_MOSI,
      .sclk_io_num = GPIO_NUM_SPI2_SCLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 4000,
  };

  // Initialize the SPI bus in HSPI mode. DMA channel might need changing later?
  err = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "SPI init ok");

  /* flash partition is formatted as fat32. 
   * It is NOT encrypted. sorry. if someone really wants this
   * i'll make it, i just don't want too.
  */
  err = flash_init();
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "mounted fs");

  /* opens the database, but does not do any database operations */
  spect_config_cfg_t cfg;
  cfg = (spect_config_cfg_t){
    .path="/flash/config.db",
  };

  err = spect_config_init(&cfg, &config_ctx);
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "config init ok");

  /* loads config from sqlite database. */
  err = spect_config_load(config_ctx);
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "config load ok");

  ESP_LOGI(TAG, "loaded db");
  ESP_LOGI(TAG, "free memory=%d", esp_get_minimum_free_heap_size());
  ESP_LOGI(TAG, "mode=%d", config_ctx->config->state->mode);

  /* spawns a thread, consider putting this on a different core. */
  ESP_LOGI(TAG, "ble init");
  err = spect_ble_init(config_ctx);
  ESP_ERROR_CHECK(err);
  ESP_LOGI(TAG, "ble init ok");

  /* 
   * very important variable. controls program flow. 
   * it is marked static because it should be treated as 
   * the PRIMARY mode switch. this allows this address to be
   * not thread safe. mode switching should be handled by
   * setting the global config_ctx mode, this handles all
   * cleanup
   */
  spect_mode_t current_mode = config_ctx->config->state->mode;
  ESP_LOGI(TAG, "bootup mode=%s", spect_mode_to_string(current_mode));

  /* begin initialization of peripherals */

  spect_rgb_config_t rgb_cfg;
  rgb_cfg = (spect_rgb_config_t) {
    .num_leds=config_ctx->config->strip_channel_1_length,
    .led_strip_gpio=GPIO_NUM_STRIP0,
    .ledc_channel_offset=0,
    .led_red_gpio=GPIO_NUM_RGB0_R,
    .led_green_gpio=GPIO_NUM_RGB0_G,
    .led_blue_gpio=GPIO_NUM_RGB0_B,
  };
  err = spect_rgb_install();
  ESP_ERROR_CHECK(err);
  // TODO: implement second channel

  /* rgb initializing */
  err = spect_rgb_initialize(&rgb_cfg, &channel0);
  ESP_ERROR_CHECK(err);
  config_ctx->rgb0 = channel0;
  err = spect_rgb_enable_strip(channel0);
  ESP_ERROR_CHECK(err);

  rgb_t color = {.red=0, .green=0, .blue=0};
  spect_rgb_fill(channel0, 0, channel0->strip->length, color);
  spect_rgb_set_color(channel0, color);
  spect_rgb_blit(channel0);
  spect_rgb_wait(channel0);

  SX1231_config_t radio_cfg = {
      .gpio_cs = GPIO_NUM_SX1231_CSN,
      .gpio_int = GPIO_NUM_SX1231_IRQ,
      .gpio_reset = GPIO_NUM_SX1231_RESET,
      .freqBand = RF69_915MHZ,
      .nodeID = 100, 
      .networkID = 100,
      .isRFM69HW_HCW = true,
      .host = SPI2_HOST
  };
  err = spect_radio_initialize(config_ctx, &radio_cfg);
  ESP_ERROR_CHECK(err);

uint8_t color_buffer[4] = {0};
rgb_t   color_          = {0};

/* VERY IMPORTANT!!!!!! DO NOT CREATE MORE STACK VARIABLES HERE U FOOL!!! */
main_loop_init:
  if(current_mode == SPECT_MODE_EFFECT_RAINBOW)  {
    ESP_LOGI(TAG, "Starting rainbow");
    rainbow_state_init(channel0, 
                       channel0->strip->length, 
                       config_ctx->config->state->data.rainbow.delay_time);
  }

  if(current_mode == SPECT_MODE_EFFECT_SOLID || current_mode == SPECT_MODE_RADIO) {
    memset(color_buffer, 0, 4);
    memset(&color_, 0, sizeof(rgb_t));

    color_buffer[0] = config_ctx->config->state->data.solid.channel0;
    color_buffer[1] = config_ctx->config->state->data.solid.channel0 >>  8;
    color_buffer[2] = config_ctx->config->state->data.solid.channel0 >> 16;
    color_buffer[3] = config_ctx->config->state->data.solid.channel0 >> 24;

    // LEDC uses RGB
    color_.red   = color_buffer[0];
    color_.green = color_buffer[1];
    color_.blue  = color_buffer[2];
    spect_rgb_set_color(channel0, color_);

    // Strip uses GRB
    color_.red   = color_buffer[1];
    color_.green = color_buffer[0];
    color_.blue  = color_buffer[2];
    ESP_LOGE("LED INIT", "fill[%d] %02X %02X %02X", 
      config_ctx->rgb0->strip->length, 
      color_.red, color_.green, color_.blue
    );
    spect_rgb_fill(channel0, 0, config_ctx->rgb0->strip->length, color_);
    spect_rgb_blit(channel0);
    spect_rgb_wait(channel0);
  }

  if(current_mode == SPECT_MODE_EFFECT_PULSE) {
    ESP_LOGE("LED", "SPECT_MODE_EFFECT_PULSE not implemented");
    abort();
  }

  if(current_mode == SPECT_MODE_RADIO) {
    ESP_LOGI("RADIO", "SPECT_MODE_RADIO init");
  }

  /* Main loop begins here. */
  while(config_ctx->config->state->mode == current_mode) {
    /* each mode should implement a `loop` function where 
     * relevant. This `loop` should only be one step of the loop
     * not enter an endless loop. This is subtly quite important
     * the main control loop will jump back before this point
     * all control flow is handled in this way, meaning if 
     * a mode takes too long to "step" it's state, the entire
     * UX will suffer */
    if(current_mode == SPECT_MODE_EFFECT_SOLID) {};
    if(current_mode == SPECT_MODE_EFFECT_RAINBOW) rainbow_loop(channel0);
    if(current_mode == SPECT_MODE_EFFECT_SOLID || current_mode == SPECT_MODE_RADIO) {
        color_buffer[0] = config_ctx->config->state->data.solid.channel0;
        color_buffer[1] = config_ctx->config->state->data.solid.channel0 >>  8;
        color_buffer[2] = config_ctx->config->state->data.solid.channel0 >> 16;
        color_buffer[3] = config_ctx->config->state->data.solid.channel0 >> 24;

        if((color_.red != color_buffer[1]) || (color_.green != color_buffer[0]) || (color_.blue != color_buffer[2])) {
            // LEDC uses RGB
            color_.red   = color_buffer[0];
            color_.green = color_buffer[1];
            color_.blue  = color_buffer[2];
            spect_rgb_set_color(channel0, color_);

            // Strip uses GRB
            color_.red   = color_buffer[1];
            color_.green = color_buffer[0];
            color_.blue  = color_buffer[2];
            ESP_LOGI("LED change", "fill[%d] %02X %02X %02X", 
                config_ctx->rgb0->strip->length, 
                color_.red, color_.green, color_.blue
            );
            spect_rgb_fill(channel0, 0, config_ctx->rgb0->strip->length, color_);
            spect_rgb_blit(channel0);
            spect_rgb_wait(channel0);
            spect_radio_broadcast_state(config_ctx, &color_);
        }
    }
    if(current_mode == SPECT_MODE_RADIO) spect_radio_loop(config_ctx);
    vTaskDelay(pdMS_TO_TICKS(10));
    // ESP_LOGI(TAG, "LOOP done");
  }
  ESP_LOGW(TAG, "mode changed from %s to %s", spect_mode_to_string(current_mode), spect_mode_to_string(config_ctx->config->state->mode));
  /* do not reset any other state here. all initialization should 
   * be done at the next address */
  current_mode = config_ctx->config->state->mode;
  goto main_loop_init;
}
