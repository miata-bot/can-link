#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nfd.h"
#include "file_dialog.h"

#ifndef __EMSCRIPTEN__
// PATH_MAX
const size_t max_path_length = 4096;
#endif

char* path_open_for_write()
{
#ifdef __EMSCRIPTEN__
  return "/config.db";
#else
  nfdchar_t *outPath;
  nfdfilteritem_t filterItem[] = { { "ConeRGB Config File", "db" } };
  nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL /* "../spect" */);
  size_t length = max_path_length;
  char* path = NULL;
  if(result == NFD_OKAY) {
    length = strnlen(outPath, max_path_length);
    if(length && length < max_path_length) {
      path = (char*)malloc(sizeof(char) * length + 1);
      assert(path);
      memset(path, 0, sizeof(char) * length + 1);
      memcpy(path, outPath, sizeof(char) * length);
    }
    NFD_FreePath(outPath);
    fprintf(stderr, "opened %s\n", path);
    return path;
  } else if(result == NFD_CANCEL) {
    fprintf(stderr, "Error opening file: user canceled\n");
    return NULL;
  } else {
    fprintf(stderr, "Error opening file: %s\n", NFD_GetError());
    return NULL;
  }
#endif
}

void path_free(char* path)
{
#ifdef __EMSCRIPTEN__
  // no need to free the path since it's const here
  return;
#endif
  if(path) free(path);
}