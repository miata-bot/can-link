#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sqlite3.h"
#include "spect.h"

bool Spect::scripts_load(struct Spect::Connection* conn, struct Spect::Script** scripts_out, int* script_count_out)
{
  assert(conn); assert(conn->db); assert(*scripts_out == NULL);
  struct Spect::Script* scripts = NULL;
  sqlite3_stmt* query = NULL;
  int rc; int i = 0; int script_count = 0;

  rc = sqlite3_prepare_v2(conn->db, "SELECT "\
  "count(id)"\
  "FROM scripts;"
   , -1, &query, NULL);

  if(rc != SQLITE_OK) {
    fprintf(stderr, "[Spect] Failed to prepare query %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(query);
    return false;
  }

  rc = sqlite3_step(query);
  if(rc != SQLITE_ROW) {
    fprintf(stderr, "[Spect] Failed to get script count %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(query);
    return false;
  }
  script_count = sqlite3_column_int(query,  0);
  scripts = (Spect::Script*)malloc(script_count * sizeof(*scripts));
  assert(scripts);
  memset(scripts, 0, script_count * sizeof(*scripts));

  rc = sqlite3_finalize(query);
  query = NULL;

  rc = sqlite3_prepare_v2(conn->db, "SELECT "\
  "id, name, description, content "\
  "FROM scripts;"
  , -1, &query, NULL);

  if(rc != SQLITE_OK) {
    fprintf(stderr, "[Spect] Failed to prepare query %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(query);
    free(scripts);
    return false;
  }

  do {
    const unsigned char* tmp = NULL;
    int length = 0;
    rc = sqlite3_step(query);
    if(rc == SQLITE_ROW) {
      scripts[i].id          = sqlite3_column_int(query, 0);
      scripts[i].name        = (char*)malloc(MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));
      assert(scripts[i].name);
      memset(scripts[i].name, 0, MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));

      scripts[i].description = (char*)malloc(MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));
      assert(scripts[i].description);
      memset(scripts[i].description, 0, MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));

      scripts[i].content     = (char*)malloc(32000 * sizeof(char));
      assert(scripts[i].content);
      memset(scripts[i].content, 0, 32000 * sizeof(char));

      tmp = sqlite3_column_text(query, 1);
      length = sqlite3_column_bytes(query, 1);
      if(length < MAX_EFFECT_SLOT_NAME_STR_LEN && tmp) {
        memcpy(scripts[i].name, tmp, length);
      }

      tmp = sqlite3_column_text(query, 2);
      length = sqlite3_column_bytes(query, 2);
      if(length < MAX_EFFECT_SLOT_NAME_STR_LEN && tmp) {
        memcpy(scripts[i].description, tmp, length);
      }

      tmp = sqlite3_column_text(query, 3);
      length = sqlite3_column_bytes(query, 3);
      if(length < 32000 && tmp) {
        memcpy(scripts[i].content, tmp, length);
      }
    }
    i++;
  } while(rc == SQLITE_ROW);

  if(rc != SQLITE_DONE) {
    fprintf(stderr, "[Spect] Failed to load Scripts %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(query);
    for(int i = 0; i < script_count; i++) {
      if(scripts[i].name) free(scripts[i].name);
      scripts[i].name = NULL;
      if(scripts[i].description) free(scripts[i].description);
      scripts[i].description = NULL;
      if(scripts[i].content) free(scripts[i].content);
      scripts[i].content = NULL;
    }
    free(scripts);
    return false;
  }

  sqlite3_finalize(query);
  *script_count_out = script_count;
  *scripts_out = scripts;
  return true;
}

void Spect::scripts_unload(struct Spect::Script* scripts, int script_count)
{
  for(int i = 0; i < script_count; i++) {
    if(scripts[i].name) free(scripts[i].name);
    scripts[i].name = NULL;
    if(scripts[i].description) free(scripts[i].description);
    scripts[i].description = NULL;
    if(scripts[i].content) free(scripts[i].content);
    scripts[i].content = NULL;
  }
  free(scripts);
}

void Spect::scripts_save(struct Spect::Connection*, struct Spect::Script*)
{

}