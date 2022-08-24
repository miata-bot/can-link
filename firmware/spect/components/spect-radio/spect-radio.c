#include <stdio.h>
#include <inttypes.h>
#include "esp_log.h"

#include "spect-radio.h"
#include "sx1231.h"

const char* TAG = "RADIO";

#define SPECT_RADIO_INITIAL_STATE SPECT_RADIO_STATE_REQUEST_LEADER
#define SPECT_RADIO_INITIAL_LEADER 1

// #define SPECT_RADIO_INITIAL_STATE SPECT_RADIO_LEADER
// #define SPECT_RADIO_INITIAL_LEADER 1

/**
 * @brief Gloabal radio handle. Should not be shared
 */
SX1231_t* sx1231;

/**
 * @brief current packet for send/receive
 */
spect_radio_packet_t* packet;

/**
 * @brief Current network leader. 
 * this data is stored in the config context, but needs to be validated
 * on every boot. 
 */
spect_node_id_t leader_id = SPECT_RADIO_INITIAL_LEADER;

/**
 * @brief state machine sigil. Changes when events come/go
 */
spect_radio_state_t radio_state;
uint64_t timeout;

esp_err_t spect_radio_broadcast_state(spect_config_context_t* config_ctx, rgb_t* rgb)
{
  if(sx1231 == NULL) return ESP_OK;
  // bool current_node_is_leader   = config_ctx->config->network->identity->node_id == config_ctx->config->network->leader->node_id;
  bool current_node_is_leader   = config_ctx->config->network->identity->node_id == leader_id;
  if(current_node_is_leader) {
    memset(packet, 0, sizeof(spect_radio_packet_t));
    packet->opcode = SPECT_RADIO_RGB_FILL;
    packet->data.fill.channel = 0;
    packet->data.fill.red     = rgb->red;
    packet->data.fill.green   = rgb->green;
    packet->data.fill.blue    = rgb->blue;
    packet->data.fill._unused = 0;
    return spect_radio_send_packet(packet);
  } else {
    ESP_LOGE(TAG, "not leader, no state broadcast");
    return ESP_ERR_INVALID_STATE;
  }
}

esp_err_t spect_packet_decode_init(spect_radio_packet_t** out_packet)
{
  spect_radio_packet_t* ctx = malloc(sizeof(spect_radio_packet_t));
  if(!ctx) return ESP_ERR_NO_MEM;
  *out_packet = ctx;
  return ESP_OK;
}

esp_err_t spect_packet_decode(spect_config_context_t* config_ctx,
                              uint8_t*                data, 
                              uint8_t                 length, 
                              spect_node_id_t         sender_id,
                              uint16_t                rssi,
                              spect_radio_packet_t*   packet
)
{
  if(!packet) {
    ESP_LOGE(TAG, "packet not initialized");
    return ESP_ERR_INVALID_ARG;
  }

  memset(&packet->data, 0, sizeof(spect_radio_packet_data_t));

  /* Load the node from the database. If not found, error */
  spect_node_t* node = spect_config_lookup_node(config_ctx, sender_id);
  if(!node) {
    ESP_LOGE(TAG, "Unknown node %d", sender_id);
    return ESP_ERR_INVALID_STATE;
  }
  packet->sender = node;
  
  /* unlikely, but opcode might not fit in data in a currupt packet */
  assert(length >= 1);
  spect_radio_opcode_t opcode = data[0];
  ESP_LOGI(TAG, "got opcode=%d", data[0]);
  if(opcode >= SPECT_RADIO_OP_MAX) return ESP_ERR_INVALID_STATE;
  packet->opcode = opcode;

  /* each opcode has it's own data, decode it */
  switch(opcode)
  {
    case SPECT_RADIO_NETWORK_UPDATE_NEW_LEADER: {
      assert(length == sizeof(spect_radio_network_update_new_leader_t) + 1);
      packet->data.new_leader.new_node_id = data[2] | (data[1] << 8);
    } break;

    // careful with this, it only happens to work
    case SPECT_RADIO_RGB_FILL:
    case SPECT_RADIO_RGB_PIXEL: {
      assert(length == sizeof(spect_radio_rgb_fill_t) + 1);
      packet->data.fill.channel = data[1];
      packet->data.fill.red     = data[2];
      packet->data.fill.green   = data[3];
      packet->data.fill.blue    = data[4];
      packet->data.fill._unused = data[5];
    } break;

    // packet currently has no data assosiated with it.
    case SPECT_RADIO_NETWORK_REQ_CURRENT_LEADER: {
      assert(length == sizeof(spect_radio_network_request_current_leader_t) + 1);
    } break;

    case SPECT_RADIO_NETWORK_RESP_CURRENT_LEADER: {
      assert(length == sizeof(spect_radio_network_response_current_leader_t) + 1);
      packet->data.response_current_leader.node_id = data[1] | (data[2] << 8);
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
  err = spect_packet_decode_init(&packet);
  if(err != ESP_OK) return err;

  err = sx1231_initialize(cfg,  &sx1231);
  if(err != ESP_OK) {
    // should probably free this?
    sx1231 = NULL;
    return err;
  };

  // clear out state before entering the main loop
  radio_state = SPECT_RADIO_INITIAL_STATE;
  leader_id = SPECT_RADIO_INITIAL_LEADER;
  memset(packet->payload, 0, SPECT_RADIO_MAX_DATA_LENGTH);
  memset(&packet->data, 0, sizeof(spect_radio_packet_data_t));
  
  ESP_LOGI("RADIO", "Initalized radio %lu", sx1231_getFrequency(sx1231));
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
  if(sx1231 == NULL) return ESP_OK;

  if(!packet) {
    ESP_LOGE(TAG, "packet not initialized");
    return ESP_ERR_INVALID_ARG;
  }
  // char payload[8] = {0};
  // sx1231_send(sx1231, 
  //             SPECT_RADIO_BROADCAST_ADDR, 
  //             payload, 
  //             8, 
  //             false);

  esp_err_t err;
  if(radio_state == SPECT_RADIO_STATE_REQUEST_LEADER) {
    memset(packet, 0, sizeof(spect_radio_packet_t));
    memset(&packet->data, 0, sizeof(spect_radio_packet_data_t));
    packet->sender = config_ctx->config->network->identity->node;
    packet->opcode = SPECT_RADIO_NETWORK_REQ_CURRENT_LEADER;
    // packet->data = {0};
    // packet->rssi = 0;
    err = spect_radio_send_packet(packet);
    if(err != ESP_OK) return err;
    radio_state = SPECT_RADIO_STATE_WAIT_RESPONSE;
    timeout = esp_timer_get_time();

    // return early, next loop() will probably
    // have a packet available.
    return ESP_OK; 
  }

  if(radio_state == SPECT_RADIO_STATE_WAIT_RESPONSE) {
    if((esp_timer_get_time() - timeout) > 3000000) {
      ESP_LOGE(TAG, "timeout waiting for leader... trying again");
      radio_state = SPECT_RADIO_STATE_REQUEST_LEADER;
    }
  }

  if(sx1231_receiveDone(sx1231)) {
    ESP_LOGI("RADIO", "SENDER=%d RSSI=%d dbm", sx1231->SENDERID, sx1231->RSSI);
    ESP_LOG_BUFFER_HEXDUMP("radio payload in", sx1231->DATA, sx1231->DATALEN, ESP_LOG_INFO);
    err = spect_packet_decode(config_ctx,
                              sx1231->DATA, 
                              sx1231->DATALEN, 
                              sx1231->SENDERID, 
                              sx1231->RSSI, 
                              packet
    );
    if(err != ESP_OK) return err;

    // is this even possible?
    // bool packet_comes_from_self   = packet->sender->id == config_ctx->config->network->identity->node_id;
    // if(packet_comes_from_self) return ESP_OK;

    // command can only come from the leader.
    // TODO: how to change leader when leader offline?
    // bool packet_comes_from_leader = packet->sender->id == config_ctx->config->network->leader->node_id;
    bool packet_comes_from_leader = packet->sender->id == leader_id;
    
    // if this device is the leader, what do?
    // bool current_node_is_leader   = config_ctx->config->network->identity->node_id == config_ctx->config->network->leader->node_id;
    bool current_node_is_leader   = config_ctx->config->network->identity->node_id == leader_id;
    ESP_LOGI(TAG, "packet from node=%d current node=%d leader=%d", packet->sender->id, config_ctx->config->network->identity->node_id, config_ctx->config->network->leader->node_id);

    switch(packet->opcode) {
      case SPECT_RADIO_NETWORK_REQ_CURRENT_LEADER: {
        // only respond to this packet if we know the current leader
        // idea: respond with 0 if the leader is unknown so the node
        // who sent this request can know about the topology?
        if(radio_state > SPECT_RADIO_STATE_WAIT_RESPONSE) {
          memset(&packet->data, 0, sizeof(spect_radio_packet_data_t));
          packet->sender = config_ctx->config->network->identity->node;
          packet->opcode = SPECT_RADIO_NETWORK_RESP_CURRENT_LEADER;
          packet->data.response_current_leader.node_id = leader_id;
          // packet->rssi = 0;
          err = spect_radio_send_packet(packet);
          if(err != ESP_OK) return err;
        } else {
          ESP_LOGE(TAG, "Request for leader, but don't know the leader");
        }
      } break;

      case SPECT_RADIO_NETWORK_UPDATE_NEW_LEADER:
      case SPECT_RADIO_NETWORK_RESP_CURRENT_LEADER: {
        ESP_LOGI(TAG, "got network leader");

        // todo: save this to DB
        leader_id = packet->data.response_current_leader.node_id;
        
        // if this node is the leader (is this possible?)
        if(leader_id == config_ctx->config->network->identity->node_id) {
          ESP_LOGI(TAG, "current node is leader!");
          radio_state = SPECT_RADIO_LEADER;
        } else {
          ESP_LOGI(TAG, "leader=%d", packet->data.response_current_leader.node_id);
          radio_state = SPECT_RADIO_CLIENT;
        }
      } break;

      case SPECT_RADIO_RGB_FILL: {
        /* only execute this if this node is a client node and packet comes from leader */
        if(packet_comes_from_leader && !current_node_is_leader) {
          ESP_LOGI(TAG, "accepting fill command channel=%02X red=%02X green=%02X blue=%02X", 
            packet->data.fill.channel,
            packet->data.fill.red,
            packet->data.fill.green,
            packet->data.fill.blue
          );
          rgb_t color = {.green=packet->data.fill.green, .red=packet->data.fill.red, .blue=packet->data.fill.blue};
          // spect_rgb_fill(config_ctx->rgb0, 0, config_ctx->rgb0->strip->length, color);
          for(uint8_t i = 0; i < config_ctx->rgb0->strip->length; i++) {
            spect_rgb_set_pixel(config_ctx->rgb0, i, color);
            spect_rgb_wait(config_ctx->rgb0);
            spect_rgb_blit(config_ctx->rgb0);
          }
        } else {
          ESP_LOGE(TAG, "not accepting comand: packet_comes_from_leader=%d current_node_is_leader=%d", packet_comes_from_leader, current_node_is_leader);
        }
      } break;

      default:
        ESP_LOGE(TAG, "unknown packet %d", sx1231->DATA[0]);
        break;
    }
  }
  return ESP_OK;
}

esp_err_t spect_radio_send_packet(spect_radio_packet_t* packet)
{
  if(!packet) {
    ESP_LOGE(TAG, "packet not initialized");
    return ESP_ERR_INVALID_ARG;
  }

  switch(packet->opcode) {
    case SPECT_RADIO_NETWORK_UPDATE_NEW_LEADER: {
      packet->payload_length = sizeof(spect_radio_network_update_new_leader_t) + 1;
      packet->payload[0] = SPECT_RADIO_NETWORK_UPDATE_NEW_LEADER;
      packet->payload[1] = packet->data.new_leader.new_node_id & 0xFF;
      packet->payload[2] = (packet->data.new_leader.new_node_id >> 8);
    } break;

    case SPECT_RADIO_NETWORK_REQ_CURRENT_LEADER: {
      packet->payload_length = sizeof(spect_radio_network_request_current_leader_t) + 1;
      packet->payload[0] = SPECT_RADIO_NETWORK_REQ_CURRENT_LEADER;
    } break;

    case SPECT_RADIO_NETWORK_RESP_CURRENT_LEADER: {
      packet->payload_length = sizeof(spect_radio_network_response_current_leader_t) + 1;
      packet->payload[0] = SPECT_RADIO_NETWORK_RESP_CURRENT_LEADER;
      packet->payload[1] = packet->data.response_current_leader.node_id & 0xFF;
      packet->payload[2] = (packet->data.response_current_leader.node_id >> 8);
    } break;

    case SPECT_RADIO_RGB_FILL: {
      packet->payload_length = sizeof(spect_radio_rgb_fill_t) + 1;
      packet->payload[0] = SPECT_RADIO_RGB_FILL;
      packet->payload[1] = packet->data.fill.channel;
      packet->payload[2] = packet->data.fill.red;
      packet->payload[3] = packet->data.fill.green;
      packet->payload[4] = packet->data.fill.blue;
      packet->payload[5] = packet->data.fill._unused;
    } break;

    default:
      ESP_LOGE(TAG, "unknown packet type %d. not sending", packet->opcode);
      return ESP_ERR_INVALID_ARG;
  }

  ESP_LOG_BUFFER_HEXDUMP("radio payload out", 
                         packet->payload, 
                         packet->payload_length, 
                         ESP_LOG_INFO);
  sx1231_send(sx1231, 
              SPECT_RADIO_BROADCAST_ADDR, 
              packet->payload, 
              packet->payload_length, 
              false);

  /* Not sure about acking on the broadcast address */
  // sx1231_sendWithRetry();
  return ESP_OK;
}