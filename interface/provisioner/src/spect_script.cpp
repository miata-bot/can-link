#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sqlite3.h"
#include "spect.h"

bool Spect::script_list_load(struct Spect::Connection* conn, struct Spect::ScriptList** script_list_out)
{
  assert(conn); assert(conn->db); assert(*script_list_out == NULL);
  struct Spect::ScriptList* script_list = NULL;
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
  script_list = (Spect::ScriptList*)malloc(script_count * sizeof(*script_list));
  assert(script_list);
  memset(script_list, 0, script_count * sizeof(*script_list));

  rc = sqlite3_finalize(query);
  query = NULL;

  rc = sqlite3_prepare_v2(conn->db, "SELECT "\
  "id, name, description, content "\
  "FROM scripts;"
  , -1, &query, NULL);

  if(rc != SQLITE_OK) {
    fprintf(stderr, "[Spect] Failed to prepare query %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(query);
    free(script_list);
    return false;
  }

  Spect::ScriptListNode *node = NULL, *dummy_node = NULL;
  dummy_node = (Spect::ScriptListNode*)malloc(sizeof(*dummy_node));
  assert(dummy_node);
  memset(dummy_node, 0, sizeof(*dummy_node));
  node = dummy_node;

  do {
    const unsigned char* tmp = NULL;
    int length = 0;
    rc = sqlite3_step(query);
    if(rc == SQLITE_ROW) {
      node->next = (Spect::ScriptListNode*)malloc(sizeof(struct Spect::ScriptListNode));
      assert(node->next);
      memset(node->next, 0, sizeof(struct Spect::ScriptListNode));
      node = node->next;
      node->next = NULL;

      node->script = (struct Spect::Script*)malloc(sizeof(struct Spect::Script));
      assert(node->script);
      memset(node->script, 0, sizeof(struct Spect::Script));
      
      node->script->id          = sqlite3_column_int(query, 0);
      node->script->name        = (char*)malloc(MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));
      assert(node->script->name);
      memset(node->script->name, 0, MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));

      node->script->description = (char*)malloc(MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));
      assert(node->script->description);
      memset(node->script->description, 0, MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));

      node->script->content     = (char*)malloc(32000 * sizeof(char));
      assert(node->script->content);
      memset(node->script->content, 0, 32000 * sizeof(char));

      tmp = sqlite3_column_text(query, 1);
      length = sqlite3_column_bytes(query, 1);
      if(length < MAX_EFFECT_SLOT_NAME_STR_LEN && tmp) {
        memcpy(node->script->name, tmp, length);
      }

      tmp = sqlite3_column_text(query, 2);
      length = sqlite3_column_bytes(query, 2);
      if(length < MAX_EFFECT_SLOT_NAME_STR_LEN && tmp) {
        memcpy(node->script->description, tmp, length);
      }

      tmp = sqlite3_column_text(query, 3);
      length = sqlite3_column_bytes(query, 3);
      if(length < 32000 && tmp) {
        memcpy(node->script->content, tmp, length);
      }
    }
    i++;
  } while(rc == SQLITE_ROW);

  if(rc != SQLITE_DONE) {
    fprintf(stderr, "[Spect] Failed to load Scripts %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(query);
    script_list_unload(script_list);
    return false;
  }
  script_list->count = script_count;
  script_list->start = dummy_node->next;
  free(dummy_node);

  sqlite3_finalize(query);
  *script_list_out = script_list;
  return true;
}

void Spect::script_list_unload(struct Spect::ScriptList* script_list)
{
  assert(script_list);
  assert(script_list->start);

  Spect::ScriptListNode* node = NULL;
  node = script_list->start;
  int i = 0;
  do {
    Spect::ScriptListNode* this_node = node;
    if(node->script->name) free(node->script->name);
    node->script->name = NULL;

    if(node->script->description) free(node->script->description);
    node->script->description = NULL;

    if(node->script->content) free(node->script->content);
    node->script->content = NULL;

    free(node->script);
    i++;
    node = node->next;
    free(this_node);
  } while(node && i < script_list->count);
  free(script_list);
  script_list = NULL;
  return;
}

const char* script_save_sql[] = {
  "INSERT INTO scripts(name, description, content) VALUES(?, ?, ?);",
  "UPDATE scripts SET name = ?, description = ?, content = ? WHERE id = ?;",
  "DELETE FROM scripts WHERE id = ?;"
};

bool Spect::script_list_save(struct Spect::Connection* conn, Spect::ScriptList* script_list)
{
  assert(conn); assert(conn->db); assert(script_list);
  fprintf(stderr, "[Spect] Saving scripts\n");
  Spect::ScriptListNode* node = NULL;
  sqlite3_stmt* stmt = NULL;
  int rc;

  node = script_list->start;

  do {
    // if no action, skip all this business, go to the next record
    if(node->script->action == Spect::DatabaseAction::SPECT_NONE) goto skip;

    if(node->script->action == Spect::DatabaseAction::SPECT_INSERT) {
      fprintf(stderr, "prepare insert\n");
      // prepare insert
      rc = sqlite3_prepare_v2(conn->db, script_save_sql[0], -1, &stmt, NULL);
      if(rc != SQLITE_OK) goto error;
      fprintf(stderr, "prepared insert\n");

      // Bind
      rc = sqlite3_bind_text(stmt, 1, node->script->name, -1, NULL);
      if(rc != SQLITE_OK) goto error;
      fprintf(stderr, "bind name\n");

      sqlite3_bind_text(stmt, 2, node->script->description, -1, NULL);
      if(rc != SQLITE_OK) goto error;
      fprintf(stderr, "bind descrip\n");

      sqlite3_bind_text(stmt, 3, node->script->content, -1, NULL);
      if(rc != SQLITE_OK) goto error;
      fprintf(stderr, "bind content\n");

    } else if(node->script->action == Spect::DatabaseAction::SPECT_UPDATE) {
      // prepare update
      rc = sqlite3_prepare_v2(conn->db, script_save_sql[1], -1, &stmt, NULL);
      if(rc != SQLITE_OK) goto error;

      // Bind
      rc = sqlite3_bind_text(stmt, 1, node->script->name, -1, NULL);
      if(rc != SQLITE_OK) goto error;

      sqlite3_bind_text(stmt, 2, node->script->description, -1, NULL);
      if(rc != SQLITE_OK) goto error;

      sqlite3_bind_text(stmt, 3, node->script->content, -1, NULL);
      if(rc != SQLITE_OK) goto error;

      rc = sqlite3_bind_int(stmt, 4, node->script->id);
      if(rc != SQLITE_OK) goto error;

    } else if(node->script->action == Spect::DatabaseAction::SPECT_DELETE) {
      // prepare update
      rc = sqlite3_prepare_v2(conn->db, script_save_sql[2], -1, &stmt, NULL);
      if(rc != SQLITE_OK) goto error;
      
      // Bind
      rc = sqlite3_bind_int(stmt, 1, node->script->id);
      if(rc != SQLITE_OK) goto error;
    }
    
    rc = sqlite3_step(stmt);
    fprintf(stderr, "[Spect] script %s\n", sqlite3_errstr(rc));

    sqlite3_finalize(stmt);
    stmt = NULL;
skip:
    node = node->next;
  } while(node);
  return true;

error:
  fprintf(stderr, "[Spect] Failed to save scripts %s\n", sqlite3_errstr(rc));
  if(stmt) sqlite3_finalize(stmt);
  return false;
}
