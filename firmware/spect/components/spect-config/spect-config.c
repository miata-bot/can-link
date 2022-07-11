#include <stdio.h>
#include <esp_log.h>

#include "spect-config.h"

static const char *TAG = "SPECT-CONFIG";

esp_err_t spect_config_init(spect_config_cfg_t* cfg, spect_config_t** out_ctx)
{    
  sqlite3_initialize();
  ESP_LOGI(TAG, "config init");
  spect_config_t* ctx = (spect_config_t*)malloc(sizeof(spect_config_t));
  if (!ctx) return ESP_ERR_NO_MEM;

  *ctx = (spect_config_t){
    .cfg = cfg
  };

  ESP_LOGI(TAG, "db init");
  int rc = sqlite3_open(cfg->path, &ctx->db);
  if(rc) {
    ESP_LOGE(TAG, "failed to open DB %s", sqlite3_errmsg(ctx->db));
    return ESP_ERR_INVALID_RESPONSE;
  }
  ESP_LOGI(TAG, "db init ok");

  *out_ctx = ctx;
  return ESP_OK;
}