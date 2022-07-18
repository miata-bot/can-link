#ifndef SPECT_RADIO_H
#define SPECT_RADIO_H

#include "spect-config.h"
#include "sx1231.h"

typedef enum SpectRadioOpcode {
  SPECT_RADIO_NETWORK_UPDATE_NEW_LEADER =0x10,
  SPECT_RADIO_RGB_FILL                  =0x20,
} __attribute__ ((__packed__)) spect_radio_opcode_t;

typedef struct SpectRadioNetworkUpdateNewLeader {
  spect_node_id_t new_node_id;
} __attribute__ ((__packed__)) spect_radio_network_update_new_leader_t;

typedef struct SpectRadioRGBFill {
  uint8_t channel;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t _unused;
} __attribute__ ((__packed__)) spect_radio_rgb_fill_t;

typedef union {
  spect_radio_network_update_new_leader_t new_leader;
  spect_radio_rgb_fill_t                  fill;
} spect_radio_packet_data_t;

typedef struct SpectRadioPacket {
  spect_node_t*             sender;
  spect_radio_opcode_t      opcode;
  spect_radio_packet_data_t data;
  uint16_t                  rssi;
} spect_radio_packet_t;

esp_err_t spect_radio_initialize(spect_config_context_t* config_ctx, SX1231_config_t* cfg);
esp_err_t spect_radio_loop(spect_config_context_t* config_ctx);

esp_err_t spect_packet_decode_init(spect_radio_packet_t** packet);
esp_err_t spect_packet_decode(spect_config_context_t*     config_ctx, 
                              uint8_t*                    data, 
                              uint8_t                     length, 
                              spect_node_id_t             sender_node_id,
                              uint16_t                    rssi,
                              spect_radio_packet_t*       packet
);

#endif