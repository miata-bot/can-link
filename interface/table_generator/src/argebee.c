#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

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
} argebee_t;

void argebee_ld(argebee_t* engine, uint8_t reg, uint32_t value);
void argebee_inc(argebee_t* engine, uint8_t reg);
void argebee_dec(argebee_t* engine, uint8_t reg);
void argebee_cls(argebee_t* engine, uint8_t reg);
void argebee_set(argebee_t* engine, uint8_t r1, uint8_t r2, uint8_t r3);
uint32_t argebee_read_register(argebee_t* engine, uint8_t r1);
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

const char* regmap[7] = {
  "X",
  "Y",
  "ZA",
  "ZB",
  "LP",
  "TM",
  "CHN"
};

void argebee_init(argebee_cfg_t* cfg, argebee_t** out_engine)
{
  argebee_t* engine = (argebee_t*)malloc(sizeof(argebee_t));
  if(!engine)
    return;
  
  *engine = (argebee_t){
    .cfg = *cfg,
    .halted = false,
    .PC = 0,
    .X = 0,
    .Y = 0,
    .ZA = 0,
    .ZB = 0,
    .LP = 0,
    .TM = 0,
    .CHN = 0
  };
  *out_engine = engine;
}

void argebee_load_program(argebee_t* engine, uint8_t* program, uint16_t size)
{
  memset(&engine->program, 0, sizeof(uint16_t));
  memcpy(engine->program, program, size);
  fprintf(stderr, "loading program size=%d\n", size);
  hexdump(engine->program, size);
}

void argebee_step(argebee_t* engine)
{
  if(engine->halted) {
    fprintf(stderr, "tried to step while halted\n");
    return;
  }

  uint8_t opcode = engine->program[engine->PC++];
  // fprintf(stderr, "step op=%2X pc=%2X\n", opcode, engine->PC);
  switch(opcode) {
    case OP_NOP : {
      fprintf(stderr, "NOP\n");
      engine->halted = true;
      // hexdump(engine->program, 0xffff);
      break;
    };
    case OP_HALT : {
      fprintf(stderr, "HALT\n");
      engine->halted = true;
      break;
    };
    case OP_LD : {
      uint8_t reg = engine->program[engine->PC++];
      uint32_t value = (engine->program[engine->PC++] << 24) | 
                       (engine->program[engine->PC++] << 16) | 
                       (engine->program[engine->PC++] << 8)  | 
                       engine->program[engine->PC++];
      fprintf(stderr, "LD %s %8X\n", regmap[reg], value);
      argebee_ld(engine, reg, value);
      break;
    };
    case OP_INC: {
      uint8_t reg = engine->program[engine->PC++];
      fprintf(stderr, "INC %s\n", regmap[reg]);
      argebee_inc(engine, reg);
      break;
    };
    case OP_DEC: {
      uint8_t reg = engine->program[engine->PC++];
      fprintf(stderr, "DEC %s\n", regmap[reg]);
      argebee_dec(engine, reg);
      break;
    };
    case OP_CLS: {
      uint8_t reg = engine->program[engine->PC++];
      fprintf(stderr, "CLS %s\n", regmap[reg]);
      argebee_cls(engine, reg);
      break;
    };
    case OP_SET_R: {
      uint8_t r1    = engine->program[engine->PC++];
      uint8_t r2    = engine->program[engine->PC++];
      uint8_t r3    = engine->program[engine->PC++];
      fprintf(stderr, "SET %s %s %s\n", regmap[r1], regmap[r2], regmap[r3]);
      argebee_set(engine, r1, r2, r2);
      break;
    };
    case OP_FLUSH: {
      fprintf(stderr, "FLUSH\n");
      break;
    }

    case OP_FILL: {
      uint8_t r1 = engine->program[engine->PC++];
      uint8_t r2 = engine->program[engine->PC++];
      fprintf(stderr, "FILL %s %s\n", regmap[r1], regmap[r2]);
      break;
    }
    case OP_JP: {
      fprintf(stderr, "JP\n");
      break;
    }
    case OP_JPNZ: {
      uint8_t r1 = engine->program[engine->PC++];
      uint32_t address = (engine->program[engine->PC++] << 8)  | 
                  engine->program[engine->PC++];
      fprintf(stderr, "JNZ %s %2X\n", regmap[r1], address);
      uint32_t register_value = argebee_read_register(engine, r1);
      if(register_value != 0) {
        fprintf(stderr, "jumping to %d\n", address);
        engine->PC = address;
      }
      break;
    }
    default: {
      fprintf(stderr, "unknown instruction %02X\n", opcode);
      engine->halted = true;
      break;
    }; 

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
      fprintf(stderr, "unknown register %2X\n", reg);
      exit(EXIT_FAILURE);
    }
  }
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
      fprintf(stderr, "unknown register %2X\n", reg);
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
      fprintf(stderr, "unknown register %2X\n", reg);
      exit(EXIT_FAILURE);
    }
  }
}
void argebee_cls(argebee_t* engine, uint8_t reg) {

}
void argebee_set(argebee_t* engine, uint8_t r1, uint8_t r2, uint8_t r3) {}

enum { NS_PER_SECOND = 1000000000 };

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

int main(int argc, char const *argv[])
{
  if(argc != 2) {
    fprintf(stderr, "usage: %s bytecodefile\n", argv[0]);
    return EXIT_FAILURE;
  }

  argebee_cfg_t cfg = {};
  argebee_t* engine;
  argebee_init(&cfg, &engine);
  
  FILE *fp = fopen(argv[1], "rb");
  if(!fp) {
    fprintf(stderr, "could not open file\n");
    return EXIT_FAILURE;
  }

  uint8_t source[0xffff];
  memset(source, 0, sizeof(source));

  fseek(fp, 0L, SEEK_END);
  uint16_t program_size = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  rewind(fp);
  fread(source, sizeof(source),1, fp); // read 10 bytes to our buffer
  fclose(fp);

  fprintf(stderr, "loading program %d\n", program_size);
  argebee_load_program(engine, source, program_size);

  fprintf(stderr, "starting program %p %d\n", engine, engine->halted);

 
  struct timespec t1; 
  struct timespec t2;
  if(clock_gettime(CLOCK_MONOTONIC, &t1)) {
    fprintf(stderr, "failed to get time1\n");
    return EXIT_FAILURE;
  }
  while(engine->halted == false) {

  argebee_step(engine);

  // sleep(1);
  // usleep(1000);
  if(clock_gettime(CLOCK_MONOTONIC, &t2)) {
    fprintf(stderr, "failed to get time2\n");
    return EXIT_FAILURE;
  }
  #define MILLION 1000000
  int elapsedTime = (MILLION * (t2.tv_sec - t1.tv_sec) + t2.tv_nsec - t1.tv_nsec) / 1000;
  struct timespec diff;
  sub_timespec(t1, t2, &diff);
  if(elapsedTime > 1000) {
    engine->TM-=1;
    // fprintf(stderr, "dec TM %d\n", engine->TM);
    if(engine->TM == 0) {
      fprintf(stderr, "TM == 0!\r\n");
      fprintf(stderr, "%10u : %ld.%.9ld - %ld.%.9ld (%ld.%.9ld)  - \r\n",
              elapsedTime,
              t2.tv_sec, t2.tv_nsec,
              t1.tv_sec, t1.tv_nsec,
              diff.tv_sec, diff.tv_nsec);
      // return EXIT_FAILURE;
    }
  }
  }


  return 0;
}

uint32_t argebee_read_register(argebee_t* engine, uint8_t r1)
{
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
      fprintf(stderr, "unknown register %2X\n", r1);
      exit(EXIT_FAILURE);
    }
  }
}