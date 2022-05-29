#include "ws2812.h"


typedef void (*pattern)(ws2812_t*, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {ws2812_pattern_snakes,  "Snakes!"},
        {ws2812_pattern_random,  "Random data"},
        {ws2812_pattern_sparkle, "Sparkles"},
        {ws2812_pattern_greys,   "Greys"},
};

void ws2812_init(ws2812_t* ws2812)
{
    ws2812->sm = 0;
    ws2812->offset = pio_add_program(ws2812->pio, &ws2812_program);
    ws2812_program_init(ws2812->pio, ws2812->sm, ws2812->offset, ws2812->gpio, ws2812->frequency, ws2812->is_rgbw);
    for(uint8_t i = 0; i < ws2812->num_nodes; i++)
      ws2812_put_pixel(ws2812, 0);

    // int t = 0;
    // while (1) {
    //     printf("t=%d\n", t);
    //     int pat = rand() % count_of(pattern_table);
    //     int dir = (rand() >> 30) & 1 ? 1 : -1;
    //     puts(dir == 1 ? "(forward)" : "(backward)");
    //     for (int i = 0; i < 1000; ++i) {
    //         pattern_table[pat].pat(ws2812, t);
    //         sleep_ms(10);
    //         t += dir;
    //     }
    // }
}

void ws2812_put_pixel(ws2812_t* ws2812, uint32_t pixel_grb) {
    pio_sm_put_blocking(ws2812->pio, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void ws2812_pattern_snakes(ws2812_t* ws2812, uint t) {
    for (uint i = 0; i < ws2812->num_nodes; ++i) {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10)
            ws2812_put_pixel(ws2812, urgb_u32(0xff, 0, 0));
        else if (x >= 15 && x < 25)
            ws2812_put_pixel(ws2812, urgb_u32(0, 0xff, 0));
        else if (x >= 30 && x < 40)
            ws2812_put_pixel(ws2812, urgb_u32(0, 0, 0xff));
        else
            ws2812_put_pixel(ws2812, 0);
    }
}

void ws2812_pattern_random(ws2812_t* ws2812, uint t) {
    if (t % 8)
        return;
    for (int i = 0; i < ws2812->num_nodes; ++i)
        ws2812_put_pixel(ws2812, rand());
}

void ws2812_pattern_sparkle(ws2812_t* ws2812, uint t) {
    if (t % 8)
        return;
    for (int i = 0; i < ws2812->num_nodes; ++i)
        ws2812_put_pixel(ws2812, rand() % 16 ? 0 : 0xffffffff);
}

void ws2812_pattern_greys(ws2812_t* ws2812, uint t) {
    int max = 100; // let's not draw too much current!
    t %= max;
    for (int i = 0; i < ws2812->num_nodes; ++i) {
        ws2812_put_pixel(ws2812, t * 0x10101);
        if (++t >= max) t = 0;
    }
}