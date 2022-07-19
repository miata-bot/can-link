#ifndef SPECT_RADIO_H
#define SPECT_RADIO_H

#include "spect-config.h"
#include "sx1231.h"

#define SPECT_RADIO_BROADCAST_ADDR 0
#define SPECT_RADIO_MAX_DATA_LENGTH RF69_MAX_DATA_LEN

/* Current state of the radio system */
typedef enum SpectRadioState {
  SPECT_RADIO_STATE_INIT = 0,
  SPECT_RADIO_STATE_REQUEST_LEADER,
  SPECT_RADIO_STATE_WAIT_RESPONSE,
  SPECT_RADIO_LEADER,
  SPECT_RADIO_CLIENT,
  SPECT_RADIO_STATE_MAX
} spect_radio_state_t;

typedef enum SpectRadioOpcode {
  SPECT_RADIO_NETWORK_UPDATE_NEW_LEADER      = 0x10,
  SPECT_RADIO_NETWORK_REQ_CURRENT_LEADER     = 0x11,
  SPECT_RADIO_NETWORK_RESP_CURRENT_LEADER    = 0x12,
  SPECT_RADIO_RGB_FILL                       = 0x20,
  SPECT_RADIO_RGB_PIXEL                      = 0x21,
  SPECT_RADIO_OP_MAX
} __attribute__ ((__packed__)) spect_radio_opcode_t;

// data per each packet type. Marked as packed as there is a
// size check in the decode function.

/**
 * @brief sent by the current leader to relinquish leadership
 * new leader need-not  be online currently.
 */
typedef struct SpectRadioNetworkUpdateNewLeader {
  spect_node_id_t new_node_id;
} __attribute__ ((__packed__)) spect_radio_network_update_new_leader_t;

/**
 * @brief Request the current leader. No arguments required.
 * 
 * A node should send this packet on boot to get the current
 * network's leader. If there is no response (in how long?)
 * it can claim the leader position?
 */
typedef struct SpectRadioNetworkRequestCurrentLeader {
} __attribute__ ((__packed__)) spect_radio_network_request_current_leader_t;

/**
 * @brief Response to the request current leader command
 * every node that currently knows the leader should respond
 * if it can.
 */
typedef struct SpectRadioNetworkResponseCurrentLeader {
  spect_node_id_t node_id;
} __attribute__ ((__packed__)) spect_radio_network_response_current_leader_t;

/**
 * @brief LED Fill command
 */
typedef struct SpectRadioRGBFill {
  uint8_t channel;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t _unused;
} __attribute__ ((__packed__)) spect_radio_rgb_fill_t;

typedef struct SpectRadioRGBPixel {
  uint8_t address;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t _unused;
} __attribute__ ((__packed__)) spect_radio_rgb_pixel_t;

typedef union {
  spect_radio_network_update_new_leader_t       new_leader;
  spect_radio_network_request_current_leader_t  request_current_leader;
  spect_radio_network_response_current_leader_t response_current_leader;
  spect_radio_rgb_fill_t                        fill;
  spect_radio_rgb_pixel_t                       pixel;
} spect_radio_packet_data_t;

typedef struct SpectRadioPacket {
  spect_node_t*             sender;
  spect_radio_opcode_t      opcode;
  spect_radio_packet_data_t data;
  uint16_t                  rssi;

  // private:
  uint8_t                   payload_length;
  uint8_t                   payload[SPECT_RADIO_MAX_DATA_LENGTH];
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
esp_err_t spect_radio_send_packet(spect_radio_packet_t* packet);
esp_err_t spect_radio_broadcast_state(spect_config_context_t* config_ctx, rgb_t* rgb);

#endif