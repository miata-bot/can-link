#include "can.h"

void twai_init()
{
    // //Initialize configuration structures using macro initializers
    // twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
    // twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    // twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // //Install TWAI driver
    // if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    //     printf("Driver installed\n");
    // } else {
    //     printf("Failed to install driver\n");
    //     return;
    // }

    // //Start TWAI driver
    // if (twai_start() == ESP_OK) {
    //     printf("Driver started\n");
    // } else {
    //     printf("Failed to start driver\n");
    //     return;
    // }
}

void twai_deinit()
{
    // //Stop the TWAI driver
    // if (twai_stop() == ESP_OK) {
    //     printf("Driver stopped\n");
    // } else {
    //     printf("Failed to stop driver\n");
    //     return;
    // }

    // //Uninstall the TWAI driver
    // if (twai_driver_uninstall() == ESP_OK) {
    //     printf("Driver uninstalled\n");
    // } else {
    //     printf("Failed to uninstall driver\n");
    //     return;
    // }
}