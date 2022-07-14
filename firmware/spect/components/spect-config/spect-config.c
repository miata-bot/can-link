#include <stdio.h>
#include <esp_log.h>

#include "spect-config.h"

static const char *TAG = "CONFIG";

static const char *mode_map[] = { 
  "SOLID",
  "RAINBOW",
  "PULSE",
  "RADIO",
  "SCRIPTED",
};

const char* spect_mode_to_string(spect_mode_t mode)
{
  if(mode >= SPECT_MODE_MAX) return NULL;
  return mode_map[mode];
}

esp_err_t spect_set_state(spect_config_context_t* ctx)
{
  int rc;
  sqlite3_stmt *res;

  switch(ctx->config->state->mode) {
    case SPECT_MODE_EFFECT_SOLID: {
      ESP_LOGI(TAG, "saving solid effect state %d, %d", ctx->config->state->data.solid.channel0, ctx->config->state->data.solid.channel1);
      rc = sqlite3_prepare_v2(ctx->db, "UPDATE state SET mode_solid_channel0 = ?1, mode_solid_channel1 = ?2;", -1, &res, 0);
      if(rc != ESP_OK) return ESP_ERR_INVALID_STATE;
      
      rc = sqlite3_bind_int(res, 1, ctx->config->state->data.solid.channel0);
      if(rc != ESP_OK) return ESP_ERR_INVALID_STATE;
      
      rc = sqlite3_bind_int(res, 2, ctx->config->state->data.solid.channel1);
      if(rc != ESP_OK) return ESP_ERR_INVALID_STATE;
    } break;
    case SPECT_MODE_EFFECT_RAINBOW: {
      ESP_LOGI(TAG, "saving rainbow effect state %d, %d", ctx->config->state->data.rainbow.length, ctx->config->state->data.rainbow.delay_time);
      rc = sqlite3_prepare_v2(ctx->db, "UPDATE state SET mode_rainbow_length = ?1, mode_rainbow_delay_time = ?2;", -1, &res, 0);
      if(rc != ESP_OK) return ESP_ERR_INVALID_STATE;
      
      rc = sqlite3_bind_int(res, 1, ctx->config->state->data.rainbow.length);
      if(rc != ESP_OK) return ESP_ERR_INVALID_STATE;
      
      rc = sqlite3_bind_int(res, 2, ctx->config->state->data.rainbow.delay_time);
      if(rc != ESP_OK) return ESP_ERR_INVALID_STATE;
    } break;
    case SPECT_MODE_EFFECT_PULSE: {
      ESP_LOGI(TAG, "saving pulse effect state %d, %d", ctx->config->state->data.pulse.length, ctx->config->state->data.pulse.pulsewidth);
      rc = sqlite3_prepare_v2(ctx->db, "UPDATE state SET mode_pulse_length = ?1, mode_pulse_pulsewidth = ?2;", -1, &res, 0);
      if(rc != ESP_OK) return ESP_ERR_INVALID_STATE;
      
      rc = sqlite3_bind_int(res, 1, ctx->config->state->data.pulse.length);
      if(rc != ESP_OK) return ESP_ERR_INVALID_STATE;
      
      rc = sqlite3_bind_int(res, 2, ctx->config->state->data.pulse.pulsewidth);
      if(rc != ESP_OK) return ESP_ERR_INVALID_STATE;
    } break;
    default: {
      ESP_LOGE(TAG, "unknown mode!!!");
      rc = sqlite3_prepare_v2(ctx->db, "PRAGMA noop", -1, &res, 0);
      if(rc != ESP_OK) return ESP_ERR_INVALID_STATE;
    } break;
  }

  rc = sqlite3_step(res);

  if (rc != SQLITE_DONE) {
    ESP_LOGE(TAG, "Failed to update state: %s\n", sqlite3_errmsg(ctx->db));
    return ESP_ERR_INVALID_STATE;
  }

  return ESP_OK;
}

esp_err_t spect_set_mode(spect_config_context_t* ctx, spect_mode_t mode) 
{
  if(mode >= SPECT_MODE_MAX) {
    ESP_LOGE(TAG, "invalid mode: %d", mode);
    return ESP_ERR_INVALID_ARG;
  }

  if(ctx->config->state->mode == mode) return ESP_OK;

  int rc;
  sqlite3_stmt *res;

  rc = sqlite3_prepare_v2(ctx->db, "UPDATE state SET mode = ?1;", -1, &res, 0);

  if (rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to update state: %s\n", sqlite3_errmsg(ctx->db));
    return ESP_ERR_INVALID_STATE;
  }

  rc = sqlite3_bind_int(res, 1, mode);

  if (rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to bind network\n");
    return ESP_ERR_INVALID_STATE;
  }

  rc = sqlite3_step(res);

  if (rc != SQLITE_DONE) {
    ESP_LOGE(TAG, "Failed to update mode: %s\n", sqlite3_errmsg(ctx->db));
    return ESP_ERR_INVALID_STATE;
  } 
  ESP_LOGI(TAG, "mode=%s (%d) => %s (%d)", mode_map[ctx->config->state->mode], ctx->config->state->mode, mode_map[mode], mode);
  
  rc = spect_config_load_state(ctx);
  if(rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to reload state");
    return ESP_ERR_INVALID_STATE;
  }

  return ESP_OK;
}

int spect_config_load_state(spect_config_context_t* ctx)
{
  int rc;
  sqlite3_stmt *res;
  
  rc = sqlite3_prepare_v2(ctx->db, "SELECT c0.mode, c0.mode_solid_channel0, c0.mode_solid_channel1, c0.mode_rainbow_length, c0.mode_rainbow_delay_time, c0.mode_pulse_length, c0.mode_pulse_pulsewidth FROM state AS c0;", -1, &res, 0);
  if (rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to fetch state: %s\n", sqlite3_errmsg(ctx->db));
    return rc;
  }
  rc = sqlite3_step(res);
  
  if (rc == SQLITE_ROW) {
    ctx->config->state->mode = sqlite3_column_int(res, 0);
    switch(ctx->config->state->mode) {
      case SPECT_MODE_EFFECT_SOLID: {
        ESP_LOGI(TAG, "loading solid state");
        ctx->config->state->data.solid.channel0 = sqlite3_column_int(res, 1);
        ctx->config->state->data.solid.channel1 = sqlite3_column_int(res, 2);
      } break;
      case SPECT_MODE_EFFECT_RAINBOW: {
        ctx->config->state->data.rainbow.length = sqlite3_column_int(res, 3);
        ctx->config->state->data.rainbow.delay_time = sqlite3_column_int(res, 4);
      } break;
      case SPECT_MODE_EFFECT_PULSE: {
        ctx->config->state->data.pulse.length = sqlite3_column_int(res, 5);
        ctx->config->state->data.pulse.pulsewidth = sqlite3_column_int(res, 6);
      } break;
      // case SPECT_MODE_RADIO: {} break;
      // case SPECT_MODE_SCRIPTED: {} break;
      default:
        ESP_LOGE(TAG, "unknown mode!!");
        break;
    }
    rc = SQLITE_OK;
  } else {
    ESP_LOGE(TAG, "Failed to fetch config: %s\n", sqlite3_errmsg(ctx->db));
  }
  
  sqlite3_finalize(res);
  return rc;
}

int spect_config_load_config(spect_config_context_t* ctx)
{
  ESP_LOGI(TAG, "loading config");
  int rc;
  sqlite3_stmt *res;
  rc = sqlite3_prepare_v2(ctx->db, "SELECT c0.id , c0.version , c0.rgb_channel_1_enable , c0.rgb_channel_2_enable , c0.strip_channel_1_enable , c0.strip_channel_2_enable , c0.digital_input_1_enable , c0.digital_input_2_enable , c0.digital_input_3_enable , c0.digital_input_4_enable , c0.network_identity_id , c0.network_leader_id , c0.network_id  FROM  config  AS c0;", -1, &res, 0);
  if (rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to fetch config: %s\n", sqlite3_errmsg(ctx->db));
    return rc;
  }
  rc = sqlite3_step(res);
  
  if (rc == SQLITE_ROW) {
    ctx->config->version = sqlite3_column_int(res, 1);
    ctx->config->rgb_channel_1_enable = sqlite3_column_int(res, 2);
    ctx->config->rgb_channel_2_enable = sqlite3_column_int(res, 3);
    ctx->config->strip_channel_1_enable = sqlite3_column_int(res, 4);
    ctx->config->strip_channel_2_enable = sqlite3_column_int(res, 5);
    ctx->config->digital_input_1_enable = sqlite3_column_int(res, 6);
    ctx->config->digital_input_2_enable = sqlite3_column_int(res, 7);
    ctx->config->digital_input_3_enable = sqlite3_column_int(res, 8);
    ctx->config->digital_input_4_enable = sqlite3_column_int(res, 9);
    ctx->config->network_identity_id = sqlite3_column_int(res, 10);
    ctx->config->network_leader_id = sqlite3_column_int(res, 11);
    ctx->config->network_id = sqlite3_column_int(res, 12);
    rc = SQLITE_OK;
  } else {
    ESP_LOGE(TAG, "Failed to fetch config: %s\n", sqlite3_errmsg(ctx->db));
  }
  
  sqlite3_finalize(res);
  return rc;
}

int spect_config_load_network(spect_config_context_t* ctx)
{
  sqlite3_stmt *res;
  int rc = 0;
  rc = sqlite3_prepare_v2(ctx->db, "SELECT n0.id, n0.config_id, n0.key, n0.config_id FROM networks AS n0 WHERE (n0.id = ?1) ORDER BY n0.config_id;", -1, &res, 0);    
  if (rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to fetch network: %s\n", sqlite3_errmsg(ctx->db));
    return rc;
  }

  rc = sqlite3_bind_int(res, 1, ctx->config->network_id);
  if (rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to bind network\n");
    return rc;
  }

  rc = sqlite3_step(res);

  if (rc == SQLITE_ROW) {
    ctx->config->network->id = sqlite3_column_int(res, 0);
    int key_length = sqlite3_column_bytes(res, 2)+1;
    if(key_length > 1) {
      ctx->config->network->key = malloc(sizeof(unsigned char) * key_length);
      strncpy(ctx->config->network->key, (const char*)sqlite3_column_text(res, 2), key_length);
      ctx->config->network->key[key_length] = '\0';
      ESP_LOGI(TAG, "network_key[%d]=%s", key_length, sqlite3_column_text(res, 2));
    } else {
      ctx->config->network->key = NULL;
    }
    rc = SQLITE_OK;
  } else {
    ESP_LOGE(TAG, "Failed to fetch network %d: %s\n", ctx->config->network_id, sqlite3_errmsg(ctx->db));
  }

  sqlite3_finalize(res);
  return rc;
}

int spect_config_load_network_identity(spect_config_context_t* ctx)
{
  int rc=0;
  sqlite3_stmt *res;
  
  rc = sqlite3_prepare_v2(ctx->db, "SELECT n0.id , n0.network_id , n0.node_id  FROM  network_identity AS n0 WHERE (n0.network_id = ?1);", -1, &res, 0);
  if (rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to fetch network identity: %s\n", sqlite3_errmsg(ctx->db));
    return rc;
  }

  rc = sqlite3_bind_int(res, 1, ctx->config->network_id);
  if (rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to bind network identity\n");
    return rc;
  }

  rc = sqlite3_step(res);

  if (rc == SQLITE_ROW) {
    ctx->config->network->identity->network = ctx->config->network;
    ctx->config->network->identity->id = sqlite3_column_int(res, 0);
    ctx->config->network->identity->network_id = sqlite3_column_int(res, 1);
    ctx->config->network->identity->node_id = sqlite3_column_int(res, 2);
    rc = SQLITE_OK;
  } else {
    ESP_LOGE(TAG, "Failed to fetch network identity: %s\n", sqlite3_errmsg(ctx->db));
  }

  sqlite3_finalize(res);
  return rc;
}

int spect_config_load_network_leader(spect_config_context_t* ctx)
{
  int rc=0;
  sqlite3_stmt *res;

  rc = spect_config_load_network_identity(ctx);
  if(rc != SQLITE_OK)
    return rc;

  rc = sqlite3_prepare_v2(ctx->db, "SELECT n0.id, n0.network_id, n0.node_id FROM network_leader AS n0 WHERE (n0.network_id = ?1);", -1, &res, 0);
  if (rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to fetch network leader: %s\n", sqlite3_errmsg(ctx->db));
    return rc;
  }

  rc = sqlite3_bind_int(res, 1, ctx->config->network_id);
  if (rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to bind network leader\n");
    return rc;
  }

  rc = sqlite3_step(res);

  if (rc == SQLITE_ROW) {
    ctx->config->network->leader->network = ctx->config->network;
    ctx->config->network->leader->id = sqlite3_column_int(res, 0);
    ctx->config->network->leader->network_id = sqlite3_column_int(res, 1);
    ctx->config->network->leader->node_id = sqlite3_column_int(res, 2);
    rc = SQLITE_OK;
  } else {
    ESP_LOGE(TAG, "Failed to fetch network leader: %s\n", sqlite3_errmsg(ctx->db));
  }

  sqlite3_finalize(res);
  return rc;
}

int spect_config_load_nodes(spect_config_context_t* ctx)
{
  int rc=0;
  sqlite3_stmt *res;

  rc = sqlite3_prepare_v2(ctx->db, "SELECT n0.id, n0.network_id, n0.name, n0.network_id FROM nodes AS n0 WHERE (n0.network_id = ?1) ORDER BY n0.id;", -1, &res, 0);
  if (rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to fetch nodes: %s\n", sqlite3_errmsg(ctx->db));
    return rc;
  }
  rc = sqlite3_bind_int(res, 1, ctx->config->network_id);
  if (rc != SQLITE_OK) {
    ESP_LOGE(TAG, "Failed to bind nodes\n");
    return rc;
  }
  int i = 0;

  do {
    rc = sqlite3_step(res);
    if(rc != SQLITE_ROW) break;

    ctx->config->network->nodes[i].network = ctx->config->network;
    ctx->config->network->nodes[i].id = sqlite3_column_int(res, 0);

    if(ctx->config->network->nodes[i].id == ctx->config->network->identity->node_id) {
      ctx->config->network->identity->node = &ctx->config->network->nodes[i];
    }

    if(ctx->config->network->nodes[i].id == ctx->config->network->leader->node_id) {
      ctx->config->network->leader->node = &ctx->config->network->nodes[i];
    }

    ctx->config->network->nodes[i].network_id = sqlite3_column_int(res, 1);
    int name_length = sqlite3_column_bytes(res, 2) + 1;
    if(name_length > 1) {
      ctx->config->network->nodes[i].name = malloc((sizeof(unsigned char) * name_length));
      strncpy(ctx->config->network->nodes[i].name, (const char*)sqlite3_column_text(res, 2), name_length);
      ctx->config->network->nodes[i].name[name_length] = '\0';
      ESP_LOGI(TAG, "node.name[%d]=%s", name_length, sqlite3_column_text(res, 2));
    } else {
      ctx->config->network->nodes[i].name = NULL;
      ESP_LOGI(TAG, "unnamed node");
    }
    ESP_LOGI(TAG, 
"\
node[%d]={\
id=%d \
network_id=%d \
name=%s}\
", i, ctx->config->network->nodes[i].id, ctx->config->network->nodes[i].network_id, ctx->config->network->nodes[i].name ? ctx->config->network->nodes[i].name : "[null]");
    i++;
  } while(rc == SQLITE_ROW);
  rc = SQLITE_OK;

  ESP_LOGE(TAG, "Failed to fetch nodes %d: %s\n", ctx->config->network_id, sqlite3_errmsg(ctx->db));
  sqlite3_finalize(res);
  return rc;
}

esp_err_t spect_config_load(spect_config_context_t* ctx)
{
  int rc = 0;
  rc = spect_config_load_state(ctx);
  if(rc != SQLITE_OK) return ESP_ERR_INVALID_ARG;
  
  rc = spect_config_load_config(ctx);
  if(rc != SQLITE_OK) return ESP_ERR_INVALID_ARG;

  ESP_LOGI(TAG, "loaded config={\n\
  .rgb_channel_1_enable=%d\n\
  .rgb_channel_2_enable=%d\n\
  .strip_channel_1_enable=%d\n\
  .strip_channel_2_enable=%d\n\
  .digital_input_1_enable=%d\n\
  .digital_input_2_enable=%d\n\
  .digital_input_3_enable=%d\n\
  .digital_input_4_enable=%d\n\
  .network_id=%d\n\
  .network_identity_id=%d\n\
  .network_leader_id=%d\r\n}\
",
    ctx->config->rgb_channel_1_enable,
    ctx->config->rgb_channel_2_enable,
    ctx->config->strip_channel_1_enable,
    ctx->config->strip_channel_2_enable,
    ctx->config->digital_input_1_enable,
    ctx->config->digital_input_2_enable,
    ctx->config->digital_input_3_enable,
    ctx->config->digital_input_4_enable,
    ctx->config->network_id,
    ctx->config->network_identity_id,
    ctx->config->network_leader_id
  );

  rc = spect_config_load_network(ctx);
  if(rc != SQLITE_OK) return ESP_ERR_INVALID_STATE;
  ESP_LOGI(TAG, "network={.id=%d, .key=%s}", ctx->config->network->id, ctx->config->network->key ? ctx->config->network->key : "[null]");

  rc = spect_config_load_network_identity(ctx);
  if(rc != SQLITE_OK) return ESP_ERR_INVALID_STATE;
  ESP_LOGI(TAG, "network_identity={.id=%d, .network_id=%d, .node_id=%d}", ctx->config->network->identity->id, ctx->config->network->identity->network_id, ctx->config->network->identity->node_id);

  rc = spect_config_load_network_leader(ctx);
  if(rc != SQLITE_OK) return ESP_ERR_INVALID_STATE;
  ESP_LOGI(TAG, "network_leader={.id=%d, .network_id=%d, .node_id=%d}", ctx->config->network->leader->id, ctx->config->network->leader->network_id, ctx->config->network->leader->node_id);

  rc = spect_config_load_nodes(ctx);
  if(rc != SQLITE_OK) return ESP_ERR_INVALID_STATE;

  return ESP_OK;
}

esp_err_t spect_config_init(spect_config_cfg_t* cfg, spect_config_context_t** out_ctx)
{    
  sqlite3_initialize();
  ESP_LOGI(TAG, "config init");
  spect_state_t* state;
  spect_network_t* network;
  spect_node_t* nodes;
  spect_network_identity_t* identity;
  spect_network_leader_t* leader;
  spect_config_t* config;
  spect_config_context_t* ctx;

  ctx = (spect_config_context_t*)malloc(sizeof(spect_config_context_t));
  if (!ctx) {ESP_LOGI(TAG, "failed to allocate"); return ESP_ERR_NO_MEM;}

  config = (spect_config_t*)malloc(sizeof(spect_config_t));
  if(!config) {ESP_LOGI(TAG, "failed to allocate"); return ESP_ERR_NO_MEM;}

  state = (spect_state_t*)malloc(sizeof(spect_state_t));
  if(!config) {ESP_LOGI(TAG, "failed to allocate"); return ESP_ERR_NO_MEM;}
  config->state = state;

  network = (spect_network_t*)malloc(sizeof(spect_network_t));
  if(!network) {ESP_LOGI(TAG, "failed to allocate"); return ESP_ERR_NO_MEM;}
  config->network = network;

  leader = (spect_network_leader_t*)malloc(sizeof(spect_network_leader_t));
  if(!leader) {ESP_LOGI(TAG, "failed to allocate"); return ESP_ERR_NO_MEM;}
  config->network->leader = leader;

  identity = (spect_network_identity_t*)malloc(sizeof(spect_network_identity_t));
  if(!identity) {ESP_LOGI(TAG, "failed to allocate"); return ESP_ERR_NO_MEM;}
  config->network->identity = identity;

  // this will limit the max number of nodes in a network
  // far below the technical max, but there's no way this will work
  // with more 10 nodes anyway
  nodes = malloc(10 * sizeof(struct SpectNetwork));
  if(!nodes) {ESP_LOGI(TAG, "failed to allocate"); return ESP_ERR_NO_MEM;}
  config->network->nodes = nodes;

  *ctx = (spect_config_context_t){
    .cfg = cfg,
    .config = config
  };

  ESP_LOGI(TAG, "db init %s", cfg->path);
  int rc = sqlite3_open(cfg->path, &ctx->db);
  if(rc) {
    ESP_LOGI(TAG, "failed to open DB %s", sqlite3_errmsg(ctx->db));
    return ESP_ERR_INVALID_STATE;
  }
  ESP_LOGI(TAG, "db init ok");
  *out_ctx = ctx;
  return ESP_OK;
}