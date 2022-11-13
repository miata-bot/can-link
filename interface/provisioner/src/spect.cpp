#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sqlite3.h"

#include "spect.h"

void Spect::Open(struct Spect::Connection** conn_out, char* path)
{
  assert(path);
  assert(*conn_out == NULL);

  int rc;
  struct Spect::Connection* conn;
  conn = (struct Spect::Connection*)malloc(sizeof(struct Spect::Connection));
  assert(conn);

  rc = sqlite3_open(path, &conn->db);
  if(rc != SQLITE_OK) {
    free(conn);
    return;
  }

  *conn_out = conn;
}

void Spect::Close(struct Spect::Connection* conn)
{
  assert(conn);
  int rc = sqlite3_close(conn->db);
  if(rc != SQLITE_OK) {
    fprintf(stderr, "[Spect] Failed to close DB");
    return;
  }
  conn->db = NULL;
  free(conn);
}
