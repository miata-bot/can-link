#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "sqlite3.h"

#include "spect.h"

bool spect_config_load(struct Spect_Connection* conn, struct Spect_Config** config_out)
{
  assert(conn); assert(conn->db); assert(*config_out == NULL);
  struct Spect_Config* config = NULL;
  int rc = 0;
  sqlite3_stmt* stmt = NULL;

  sqlite3_stmt* query = NULL;
  rc = sqlite3_prepare_v2(conn->db, "SELECT "\
  "version, network_id, network_identity_id, network_leader_id "\
  "FROM config;"
   , -1, &stmt, NULL);

  if(rc != SQLITE_OK) goto error;

  config = (struct Spect_Config*)malloc(sizeof(*config));
  assert(config);

  do {
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_ROW) break;
    config->version = sqlite3_column_int(stmt, 0);
    config->network_id = sqlite3_column_int(stmt, 1);
    config->network_identity_id = sqlite3_column_int(stmt, 2);
    config->network_leader_id = sqlite3_column_int(stmt, 3);
  } while(rc == SQLITE_ROW);

  *config_out = config;
  sqlite3_finalize(stmt);
  return true;

error:
  fprintf(stderr, "[Spect] Could not load config: %s\n", sqlite3_errstr(rc));
  if(stmt != NULL) sqlite3_finalize(stmt);
  if(config) free(config);
  config = NULL;
  *config_out = NULL;
  return false;
}

void spect_config_unload(struct Spect_Config* config)
{
  assert(config);
  free(config);
}