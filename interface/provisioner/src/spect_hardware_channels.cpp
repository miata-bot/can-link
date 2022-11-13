#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"
#include "spect.h"

bool Spect::hardware_channels_load(struct Spect::Connection* conn, struct Spect::HardwareChannel** hw_channels_out)
{
  assert(conn); assert(conn->db); assert(*hw_channels_out == NULL);
  struct Spect::HardwareChannel* hw_channels = NULL;
  sqlite3_stmt* query = NULL;
  int rc; int i = 0;

  rc = sqlite3_prepare_v2(conn->db, "SELECT "\
  "id, strip_enable, rgb_enable, strip_length, friendly_name "\
  "FROM hardware_channels;"
   , -1, &query, NULL);
  
  if(rc != SQLITE_OK) {
    fprintf(stderr, "[Spect] Failed to prepare statement %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(query);
    return false;
  }
  
  hw_channels = (struct Spect::HardwareChannel*)malloc(MAX_HW_CHANNELS * sizeof(*hw_channels));
  assert(hw_channels);
  memset(hw_channels, 0, MAX_HW_CHANNELS * sizeof(*hw_channels));

  do {
    rc = sqlite3_step(query);
    if(rc == SQLITE_ROW && i < MAX_HW_CHANNELS) {
      hw_channels[i].id           = sqlite3_column_int(query, 0);
      hw_channels[i].strip_enable = sqlite3_column_int(query, 1);
      hw_channels[i].rgb_enable   = sqlite3_column_int(query, 2);
      hw_channels[i].strip_length = sqlite3_column_int(query, 3);
      int length = sqlite3_column_bytes(query, 4);
      const unsigned char* tmp = sqlite3_column_text(query, 4);
      hw_channels[i].friendly_name = (char*)malloc(sizeof(char) * MAX_HW_FRIENDLY_NAME_STR_LEN);
      assert(hw_channels[i].friendly_name);
      memset(hw_channels[i].friendly_name, 0, sizeof(char) * MAX_HW_FRIENDLY_NAME_STR_LEN);
      if(length < MAX_HW_FRIENDLY_NAME_STR_LEN && tmp) {
        memcpy(hw_channels[i].friendly_name, tmp, length);
      }
    }
    i++;
  } while(rc == SQLITE_ROW);

  if(rc != SQLITE_DONE) {
    fprintf(stderr, "[Spect] Failed to load hardware_channels %d %s\n", i, sqlite3_errstr(rc));
    sqlite3_finalize(query);
    hardware_channels_unload(hw_channels);
    return false;
  }

  sqlite3_finalize(query);
  *hw_channels_out = hw_channels;
  return true;
}

void Spect::hardware_channels_unload(struct Spect::HardwareChannel* hw_channels)
{
  assert(hw_channels);

  for(int i = 0; i < MAX_HW_CHANNELS; i++) {
    if(hw_channels[i].friendly_name) free(hw_channels[i].friendly_name);
    hw_channels[i].friendly_name = NULL;
  }
  assert(hw_channels);
  free(hw_channels);
  hw_channels = NULL;
  return;
}

void Spect::hardware_chnanel_save(struct Spect::Connection* conn, struct Spect::HardwareChannel* hw_channel)
{
  assert(false && "TODO: implement save");
}
