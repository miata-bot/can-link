#include "console.h"
#include "console_rgb.h"
#include "console_motor.h"
#include "console_regulator.h"

#include "linenoise/linenoise.h"

esp_err_t console_initialize()
{
  esp_console_repl_t *repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();

  esp_console_register_help_command();
  console_motor_install();
  console_rgb_install(); 
  console_reg_install();

  repl_config.prompt = ">";
  repl_config.max_cmdline_length = 1024;
  repl_config.history_save_path = "/spiffs/history.txt";

  esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));
  ESP_ERROR_CHECK(esp_console_start_repl(repl));

  /* Tell linenoise where to get command completions and hints */
  linenoiseSetCompletionCallback(&esp_console_get_completion);
  linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

  /* Set command history size */
  linenoiseHistorySetMaxLen(100);

  /* Set command maximum length */
  linenoiseSetMaxLineLen(repl_config.max_cmdline_length);

  /* Don't return empty lines */
  linenoiseAllowEmpty(false);
  return ESP_OK;
}