#include <stdint.h>
#include <sqlite3.h>

typedef struct Network {
  uint8_t id;
  char* key;
} network_t;

typedef struct node {
  uint16_t id;
  uint8_t network_id;
  char* name;
  network_t* network;
} node_t;

typedef struct NetworkIdentity {
  uint16_t id;
  uint16_t node_id;
  uint8_t network_id;
  node_t* node;
  network_t* network;
} network_identity_t;

typedef struct NetworkLeader {
  uint16_t id;
  uint16_t node_id;
  uint8_t network_id;
  node_t* node;
  network_t* network;
} network_leader_t;

typedef struct Config {
  sqlite3 db;
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
  network_t* network;

  uint16_t network_identity_id;
  network_identity_t* network_identity;

  uint16_t network_leader_id;
  network_leader_t* network_leader;
} config_t;

void load_config(config_t** config_out) 
{

}