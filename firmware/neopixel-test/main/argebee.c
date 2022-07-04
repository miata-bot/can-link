#include "argebee.h"

const char* regmap[7] = {
    "X",
    "Y",
    "ZA",
    "ZB",
    "LP",
    "TM",
    "CHN"
};

static const rgb_t colors[] = {
    { .r = 0x0f, .g = 0x0f, .b = 0x0f },
    { .r = 0x00, .g = 0x00, .b = 0xff },
    { .r = 0x00, .g = 0xff, .b = 0x00 },
    { .r = 0xff, .g = 0x00, .b = 0x00 },
    { .r = 0x00, .g = 0x00, .b = 0x00 },
};

led_strip_t strip;

void argebee_init(argebee_cfg_t* cfg, argebee_t** out_engine)
{
    strip = (led_strip_t) {
        .type = LED_STRIP_WS2812,
        .is_rgbw = false,
        .length = 300,
        .gpio = 8,
        .buf = NULL,
        .brightness = 255,
    };
    led_strip_install();
    ESP_ERROR_CHECK(led_strip_init(&strip));
    ESP_ERROR_CHECK(led_strip_fill(&strip, 0, strip.length, colors[0]));
    ESP_ERROR_CHECK(led_strip_flush(&strip));

    argebee_t* engine = (argebee_t*)malloc(sizeof(argebee_t));
    if(!engine)
        return;

    *engine = (argebee_t) {
        .cfg = *cfg,
        .halted = false,
        .PC = 0,
        .X = 0,
        .Y = 0,
        .ZA = 0,
        .ZB = 0,
        .LP = 0,
        .TM = 0,
        .CHN = 0,
        .numChannels = 1,
        .strip = &strip
    };
    *out_engine = engine;
}

void argebee_load_program(argebee_t* engine, uint8_t* program, uint16_t size)
{
    memset(&engine->program, 0, sizeof(uint16_t));
    memcpy(engine->program, program, size);
    argebee_debug("loading program size=%d\n", size);
    hexdump(engine->program, size);
}

void argebee_step(argebee_t* engine)
{
    if(engine->halted) {
        argebee_debug("tried to step while halted\n");
        return;
    }

    uint8_t opcode = engine->program[engine->PC++];
    switch(opcode) {
    case OP_NOP : {
        argebee_debug("NOP\n");
        break;
    }

    case OP_HALT : {
        argebee_debug("HALT\n");
        engine->halted = true;
        break;
    }

    case OP_LD : {
        uint8_t reg = engine->program[engine->PC++];
        uint8_t arr[4];
        arr[0] = engine->program[engine->PC++];
        arr[1] = engine->program[engine->PC++];
        arr[2] = engine->program[engine->PC++];
        arr[3] = engine->program[engine->PC++];
        uint32_t value = (arr[0] << 24) |
                         (arr[1] << 16) |
                         (arr[2] << 8)  |
                         arr[3];
        argebee_debug("LD %s %8X\n", regmap[reg], value);
        argebee_ld(engine, reg, value);
        break;
    }

    case OP_INC: {
        uint8_t reg = engine->program[engine->PC++];
        argebee_debug("INC %s\n", regmap[reg]);
        argebee_inc(engine, reg);
        break;
    }

    case OP_DEC: {
        uint8_t reg = engine->program[engine->PC++];
        argebee_debug("DEC %s\n", regmap[reg]);
        argebee_dec(engine, reg);
        break;
    }

    case OP_CLS: {
        uint8_t reg = engine->program[engine->PC++];
        argebee_debug("CLS %s\n", regmap[reg]);
        argebee_cls(engine, reg);
        break;
    }

    case OP_SET_R: {
        uint8_t channel    = engine->program[engine->PC++];
        uint8_t address    = engine->program[engine->PC++];
        uint8_t color    = engine->program[engine->PC++];
        argebee_debug("SET %s %s %s\n", regmap[channel], regmap[address], regmap[color]);
        argebee_set(engine, channel, address, color);
        break;
    }

    case OP_FLUSH: {
        argebee_debug("FLUSH\n");
        argebee_flush(engine);
        break;
    }

    case OP_FILL: {
        uint8_t r1 = engine->program[engine->PC++];
        uint8_t r2 = engine->program[engine->PC++];
        argebee_debug("FILL %s %s\n", regmap[r1], regmap[r2]);
        argebee_fill(engine, r1, r2);
        break;
    }

    case OP_JP: {
        argebee_debug("JP\n");
        uint8_t arr[2];
        arr[0] = engine->program[engine->PC++];
        arr[1] = engine->program[engine->PC++];
        uint32_t address = (arr[0] << 8) | arr[1];
        engine->PC = address;
        break;
    }

    case OP_JPNZ: {
        uint8_t r1 = engine->program[engine->PC++];
        uint8_t arr[2];
        arr[0] = engine->program[engine->PC++];
        arr[1] = engine->program[engine->PC++];
        uint32_t address = (arr[0] << 8) | arr[1];
        argebee_debug("JNZ %s %2X\n", regmap[r1], address);
        uint32_t register_value = argebee_read_register(engine, r1);
        if(register_value != 0) {
            argebee_debug("jumping to %d\n", address);
            engine->PC = address;
        }
        break;
    }

    default: {
        argebee_debug("unknown instruction %02X\n", opcode);
        engine->halted = true;
        break;
    };
    }
}

uint32_t argebee_read_register(argebee_t* engine, uint8_t r1)
{
    argebee_debug("read_reg(%s)", regmap[r1]);
    switch(r1) {
    case REG_X:
        return engine->X;
    case REG_Y:
        return engine->Y;
    case REG_ZA:
        return engine->ZA;
    case REG_ZB:
        return engine->ZB;
    case REG_LP:
        return engine->LP;
    case REG_TM:
        return engine->TM;
    case REG_CHN:
        return engine->CHN;
    default: {
        argebee_debug("unknown register %2X\n", r1);
        engine->halted = true;
        return 0;
    }
    }
}

rgb_t argebee_read_color_from_register(argebee_t* engine, uint8_t r)
{
    uint32_t color = argebee_read_register(engine, r);
    rgb_t color_;
    // color_.red = color  & 0xff; // blue
    color_.blue = (color >> 8) & 0xff; // green
    color_.red = (color >> 16) & 0xff; // red
    color_.green = (color >> 24) & 0xff; // a
    fprintf(stderr, "reg=%04X rgb=%02X %02X %02X\r\n", color, color_.red, color_.green, color_.blue);
    return color_;
}

/*
 * Instruction Implementations
 */

void argebee_fill(argebee_t* engine, uint8_t r1, uint8_t r2)
{
    // uint32_t channel = argebee_read_register(engine, r1);
    // led_strip_t* strip = engine->channels[channel];
    rgb_t color = argebee_read_color_from_register(engine, r2);
    led_strip_t* strip = engine->strip;
    led_strip_fill(strip, 0, strip->length, color);
}

void argebee_flush(argebee_t* engine)
{
    ESP_ERROR_CHECK(led_strip_flush(engine->strip));

    // for(size_t i = 0; i < engine->numChannels; i++) {
    //     ESP_ERROR_CHECK(led_strip_flush(engine->channels[i]));
    // }
}

void argebee_set(argebee_t* engine, uint8_t r1, uint8_t r2, uint8_t r3) {
    // uint32_t channel = argebee_read_register(engine, r1);
    // led_strip_t* strip = engine->channels[channel];
    led_strip_t* strip = engine->strip;

    uint16_t address = argebee_read_register(engine, r2);
    rgb_t color = argebee_read_color_from_register(engine, r3);
    ESP_ERROR_CHECK(led_strip_set_pixel(strip, address, color));
}

void argebee_cls(argebee_t* engine, uint8_t r1) {
    // uint32_t channel = argebee_read_register(engine, r1);
    // led_strip_t* strip = engine->channels[channel];
    led_strip_t* strip = engine->strip;
    ESP_ERROR_CHECK(led_strip_fill(strip, 0, strip->length, colors[4]));
}

void argebee_inc(argebee_t* engine, uint8_t reg) {
    switch(reg) {
    case REG_X: {
        engine->X += 1;
        break;
    }
    case REG_Y: {
        engine->Y += 1;
        break;
    }
    case REG_ZA: {
        engine->ZA += 1;
        break;
    }
    case REG_ZB: {
        engine->ZB += 1;
        break;
    }
    case REG_LP: {
        engine->LP += 1;
        break;
    }
    case REG_TM: {
        engine->TM += 1;
        break;
    }
    case REG_CHN: {
        engine->CHN += 1;
        break;
    }
    default: {
        argebee_debug("unknown register %2X\n", reg);
        exit(EXIT_FAILURE);
    }
    }
}
void argebee_dec(argebee_t* engine, uint8_t reg) {
    switch(reg) {
    case REG_X: {
        engine->X -=1;
        break;
    }
    case REG_Y: {
        engine->Y -=1;
        break;
    }
    case REG_ZA: {
        engine->ZA -=1;
        break;
    }
    case REG_ZB: {
        engine->ZB -=1;
        break;
    }
    case REG_LP: {
        engine->LP -=1;
        break;
    }
    case REG_TM: {
        engine->TM -=1;
        break;
    }
    case REG_CHN: {
        engine->CHN -=1;
        break;
    }
    default: {
        argebee_debug("unknown register %2X\n", reg);
        exit(EXIT_FAILURE);
    }
    }
}

void argebee_ld(argebee_t* engine, uint8_t reg, uint32_t value) {
    switch(reg) {
    case REG_X: {
        engine->X = value;
        break;
    }
    case REG_Y: {
        engine->Y = value;
        break;
    }
    case REG_ZA: {
        engine->ZA = value;
        break;
    }
    case REG_ZB: {
        engine->ZB = value;
        break;
    }
    case REG_LP: {
        engine->LP = value;
        break;
    }
    case REG_TM: {
        engine->TM = value;
        break;
    }
    case REG_CHN: {
        engine->CHN = value;
        break;
    }
    default: {
        argebee_debug("unknown register %2X\n", reg);
        exit(EXIT_FAILURE);
    }
    }
}

void hexdump(const void* data, size_t size) {
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        printf("%02X ", ((unsigned char*)data)[i]);
        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i+1) % 8 == 0 || i+1 == size) {
            printf(" ");
            if ((i+1) % 16 == 0) {
                printf("|  %s \n", ascii);
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';
                if ((i+1) % 16 <= 8) {
                    printf(" ");
                }
                for (j = (i+1) % 16; j < 16; ++j) {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
}