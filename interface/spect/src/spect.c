#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "sqlite3.h"

#include "spect.h"

bool spect_connection_open(struct Spect_Connection** conn_out, const char* path)
{
  assert(*conn_out == NULL);
  struct Spect_Connection* conn = NULL;
  int rc;
  conn = (struct Spect_Connection*)malloc(sizeof(*conn));
  assert(conn);

  rc = sqlite3_open(path, &conn->db);
  if(rc != SQLITE_OK) {
    fprintf(stderr, "[Spect] failed to open database %s\n", sqlite3_errstr(rc));
    return false;
  }
  *conn_out = conn;
  return true;
}

void spect_connection_close(struct Spect_Connection* conn)
{
  assert(conn); assert(conn->db);
  sqlite3_close(conn->db);
  conn->db = NULL;
  free(conn);
}
