#ifndef ARGEBEE_H
#define ARGEBEE_H

#include <led_strip.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// #define CONFIG_ARGEBEE_DEBUG

#ifdef CONFIG_ARGEBEE_DEBUG
#define log_location stderr
#define argebee_debug(...) do { fprintf(log_location, __VA_ARGS__); fprintf(log_location, "\r\n"); fflush(log_location); } while(0)
#define argebee_error(...) do { debug(__VA_ARGS__); } while (0)
#else
#define argebee_debug(...)
#define argebee_error(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#endif

#define OP_NOP  0x0
#define OP_HALT 0x1

#define OP_LD   0x2
#define OP_INC  0x3
#define OP_DEC  0x4

#define OP_CLS   0x5
#define OP_SET_A 0x6
#define OP_SET_R 0x7
#define OP_FLUSH 0x8
#define OP_FILL  0x9

#define OP_JP    0xA
#define OP_JPNZ  0xB

#define REG_X 0x0
#define REG_Y 0x1
#define REG_ZA 0x2
#define REG_ZB 0x3
#define REG_LP 0x4
#define REG_TM 0x5
#define REG_CHN 0x6

typedef struct ARGEBEE_CONFIG {  
} argebee_cfg_t;

typedef struct ARGEBEE {
  argebee_cfg_t cfg;
  uint16_t PC;
  uint32_t X;
  uint32_t Y;
  uint32_t ZA;
  uint32_t ZB;
  uint32_t LP;
  uint32_t TM;
  uint32_t CHN;
  // probably enough instructions
  uint8_t program[0xffff];
  bool halted;
  led_strip_t* strip;
  led_strip_t** channels;
  // uint8_t numChannels;
  uint8_t numChannels;
} argebee_t;

void argebee_init(argebee_cfg_t* cfg, argebee_t** out_engine);
void argebee_load_program(argebee_t* engine, uint8_t* program, uint16_t size);
void argebee_step(argebee_t* engine);

void argebee_ld(argebee_t* engine, uint8_t reg, uint32_t value);
void argebee_inc(argebee_t* engine, uint8_t reg);
void argebee_dec(argebee_t* engine, uint8_t reg);
void argebee_cls(argebee_t* engine, uint8_t reg);
void argebee_set(argebee_t* engine, uint8_t r1, uint8_t r2, uint8_t r3);
void argebee_flush(argebee_t* engine);
void argebee_fill(argebee_t* engine, uint8_t r1, uint8_t r2);
uint32_t argebee_read_register(argebee_t* engine, uint8_t r1);

void hexdump(const void* data, size_t size);

#endif