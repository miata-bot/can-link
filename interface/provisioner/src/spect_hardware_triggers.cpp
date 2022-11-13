#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"
#include "spect.h"

bool Spect::hardware_triggers_load(struct Spect::Connection* conn, struct Spect::HardwareTrigger** hw_triggers_out)
{
  assert(conn); assert(conn->db); assert(*hw_triggers_out == NULL);
  struct Spect::HardwareTrigger* hw_triggers = NULL;
  sqlite3_stmt* query = NULL;
  int rc; int i = 0;

  rc = sqlite3_prepare_v2(conn->db, "SELECT "\
  "id, enable, friendly_name "\
  "FROM hardware_triggers;"
   , -1, &query, NULL);
  
  if(rc != SQLITE_OK) {
    fprintf(stderr, "[Spect] Failed to prepare statement %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(query);
    return false;
  }

  hw_triggers = (struct Spect::HardwareTrigger*)malloc(MAX_HW_TRIGGERS * sizeof(*hw_triggers));
  assert(hw_triggers);
  memset(hw_triggers, 0, MAX_HW_TRIGGERS * sizeof(*hw_triggers));

  do {
    rc = sqlite3_step(query);
    if(rc == SQLITE_ROW && i < MAX_HW_TRIGGERS) {
      hw_triggers[i].id        = sqlite3_column_int(query, 0);
      hw_triggers[i].enable    = sqlite3_column_int(query, 1);
      int length               = sqlite3_column_bytes(query, 2);
      const unsigned char* tmp = sqlite3_column_text(query, 2);
      if((length < MAX_HW_FRIENDLY_NAME_STR_LEN) && tmp) {
        hw_triggers[i].friendly_name = (char*)malloc(sizeof(char) * MAX_HW_FRIENDLY_NAME_STR_LEN);
        assert(hw_triggers[i].friendly_name);
        memset(hw_triggers[i].friendly_name, 0, sizeof(char) * MAX_HW_FRIENDLY_NAME_STR_LEN);
        memcpy(hw_triggers[i].friendly_name, tmp, length);
      }
    }
    i++;
  } while(rc == SQLITE_ROW);

  if(rc != SQLITE_DONE) {
    fprintf(stderr, "[Spect] Failed to load hardware_triggers %d %s\n", i, sqlite3_errstr(rc));
    sqlite3_finalize(query);
    hardware_triggers_unload(hw_triggers);
    return false;
  }

  sqlite3_finalize(query);
  *hw_triggers_out = hw_triggers;
  return true;
}

void Spect::hardware_triggers_unload(struct Spect::HardwareTrigger* hw_triggers)
{
  assert(hw_triggers);

  for(int i = 0; i < MAX_HW_TRIGGERS; i++) {
    if(hw_triggers[i].friendly_name) free(hw_triggers[i].friendly_name);
    hw_triggers[i].friendly_name = NULL;
  }
  free(hw_triggers);
  hw_triggers = NULL;
  return;
}

void Spect::hardware_trigger_save(struct Spect::Connection* conn, struct Spect::HardwareTrigger* hw_channel)
{
  assert(false && "TODO: implement save");
}
