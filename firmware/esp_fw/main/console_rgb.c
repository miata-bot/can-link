#include "console_rgb.h"

static const char *TAG = "CONSOLE_RGB";

extern pico_t* pico;

void fade(uint8_t r, uint8_t g, uint8_t b)
{
    pico_set_color(pico, COMMAND_RGB_INDEX_0, r, g, b);
    ESP_LOGI(TAG, "fade up %02X %02X %02X",  r, g, b);
    for(uint8_t i = 0; i < 255; i+=5) {
        pico_set_brightness(pico, COMMAND_RGB_INDEX_0, i);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "fade down %02X %02X %02X",  r, g, b);
    for(uint8_t i = 255; i != 0; i-=5) {
        pico_set_brightness(pico, COMMAND_RGB_INDEX_0, i);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
    pico_set_color(pico, COMMAND_RGB_INDEX_0, 0x00, 0x00, 0x00);
    pico_set_brightness(pico, COMMAND_RGB_INDEX_0, 0);
}

static struct {
    struct arg_int *r;
    struct arg_int *g;
    struct arg_int *b;
    struct arg_end *end;
} color_args;

static int console_color(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &color_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, color_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(TAG, "set_color(%02X,%02X,%02X)", color_args.r->ival[0], color_args.g->ival[0], color_args.b->ival[0]);
    pico_set_color(pico, COMMAND_RGB_INDEX_0, color_args.r->ival[0], color_args.g->ival[0], color_args.b->ival[0]);
    return 0;
}

static struct {
    struct arg_int *brightness;
    struct arg_end *end;
} brightness_args;

static int console_brightness(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &brightness_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, brightness_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(TAG, "set_brightness %02X", brightness_args.brightness->ival[0]);
    pico_set_brightness(pico, COMMAND_RGB_INDEX_0, brightness_args.brightness->ival[0]);
    return 0;
}

esp_err_t console_rgb_install()
{
    color_args.r = arg_intn("r", "red", "<u8>", 1, 1, "red 0-255");
    color_args.g = arg_intn("g", "green", "<u8>", 1, 1, "green 0-255");
    color_args.b = arg_intn("b", "blue", "<u8>", 1, 1, "blue 0-255");
    color_args.end = arg_end(3);

    const esp_console_cmd_t color_cmd = {
        .command = "set_color",
        .help = "set the rgb color",
        .hint = NULL,
        .func = &console_color,
        .argtable = &color_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&color_cmd));

    brightness_args.brightness = arg_intn("b", "brightness", "<u8>", 1, 1, "brightness 0-255");
    brightness_args.end = arg_end(1);

    const esp_console_cmd_t brightness_cmd = {
        .command = "set_brightness",
        .help = "set the brightness",
        .hint = NULL,
        .func = &console_brightness,
        .argtable = &brightness_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&brightness_cmd));
    return ESP_OK;
}