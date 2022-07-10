#include <stdio.h>
#include "spect-config.h"

esp_err_t spect_config_init(spect_config_cfg* cfg, spect_config_t** out_ctx)
{
  esp_err_t err;
  int rc = sqlite3_open(cfg->path, db);
  if(rc)
    return ESP_ERR_INVALID_RESPONSE;

  spect_config_t* ctx = (spect_config_t*)malloc(sizeof(spect_config_t));
  if (!ctx) return ESP_ERR_NO_MEM;

  *ctx = (spect_config_t){
    .cfg = cfg,
    .db = db,
  };

  *out_ctx = ctx;
  return ESP_OK;
}