#include "pico.h"

static const char TAG[] = "PICO";

command_response_t pico_ping(pico_t* pico)
{
    pico_command_t command;
    memset(&command, 0, sizeof(pico_command_t));
    memset(&command.args, 0, sizeof(command.args));

    command.type = COMMAND_SYNC;

    command_response_t response = pico_send_command(pico, &command);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    return response;
}

command_response_t pico_set_color(pico_t* pico, command_arg_index_t index, uint8_t r, uint8_t g, uint8_t b)
{
    pico_command_t command;
    memset(&command, 0, sizeof(pico_command_t));
    memset(&command.args, 0, sizeof(command.args));

    command.type = COMMAND_RGB_SET_COLOR;
    command.index = index;
    command.args.rgb_set_color.r = r;
    command.args.rgb_set_color.g = g;
    command.args.rgb_set_color.b = b;

    command_response_t response = pico_send_command(pico, &command);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    return response;
}

command_response_t pico_set_brightness(pico_t* pico, command_arg_index_t index, uint8_t brightness)
{
    pico_command_t command;
    memset(&command, 0, sizeof(pico_command_t));
    memset(&command.args, 0, sizeof(command.args));

    command.type = COMMAND_RGB_SET_BRIGHTNESS;
    command.index = COMMAND_RGB_INDEX_0;
    command.args.rgb_set_brightness.brightness = brightness;

    command_response_t response = pico_send_command(pico, &command);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    return response;
}

esp_err_t pico_initialize(pico_config_t *cfg, pico_t** out_ctx)
{
    esp_err_t err = ESP_OK;
    spi_device_handle_t spi;

    spi_device_interface_config_t devcfg= {
        // .clock_speed_hz = APB_CLK_FREQ/800,
        .clock_speed_hz = SPI_MASTER_FREQ_10M,
        .mode = 0,  //SPI mode 0
        .spics_io_num = cfg->gpio_cs,
        .queue_size = 1,
        // .input_delay_ns=100,
        // .flags = SPI_DEVICE_NO_DUMMY
    };

    //Attach the pico to the SPI bus
    err = spi_bus_add_device(cfg->host, &devcfg, &spi);

    if (err != ESP_OK)  {
        ESP_LOGE(TAG, "Could not create SPI device");
        return err;
    } // TODO: should cleanup here

    pico_t* ctx = (pico_t*)malloc(sizeof(pico_t));
    if (!ctx) return ESP_ERR_NO_MEM;

    *ctx = (pico_t) {
        .cfg = *cfg,
        .spi = spi
    };
    err = pico_reset(ctx);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Could not reset");
        return err;
    }

    pico_sync(ctx);

    *out_ctx = ctx;
    return ESP_OK;
}

esp_err_t pico_sync(pico_t* pico)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "starting sync");
    pico_command_t command;
    memset(&command, 0, sizeof(pico_command_t));
    memset(&command.args, 0, sizeof(command.args));

    command.type = COMMAND_SYNC;

    command_response_t response = pico_send_command(pico, &command);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    // spi_transaction_t t;
    // memset(&t, 0, sizeof(t));

    // while(t.rx_data[0] != COMMAND_SYNC) {
    //     memset(&t, 0, sizeof(t));
    //     t.length = 8;
    //     t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
    //     t.tx_data[0] = COMMAND_SYNC;
    //     t.user = pico;

    //     err = spi_device_acquire_bus(pico->spi, portMAX_DELAY);
    //     if(err != ESP_OK)
    //         return err;

    //     err = spi_device_polling_transmit(pico->spi, &t);
    //     if(err != ESP_OK)
    //         return err;

    //     ESP_LOGI(TAG, "sync response=%02X", t.rx_data[0]);
    //     spi_device_release_bus(pico->spi);
    //     vTaskDelay(500 / portTICK_PERIOD_MS);
    // }
    // ESP_LOGI(TAG, "synce OK");
    return err;
}

esp_err_t pico_reset(pico_t* pico)
{
    gpio_set_direction(pico->cfg.gpio_en, GPIO_MODE_OUTPUT);
    gpio_set_level(pico->cfg.gpio_en, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(pico->cfg.gpio_en, 1);
    return ESP_OK;
}

command_response_t pico_send_command(pico_t* pico, pico_command_t* command)
{
    esp_err_t err;
    memset(command->rx_buffer, 0, 5);
    err = spi_device_acquire_bus(pico->spi, portMAX_DELAY);
    ESP_ERROR_CHECK(err);

    /*
     * The rp2040's SPI block relies on CS being toggled every bite instead of
     * for the duration of a transaction. This means we have to buffer all 5
     * transactions required for one command individually.
     *
     * the command's args are accessed as raw bytes in the union.
     */
    uint8_t* args = command->args._raw;
    spi_transaction_t t[5] = {
        {
            .length = 8,
            .flags = SPI_TRANS_USE_TXDATA,
            .tx_data = {(command->type << 4) | command->index},
            .rx_buffer = command->rx_buffer,
            .user = pico,
        },
        {
            .length = 8,
            .flags = SPI_TRANS_USE_TXDATA,
            .tx_data = {args[0]},
            .rx_buffer = command->rx_buffer+1,
            .user = pico,
        },
        {
            .length = 8,
            .flags = SPI_TRANS_USE_TXDATA,
            .tx_data = {args[1]},
            .rx_buffer = command->rx_buffer+2,
            .user = pico,
        },
        {
            .length = 8,
            .flags = SPI_TRANS_USE_TXDATA,
            .tx_data = {args[2]},
            .rx_buffer = command->rx_buffer+3,
            .user = pico,
        },
        {
            .length = 8,
            .flags = SPI_TRANS_USE_TXDATA,
            .tx_data = {0x69},
            .rx_buffer = command->rx_buffer+4,
            .user = pico,
        }
    };

    // err = spi_device_polling_transmit(pico->spi, &t[0]);
    // ESP_ERROR_CHECK(err);
    // while(command->rx_buffer[0] != 0xfe) {
    //     ESP_LOGE(TAG, "sync error! %02X", command->rx_buffer[4]);
    //     err = spi_device_polling_transmit(pico->spi, &t[4]);
    //     ESP_ERROR_CHECK(err);
    // }

    for(uint8_t i = 0; i < 4; i++) {
        ESP_LOGI(TAG, "sending %d %02X", i, t[i].tx_data[0]);
        err = spi_device_polling_transmit(pico->spi, &t[i]);
        ESP_LOGI(TAG, "got %d %02X", i, command->rx_buffer[i]);
        ESP_ERROR_CHECK(err);
        if(command->rx_buffer[i] != 0xfe) {
            ESP_LOGE(TAG, "probably out of sync %02X", command->rx_buffer[i]);
            // vTaskDelay(500 / portTICK_PERIOD_MS);
            i = 0;
        }
    }
    err = spi_device_polling_transmit(pico->spi, &t[4]);
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "reply={%02X, %02X, %02X, %02X, %02X}", command->rx_buffer[0], command->rx_buffer[1], command->rx_buffer[2], command->rx_buffer[3], command->rx_buffer[4]);
    // while(command->rx_buffer[4] == 0xfb) {
    //     ESP_LOGE(TAG, "sync error! %02X", command->rx_buffer[4]);
    //     err = spi_device_polling_transmit(pico->spi, &t[4]);
    //     ESP_ERROR_CHECK(err);
    // }

    spi_device_release_bus(pico->spi);
    return command->rx_buffer[0];
}