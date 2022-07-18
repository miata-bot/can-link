#include <stdio.h>
#include "esp_log.h"

#include "spect-radio.h"
#include "sx1231.h"

SX1231_t* sx1231;
spect_radio_packet_t* packet;

typedef struct
{
  int version;
  uint8_t node_id;
  uint8_t network_id;
} configuration;

esp_err_t spect_packet_decode_init(spect_radio_packet_t** packet)
{
  packet = malloc(sizeof(spect_radio_packet_t));
  if(!packet) return ESP_ERR_NO_MEM;
  return ESP_OK;
}

esp_err_t spect_packet_decode(spect_config_context_t* config_ctx,
                              char* data, 
                              uint8_t length, 
                              spect_node_id_t sender_id,
                              spect_radio_packet_t* packet
)
{
  memset(packet->data, sizeof(spect_radio_packet_data_t));

  /* Load the node from the database. If not found, error */
  spect_node_t* node = spect_config_lookup_node(config_ctx, sender_id);
  if(!node) {
    ESP_LOGE(TAG, "Unknown node %d", sender_id);
    return ESP_ERROR_INVALID_STATE;
  }
  packet->sender = node;
  
  /* unlikely, but opcode might not fit in data in a currupt packet */
  assert(length > 1);
  spect_radio_opcode_t opcode = data[0];

  /* each opcode has it's own data, decode it */
  switch(opcode)
  {
    case SPECT_RADIO_NETWORK_UPDATE_NEW_LEADER: {
      assert(length == sizeof(spect_radio_network_update_new_leader_t) + 1);
      packet->data.new_leader.new_node_id = data[2] | (data[1] << 8);
    } break;

    case SPECT_RADIO_RGB_FILL: {
      assert(length == sizeof(spect_radio_rgb_fill_t) + 1);
      packet->data.fill.channel = data[1];
      packet->data.fill.red     = data[2];
      packet->data.fill.green   = data[3];
      packet->data.fill.blue    = data[4];
      packet->data.fill.unused  = data[5];
    } break;

    default: 
      ESP_LOGI(TAG, "unknown packet opcode %02X", data[0]);
      return ESP_ERR_INVALID_ARG;
  }
  return ESP_OK;
}

esp_err_t spect_radio_initialize(spect_config_context_t* config_ctx, SX1231_config_t* cfg)
{
  esp_err_t err;
  err = sx1231_initialize(cfg,  &sx1231);
  if(err != ESP_OK) return err;

  err = spect_packet_decode_init(&packet);
  if(err != ESP_OK) return err;

  ESP_LOGI("RADIO", "Initalized radio");

  return ESP_OK;
}

/**
 * General radio state machine operation:
 * 
 * # if the current node is the leader #
 *  this node should broadcast sync commands out to the networks
 *  the current leader should be the only node to relinquish leadership of
 *  the network. in practice, this of course won't be enforcable without
 *  some sort of external validation. This would also cause a deadlock 
 *  if the current leader is not online. 
 * 
 * # if the current node is not the leader #
 *  this node should accept sync commands from the network. This node
 *  should not try to send a network update, but only accept them.
 * 
 * messages outside of this sate should be ignored, but not crash.
 */
esp_err_t spect_radio_loop(spect_config_context_t* config_ctx)
{
  esp_err_t err;

  // if(sx1231_sendWithRetry(sx1231, 2, "ABCD", 4, 3, 10)) {
  //     ESP_LOGI("RADIO", "got ack");
  // }
  if(sx1231_receiveDone(sx1231)) {
    ESP_LOGI("RADIO", "SENDER=%d RSSI=%d dbm", sx1231->SENDERID, sx1231->RSSI);
    ESP_LOG_BUFFER_HEXDUMP("radio payload", sx1231->DATA, sx1231->DATALEN, ESP_LOG_INFO);
    err = spect_packet_decode(config_ctx,
                              sx1231->DATA, 
                              sx1231->DATALEN, 
                              sx1231->SENDERID, 
                              sx1231->RSSI, 
                              packet
    );
    if(err != ESP_OK) return err;

    switch(packet.opcode) {
      case SPECT_RADIO_NETWORK_UPDATE_NEW_LEADER: {
        /* update the current network leader, this essential changes the mode of 
         * radio operation. */
      } break;

      case SPECT_RADIO_RGB_FILL: {
        /* only execute this if this node is the leader */
      } break;

      default:
        ESP_LOGE(TAG, "unknown packet %d", sx1231->DATA[0]);
        break;
    }
  }
  return ESP_OK;
}
