#include "console_regulator.h"

// static const char *TAG = "CONSOLE_REGULATOR";

extern reg_t* motor_reg;

static struct {
    struct arg_int *index;
    struct arg_int *state;
    struct arg_end *end;
} reg_args;

static int console_reg(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &reg_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, reg_args.end, argv[0]);
        return 1;
    }
    if(reg_args.state->ival[0] == 1) {
        reg_enable(motor_reg);
    } else {
        reg_disable(motor_reg);
    }
    return 0;
}

esp_err_t console_reg_install()
{
    reg_args.index = arg_intn("i", "index", "1|2", 0,1, "reg index 1 or 2");
    reg_args.state = arg_intn("s", "state", "1|0", 1, 1, "reg state");
    reg_args.end = arg_end(1);

    const esp_console_cmd_t reg_cmd = {
        .command = "reg",
        .help = "set the reg state",
        .hint = NULL,
        .func = &console_reg,
        .argtable = &reg_args
    };
    return esp_console_cmd_register(&reg_cmd);
}