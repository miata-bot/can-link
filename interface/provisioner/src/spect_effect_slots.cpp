#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sqlite3.h"
#include "spect.h"

bool Spect::effect_slot_load(struct Spect::Connection* conn, struct Spect::EffectSlot** effect_slots_out)
{
  assert(conn); assert(conn->db); assert(*effect_slots_out == NULL);
  sqlite3_stmt* query = NULL;
  struct Spect::EffectSlot* effect_slots = NULL;
  int rc; int i = 0;

  rc = sqlite3_prepare_v2(conn->db, "SELECT "\
  "id, mode "\
  "FROM effect_slots;"
  , -1, &query, NULL);

  if(rc != SQLITE_OK) {
    fprintf(stderr, "[Spect] Failed to prepare query %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(query);
    return false;
  }

  effect_slots = (struct Spect::EffectSlot*)malloc(MAX_EFFECT_SLOTS * sizeof(*effect_slots));
  assert(effect_slots);
  memset(effect_slots, 0, MAX_EFFECT_SLOTS * sizeof(*effect_slots));

  do {
    rc = sqlite3_step(query);
    if(rc == SQLITE_ROW && i < MAX_EFFECT_SLOTS) {
      effect_slots[i].id = sqlite3_column_int(query, 0);
      int mode = sqlite3_column_int(query, 1);
      effect_slots[i].mode.enable     = (mode >> 0) & 1;
      effect_slots[i].mode.radio_sync = (mode >> 1) & 1;
      effect_slots[i].mode.can_sync   = (mode >> 2) & 1;
    }
    i++;
  } while(rc == SQLITE_ROW);

  if(rc != SQLITE_DONE) {
    fprintf(stderr, "[Spect] Failed to load slots %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(query);
    return false;
  }

  sqlite3_finalize(query);
  *effect_slots_out = effect_slots;
  return true;
}

void Spect::effect_slot_unload(struct Spect::EffectSlot* slots)
{
  assert(slots);
  for(int i = 0; i < MAX_EFFECT_SLOTS; i++) {
  }
  free(slots);
  return;
}

void Spect::effect_slot_save(struct Spect::Connection* conn, struct Spect::EffectSlot* slot)
{
  (void)conn;
  (void)slot;
  assert(false && "FIXME: implement slot saving");
}

