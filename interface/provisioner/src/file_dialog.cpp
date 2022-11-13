#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nfd.h"
#include "file_dialog.h"

// PATH_MAX
const size_t max_path_length = 4096;

char* path_open_for_write()
{
  nfdchar_t *outPath;
  nfdfilteritem_t filterItem[] = { { "ConeRGB Config File", "db" } };
  nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, "../spect");
  size_t length = max_path_length;
  char* path = NULL;
  if(result == NFD_OKAY) {
    length = strnlen(outPath, max_path_length);
    if(length && length < max_path_length) {
      path = (char*)malloc(sizeof(char) * length + 1);
      assert(path);
      memset(path, 0, sizeof(char) * length + 1);
      memcpy(path, outPath, sizeof(char) * length);
      // fprintf(stderr, "path=%s\n", path);
    }
    NFD_FreePath(outPath);
    fprintf(stderr, "opened %s[%ld]\n", path, length);
    return path;
  } else if(result == NFD_CANCEL) {
    fprintf(stderr, "Error opening file: user canceled\n");
    return NULL;
  } else {
    fprintf(stderr, "Error opening file: %s\n", NFD_GetError());
    return NULL;
  }
}

void path_free(char* path)
{
  if(path) free(path);
}