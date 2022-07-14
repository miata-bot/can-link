#include <stdio.h>
#include <esp_log.h>

#include "spect-config.h"

static const char *TAG = "SPECT-CONFIG";

/*
SELECT c0."id", c0."version", c0."rgb_channel_1_enable", c0."rgb_channel_2_enable", c0."strip_channel_1_enable", c0."strip_channel_2_enable", c0."digital_input_1_enable", c0."digital_input_2_enable", c0."digital_input_3_enable", c0."digital_input_4_enable", c0."inserted_at", c0."updated_at" FROM "config" AS c0;
SELECT n0."id", n0."config_id", n0."key", n0."config_id" FROM "networks" AS n0 WHERE (n0."config_id" = 0) ORDER BY n0."config_id";
SELECT s0."id", s0."config_id", s0."mode", s0."rgb_channel_1_color", s0."rgb_channel_1_brightness", s0."rgb_channel_2_color", s0."rgb_channel_2_brightness", s0."config_id" FROM "state" AS s0 WHERE (s0."config_id" = 0);
SELECT n0."id", n0."network_id", n0."name", n0."network_id" FROM "nodes" AS n0 WHERE (n0."network_id" = 100) ORDER BY n0."id"
*/

esp_err_t spect_config_load(spect_config_context_t* ctx)
{
  int rc;
  sqlite3_stmt *res;
  rc = sqlite3_prepare_v2(ctx->db, "SELECT c0.\"id\", c0.\"version\", c0.\"rgb_channel_1_enable\", c0.\"rgb_channel_2_enable\", c0.\"strip_channel_1_enable\", c0.\"strip_channel_2_enable\", c0.\"digital_input_1_enable\", c0.\"digital_input_2_enable\", c0.\"digital_input_3_enable\", c0.\"digital_input_4_enable\", c0.\"inserted_at\", c0.\"updated_at\" FROM \"config\" AS c0;", -1, &res, 0);    
  if (rc != SQLITE_OK) {
    ESP_LOGE("config", "Failed to fetch data: %s\n", sqlite3_errmsg(ctx->db));
    return ESP_ERR_INVALID_ARG;
  }    
    
  rc = sqlite3_step(res);
  
  if (rc == SQLITE_ROW) {
    //id=0
    ctx->config->version = sqlite3_column_int(res, 1);
    ctx->config->rgb_channel_1_enable = sqlite3_column_int(res, 2);
    ctx->config->rgb_channel_2_enable = sqlite3_column_int(res, 3);
    ctx->config->strip_channel_1_enable = sqlite3_column_int(res, 4);
    ctx->config->strip_channel_2_enable = sqlite3_column_int(res, 5);
    ctx->config->digital_input_1_enable = sqlite3_column_int(res, 6);
    ctx->config->digital_input_2_enable = sqlite3_column_int(res, 7);
    ctx->config->digital_input_3_enable = sqlite3_column_int(res, 8);
    ctx->config->digital_input_4_enable = sqlite3_column_int(res, 9);
    // inserted_at
    // updated_at
    ESP_LOGI("config", "loaded\n");
  }
  
  sqlite3_finalize(res);
  return ESP_OK;

  // rc = sqlite3_prepare_v2(ctx->db, "SELECT n0.\"id\", n0.\"config_id\", n0.\"key\", n0.\"config_id\" FROM \"networks\" AS n0 WHERE (n0.\"config_id\" = 0) ORDER BY n0.\"config_id\";", -1, &res, 0);    
  // if (rc != SQLITE_OK) {
  //   ESP_LOGE("config", "Failed to fetch network: %s\n", sqlite3_errmsg(db));
  //   return ESP_ERR_INVALID_ARG;
  // }

  // if (rc == SQLITE_ROW) {
  //   // id=0
  //   // config_id=0
  //   ctx->config->network->id = sqlite3_column_int(res, 9);
  //   ESP_LOGI("config", "loaded network");
  // }
}

esp_err_t spect_config_init(spect_config_cfg_t* cfg, spect_config_context_t** out_ctx)
{    
  sqlite3_initialize();
  ESP_LOGI(TAG, "config init");
  spect_network_t* network;
  spect_node_t** nodes;
  spect_network_identity_t* identity;
  spect_network_leader_t* leader;
  spect_config_t* config;
  spect_config_context_t* ctx;

  ctx = (spect_config_context_t*)malloc(sizeof(spect_config_context_t));
  if (!ctx) return ESP_ERR_NO_MEM;

  config = (spect_config_t*)malloc(sizeof(spect_config_t));
  if(!config) return ESP_ERR_NO_MEM;

  network = (spect_network_t*)malloc(sizeof(spect_network_t));
  if(!network) return ESP_ERR_NO_MEM;
  config->network = network;

  leader = (spect_network_leader_t*)malloc(sizeof(spect_network_leader_t));
  if(!leader) return ESP_ERR_NO_MEM;
  config->network->leader = leader;

  identity = (spect_network_identity_t*)malloc(sizeof(spect_network_identity_t));
  if(!identity) return ESP_ERR_NO_MEM;
  config->network->identity = identity;

  // this will limit the max number of nodes in a network
  // far below the technical max, but there's no way this will work
  // with more 10 nodes anyway
  nodes = (spect_node_t**)malloc(sizeof(spect_node_t)*10);
  if(!nodes) return ESP_ERR_NO_MEM;
  config->network->nodes = nodes;

  *ctx = (spect_config_context_t){
    .cfg = cfg,
    .config = config
  };

  ESP_LOGI(TAG, "db init %s", cfg->path);
  int rc = sqlite3_open(cfg->path, &ctx->db);
  if(rc) {
    ESP_LOGE(TAG, "failed to open DB %s", sqlite3_errmsg(ctx->db));
    return ESP_ERR_INVALID_RESPONSE;
  }
  ESP_LOGI(TAG, "db init ok");

  *out_ctx = ctx;
  return ESP_OK;
}