#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"
#include "spect.h"

bool Spect::config_load(Spect::Connection* conn, struct Spect::Config** config_out)
{
  assert(conn);
  assert(conn->db);
  assert(*config_out == NULL);

  Spect::Config* config = NULL;

  sqlite3_stmt* query = NULL;
  int rc;

  rc = sqlite3_prepare_v2(conn->db, 
    "SELECT "\
    "version, network_id, network_identity_id, network_leader_id "\
    "FROM 'config' LIMIT 1;"
  , -1, &query, NULL);

  if(rc != SQLITE_OK) {
    fprintf(stderr, "[Spect] could not prepare query: %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(query);
    return false;
  }

  config = (Spect::Config*)malloc(sizeof(Spect::Config));
  assert(config);
  memset(config, 0, sizeof(Spect::Config));

  rc = sqlite3_step(query);
  if(rc != SQLITE_ROW) {
    fprintf(stderr, "[Spect] could not run query: %s\n", sqlite3_errstr(rc));
    free(config);
    sqlite3_finalize(query);
    return false;
  }
  config->version = sqlite3_column_int(query, 0);
  config->network_id = sqlite3_column_int(query, 1);
  config->network_identity_id = sqlite3_column_int(query, 2);
  config->network_leader_id = sqlite3_column_int(query, 3);

  // steping again should complete the query
  rc = sqlite3_step(query);
  if(rc != SQLITE_DONE) {
    fprintf(stderr, "[Spect] could not run query: %s\n", sqlite3_errstr(rc));
    free(config);
    sqlite3_finalize(query);
    return false;
  }

  *config_out = config;
  sqlite3_finalize(query);
  return true;
}

void Spect::config_unload(Spect::Config* config) 
{
  assert(config);

  free(config);
  config = NULL;
}

void Spect::config_save(struct Spect::Connection* conn, struct Spect::Config* config)
{
  assert(conn); assert(config);
  sqlite3_stmt* update = NULL;
  int rc;

  rc = sqlite3_prepare_v2(conn->db, 
  "UPDATE config "\
  "SET "\
  "version = ?, "\
  "network_id = ?, "\
  "network_identity_id = ?, "\
  "network_leader_id = ?;"\
   , -1, &update, NULL);
  
  if(rc != SQLITE_OK) {
    fprintf(stderr, "[Spect] Failed to save config %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(update);
    return;
  }

  rc = sqlite3_bind_int(update, 1, config->version);
  assert(rc == SQLITE_OK);

  rc = sqlite3_bind_int(update, 2, config->network_id);
  assert(rc == SQLITE_OK);

  rc = sqlite3_bind_int(update, 3, config->network_identity_id);
  assert(rc == SQLITE_OK);

  rc = sqlite3_bind_int(update, 4, config->network_leader_id);
  assert(rc == SQLITE_OK);

#if 0
  rc = sqlite3_step(update);
  if(rc != SQLITE_ROW) {
    fprintf(stderr, "[Spect] Failed to save config %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(update);
    return;
  }
#endif

  rc = sqlite3_step(update);
  if(rc != SQLITE_DONE) {
    fprintf(stderr, "[Spect] Failed to save config %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(update);
    return;
  }

  sqlite3_finalize(update);
  return;
}
