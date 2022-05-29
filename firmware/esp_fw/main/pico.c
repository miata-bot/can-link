#include "pico.h"

static const char TAG[] = "PICO";

esp_err_t pico_initialize(pico_config_t *cfg, pico_t** out_ctx)
{
  esp_err_t err = ESP_OK;
  spi_device_handle_t spi;

  spi_device_interface_config_t devcfg={
    .clock_speed_hz = APB_CLK_FREQ/800,
    // .clock_speed_hz = SPI_MASTER_FREQ_10M,
    .mode = 0,  //SPI mode 0
    .spics_io_num = cfg->gpio_cs,
    .queue_size = 1,
    .input_delay_ns=10,
    .flags = SPI_DEVICE_NO_DUMMY
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

  *out_ctx = ctx;
  return ESP_OK;
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
      .tx_data = {COMMAND_SYNC},
      .rx_buffer = command->rx_buffer+4,
      .user = pico,
    }
  };

  for(uint8_t i = 0; i <= 4; i++) {
    ESP_LOGI(TAG, "sending %d %02X", i, t[i].tx_data[0]);
    err = spi_device_polling_transmit(pico->spi, &t[i]);
    ESP_ERROR_CHECK(err);
  }

  spi_device_release_bus(pico->spi);
  return command->rx_buffer[0];
}