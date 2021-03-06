#ifndef SPECT_CONFIG_H
#define SPECT_CONFIG_H

#include <stdint.h>
#include <string.h>

#include <esp_system.h>

#include <sqlite3.h>

#include "spect-rgb-channel.h"

#define SPECT_MAX_NODES 10

typedef uint16_t spect_node_id_t;

typedef struct SpectNode {
  uint16_t id;
  uint8_t network_id;
  char* name;
  struct SpectNetwork* network;
} spect_node_t;

/**
 * @brief the current node's identity. 
 * loaded on boot, should never change.
 */
typedef struct SpectNetworkIdentity {
  uint16_t id;
  spect_node_id_t node_id;
  uint8_t network_id;
  spect_node_t* node;
  struct SpectNetwork* network;
} spect_network_identity_t;

/**
 * @brief the current network's leader.
 * There can only be one leader per network, each device must be
 * configured with the same leader. Can change dynamically.
 */
typedef struct SpectNetworkLeader {
  spect_node_id_t id;
  spect_node_id_t node_id;
  uint8_t network_id;
  spect_node_t* node;
  struct SpectNetwork* network;
} spect_network_leader_t;

/**
 * @brief Current device's network. The database can technically contain
 * many of these, but changing networks is not implemented.
 */
typedef struct SpectNetwork {
  uint8_t id;

  /* encryption key */
  char* key;

  /* array of SPECT_MAX_NODES nodes. Dynamically allocated */
  spect_node_t* nodes;

  spect_network_leader_t* leader;
  spect_network_identity_t* identity;
} spect_network_t;

typedef enum SpectMode {
  SPECT_MODE_EFFECT_SOLID   = 0x0,
  SPECT_MODE_EFFECT_RAINBOW = 0x1,
  SPECT_MODE_EFFECT_PULSE   = 0x2,
  SPECT_MODE_RADIO          = 0x3,
  SPECT_MODE_SCRIPTED       = 0x4,
  SPECT_MODE_MAX
} __attribute__ ((__packed__)) spect_mode_t;

typedef struct SpectModeSolidData
{
  uint32_t channel0;
  uint32_t channel1; 
} spect_mode_solid_data_t;

typedef struct SpectModeRainbowData
{
  uint32_t length;
  uint32_t delay_time;
} spect_mode_rainbow_data_t;

typedef struct SpectModePulseData
{
  uint32_t length;
  uint32_t pulsewidth;
} spect_mode_pulse_data_t;

typedef struct SpectModeRadioData
{
  uint32_t channel0;
  uint32_t channel1; 
} spect_mode_radio_data_t;

typedef struct SpectState {
  spect_mode_t mode;
  union {
    spect_mode_solid_data_t   solid;
    spect_mode_rainbow_data_t rainbow;
    spect_mode_pulse_data_t   pulse;
    spect_mode_radio_data_t   radio;
  } data;
} spect_state_t;

typedef struct SpectConfig {
  // meta
  uint8_t version;

  // hardware
  bool rgb_channel_1_enable;
  bool rgb_channel_2_enable;
  bool strip_channel_1_enable;
  bool strip_channel_2_enable;

  uint16_t strip_channel_1_length;
  uint16_t strip_channel_2_length;

  bool digital_input_1_enable;
  bool digital_input_2_enable;
  bool digital_input_3_enable;
  bool digital_input_4_enable;

  // SQL ID's not network id
  uint16_t network_identity_id;
  uint16_t network_leader_id;

  // radio network
  uint8_t network_id;
  spect_network_t* network;
  spect_state_t* state;
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
  spect_rgb_t* rgb0;
} spect_config_context_t;

esp_err_t spect_config_init(spect_config_cfg_t* cfg, spect_config_context_t** out_ctx);
esp_err_t spect_config_load(spect_config_context_t* ctx);
esp_err_t spect_set_mode(spect_config_context_t* ctx, spect_mode_t mode);
const char* spect_mode_to_string(spect_mode_t mode);
esp_err_t spect_set_state(spect_config_context_t* ctx);
esp_err_t spect_set_config(spect_config_context_t* ctx);

spect_node_t* spect_config_lookup_node(spect_config_context_t* ctx, spect_node_id_t node_id);

// private
int spect_config_load_state(spect_config_context_t* ctx);
int spect_config_load_config(spect_config_context_t* ctx);

#endif