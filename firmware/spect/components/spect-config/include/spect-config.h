#include <stdint.h>
#include <string.h>

#include <esp_system.h>

#include <sqlite3.h>

typedef struct SpectNode {
  uint16_t id;
  uint8_t network_id;
  char* name;
  struct SpectNetwork* network;
} spect_node_t;

typedef struct SpectNetworkIdentity {
  uint16_t id;
  uint16_t node_id;
  uint8_t network_id;
  spect_node_t* node;
  struct SpectNetwork* network;
} spect_network_identity_t;

typedef struct SpectNetworkLeader {
  uint16_t id;
  uint16_t node_id;
  uint8_t network_id;
  spect_node_t* node;
  struct SpectNetwork* network;
} spect_network_leader_t;

typedef struct SpectNetwork {
  uint8_t id;
  char* key;
  spect_node_t* nodes;
  spect_network_leader_t* leader;
  spect_network_identity_t* identity;
} spect_network_t;

typedef struct SpectConfig {
  // meta
  uint8_t version;

  // hardware
  bool rgb_channel_1_enable;
  bool rgb_channel_2_enable;
  bool strip_channel_1_enable;
  bool strip_channel_2_enable;

  bool digital_input_1_enable;
  bool digital_input_2_enable;
  bool digital_input_3_enable;
  bool digital_input_4_enable;

  // radio network
  uint8_t network_id;
  spect_network_t* network;

  uint16_t network_identity_id;
  spect_network_identity_t* network_identity;

  uint16_t network_leader_id;
  spect_network_leader_t* network_leader;
} spect_config_t;

typedef struct spect_config_cfg 
{
  const char* path;
} spect_config_cfg_t;

typedef struct spect_config_context
{
  spect_config_cfg_t* cfg;
  sqlite3* db;

  spect_config_t* config;
} spect_config_context_t;

esp_err_t spect_config_init(spect_config_cfg_t* cfg, spect_config_context_t** out_ctx);
esp_err_t spect_config_load(spect_config_context_t* ctx);

void console_main();