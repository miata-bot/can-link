#define _GNU_SOURCE // getline
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum token_type {
  TOKEN_UNKNOWN,
  TOKEN_SYMBOL,
  TOKEN_DELIMITER_
} token_type_t;

typedef struct token {
  token_type_t type;
  char src[255];
  size_t length;
} token_t;

typedef struct parser {
  char* line_buffer;
  size_t line_length;
  size_t line_number;
  FILE *file;
  char* filename;
  size_t file_size;
} parser_t;

void process_line(parser_t* parser)
{
  token_t tokens[parser->line_length];
  memset(tokens, parser->line_length, 0);
  size_t token_index = 0;

  // split into spaces, stripping out comments
  for(size_t i = 0; i < parser->line_length; i++) {
    char character = parser->line_buffer[i];
    switch(character) {
      // break out of the loop after a semicolon
      case ';': {
        token_index++;
        i=parser->line_length;
      }

      case ' ': {
        token_index++;
        break;
      }

      case '.': {
        tokens[token_index].type = TOKEN_SYMBOL;
        break;
      }
      case ':': {
        token_index++;
        break;
      }

      default: {
        tokens[token_index].src[tokens[token_index].length++] = character;
        break;
      };
    }
    // maybe not needed?
    token_index++;

    for(size_t i = 0; i < token_index; i++) {
      if(strcmp(tokens[token_index].src, "") == 0) {
      } else if(strcmp(tokens[token_index].src, "") == 0) {

      }
    }

  }
}

int main(void)
{
  parser_t parser;
  parser.line_length = 0;
  parser.file = fopen("/Users/connor/workspace/connorrigby/haltech-link/firmware/example.asm", "r");
  if (parser.file == NULL) {
    fprintf(stderr, "could not open file\n");
    exit(EXIT_FAILURE);
  }

  fseek(parser.file, 0L, SEEK_END);
  parser.file_size = ftell(parser.file);
  fseek(parser.file, 0L, SEEK_SET);
  size_t read;

  fprintf(stderr, "reading file\n");
  parser.line_buffer = NULL;
  // read source file, strip out spaces and remove comments etc. 
  while ((read = getline(&parser.line_buffer, &parser.line_length, parser.file)) != -1) {
    printf("Retrieved line of length %zu:\n", read);
    process_line(&parser);
  }

  fclose(parser.file);
  if (parser.line_length)
      free(parser.line_buffer);
  exit(EXIT_SUCCESS);
}