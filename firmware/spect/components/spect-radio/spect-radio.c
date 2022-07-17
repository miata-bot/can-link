#include <stdio.h>
#include "esp_log.h"

#include "spect-radio.h"
#include "sx1231.h"

SX1231_t* sx1231;
typedef struct
{
  int version;
  uint8_t node_id;
  uint8_t network_id;

} configuration;


esp_err_t spect_initialize(spect_config_context_t* config_ctx, SX1231_config_t* cfg)
{
  esp_err_t err;
  err = sx1231_initialize(&cfg,  &sx1231);
  ESP_ERROR_CHECK(err);
  ESP_LOGI("RADIO", "Initalized radio");
  return ESP_OK;
}

esp_err_t spect_loop(spect_config_context_t* config_ctx)
{
  // if(sx1231_sendWithRetry(sx1231, 2, "ABCD", 4, 3, 10)) {
  //     ESP_LOGI("RADIO", "got ack");
  // }
  if(sx1231_receiveDone(sx1231)) {
    ESP_LOGI("RADIO", "SENDER=%d RSSI=%d dbm", sx1231->SENDERID, sx1231->RSSI);
    ESP_LOG_BUFFER_HEXDUMP("radio payload", sx1231->DATA, sx1231->DATALEN, ESP_LOG_INFO);

    switch(sx1231->DATA[0]) {
      default: {
          ESP_LOGE("RADIO", "unknown command %02X", sx1231->DATA[0]);
          break;
      }
    }
  }
  return ESP_OK;
}
