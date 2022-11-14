#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sqlite3.h"

#include "spect.h"

// select effect_slots.id, sections.id, slot_sections.slot_section_id from effect_slots inner join slot_sections on effect_slots.id = slot_sections.slot_id inner join sections on sections.id = slot_sections.section_id;

struct Spect_Slot_Section {
  int id;
  int start;
  int end;
};

struct Spect_Slot_Section_List_Node {
  struct Spect_Slot_Section_List_Node* next;
  struct Spect_Slot_Section* slot_section;
};

struct Spect_Slot_Section_List {
  struct Spect_Slot_Section_List_Node* first;
  int count;
};

struct Spect_Effect_Slot {
  int id;
  struct Spect_Slot_Section_List* slot_section_list;
};

struct Spect_Effect_Slot_Node {
  struct Spect_Effect_Slot_Node* next;
  struct Spect_Effect_Slot* effect_slot;
};

struct Spect_Effect_Slot_List {
  struct Spect_Effect_Slot_Node* first;
  int count;
};

void spect_effect_slot_list_load(struct Spect_Connection* conn, struct Spect_Effect_Slot_List** effect_slot_list_out)
{
  assert(conn); assert(conn->db); assert(*effect_slot_list_out = NULL);
  int rc;
  struct Spect_Effect_Slot_List* effect_slot_list = NULL;
  struct Spect_Effect_Slot_Node *node = NULL, *first_node = NULL;
  sqlite3_stmt* stmt = NULL;
  int count = 0;

  rc = sqlite3_prepare_v2(conn->db, "SELECT "\
  "effect_slots.id "\
  "FROM effect_slots;"
    , -1, &stmt, NULL);
  
  assert(rc == SQLITE_OK);

  effect_slot_list = malloc(sizeof(*effect_slot_list));
  assert(effect_slot_list);
  memset(effect_slot_list, 0, sizeof(*effect_slot_list));

  do {

  } while();
}
