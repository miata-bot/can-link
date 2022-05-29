#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "command.h"
#include "logging.h"

void command_init(command_t* command) 
{
  /// initialize SPI, set slave mode
  spi_init(command->spi, 1000*1000);
  spi_set_slave(command->spi, true);
  spi_set_format(command->spi, 8, SPI_CPHA_0, SPI_CPOL_0, SPI_MSB_FIRST);

  gpio_set_function(command->gpio_rx, GPIO_FUNC_SPI);
  gpio_set_function(command->gpio_tx, GPIO_FUNC_SPI);
  gpio_set_function(command->gpio_sclk, GPIO_FUNC_SPI);
  gpio_set_function(command->gpio_csn, GPIO_FUNC_SPI);
}

void command_sync(command_t* command)
{
  memset(command->buffer, 0, 4);
  while(command->buffer[0] != COMMAND_SYNC) {
    PICO_LOGI("Waiting sync");
    spi_read_blocking(command->spi, COMMAND_SYNC, command->buffer, 1);
    PICO_LOGI("SYNC WORD: %02X", command->buffer[0]);
  }
  // clear the buffer
  memset(command->buffer, 0, 4);
}

void command_read(command_t* command)
{
  // zero out the rx buffer
  memset(command->buffer, 0, 4);

  // this will get clocked out on the SPI bus
  command->response = COMMAND_RESPONSE_BUSY;
  
  // read 4 bytes from the SPI bus
  spi_read_blocking(command->spi, command->response, command->buffer, 4);

  // decode the payload
  command_decode(command);
}

void command_decode(command_t* command)
{
  command->response = COMMAND_RESPONSE_BUSY;

  // zero out the args for good luck
  memset(&command->args, 0, sizeof(command_args_t));

  // get the type out of the top 4 bits
  command->type = (command_type_t)command->buffer[0] >> 4;
  if(command->buffer[0] == COMMAND_SYNC)
    command->type = COMMAND_SYNC;
  PICO_LOGI("command=%d", command->type);

  // index is the next 4 bits, followed by 3 optional arguments, 8 bits each

  switch(command->type) {
    case COMMAND_SYNC: {
      PICO_LOGI("command=COMMAND_SYNC");
      command->response = COMMAND_RESPONSE_SYNC;
    } break;
    case COMMAND_STATUS_LED_SET_STATE: {
      PICO_LOGI("command=COMMAND_STATUS_LED_SET_STATE");
      command->args.status_led_set_state.index = (command_arg_index_t) command->buffer[0] & 0x0f,
      command->args.status_led_set_state.state = (status_led_state_t) command->buffer[1];
    } break;

    case COMMAND_STATUS_LED_BLINK: {
      PICO_LOGI("command=COMMAND_STATUS_LED_BLINK");
      command->args.status_led_blink.index = (command_arg_index_t) command->buffer[0] & 0x0f;
      command->args.status_led_blink.blink_ms = command->buffer[1];
    } break;

    case COMMAND_REGULATOR_SET_STATE: {
      PICO_LOGI("command=COMMAND_REGULATOR_SET_STATE");
      command->args.regulator_set_state.index = (command_arg_index_t) command->buffer[0] & 0x0f;
      command->args.regulator_set_state.state = (regulator_state_t) command->buffer[1];
    } break;

    case COMMAND_RGB_SET_COLOR: {
      PICO_LOGI("command=COMMAND_RGB_SET_COLOR");
      command->args.rgb_set_color.index = (command_arg_index_t) command->buffer[0] & 0x0f;
      command->args.rgb_set_color.r = command->buffer[1];
      command->args.rgb_set_color.g = command->buffer[2];
      command->args.rgb_set_color.b = command->buffer[3];
    } break;

    case COMMAND_RGB_SET_BRIGHTNESS: {
      PICO_LOGI("command=COMMAND_RGB_SET_BRIGHTNESS");
      command->args.rgb_set_brightness.index = (command_arg_index_t) command->buffer[0] & 0x0f;
      command->args.rgb_set_brightness.brightness = command->buffer[1];
    } break;

    case COMMAND_RGB_ENABLE: {
      PICO_LOGI("command=COMMAND_RGB_ENABLE");
      command->args.rgb_enable.index = (command_arg_index_t) command->buffer[0] & 0x0f;
    } break;

    case COMMAND_RGB_DISABLE: {
      PICO_LOGI("command=COMMAND_RGB_DISABLE");
      command->args.rgb_disable.index = (command_arg_index_t) command->buffer[0] & 0x0f;
    } break;

    default: {
      PICO_LOGI("command=UNKNOWN {%02X, %02X, %02X, %02X}", command->buffer[0], command->buffer[1], command->buffer[2], command->buffer[3]);
      command->response = COMMAND_RESPONSE_ERROR_UNKNOWN_COMMAND;
    } break;
  }
}

void command_reply(command_t* command)
{
  // clock out the response, read the sync word (ignored)
  spi_read_blocking(command->spi, command->response, command->buffer, 1);
}
