#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"
#include "spect.h"

bool Spect::network_nodes_load(Spect::Connection* conn, Spect::NetworkNode** nodes_out, int* node_count_out)
{
  assert(conn);
  assert(conn->db);
  assert(*nodes_out == NULL);
  assert(node_count_out);

  sqlite3_stmt* query = NULL;
  int rc;

  Spect::NetworkNode* nodes = NULL;
  int i = 0;

  rc = sqlite3_prepare_v2(conn->db, 
    "SELECT "\
    "node_id, name, rssi, last_seen "\
    "FROM (network_nodes INNER JOIN nodes ON network_nodes.node_id = nodes.id);"
  , -1, &query, NULL);

  if(rc != SQLITE_OK) {
    fprintf(stderr, "[Spect] could not prepare query: %s\n", sqlite3_errstr(rc));
    sqlite3_finalize(query);
    return false;
  }

  nodes = (struct Spect::NetworkNode*)malloc(sizeof(struct Spect::NetworkNode) * MAX_NODES);
  assert(nodes);
  memset(nodes, 0, sizeof(struct Spect::NetworkNode) * MAX_NODES);

  do {
    rc = sqlite3_step(query);
    const unsigned char* tmp = NULL;
    int length = sqlite3_column_bytes(query, 1);
    if(length) {
      nodes[i].name = (char*)malloc((sizeof(char) * length) + 1);
      memset(nodes[i].name, 0, (sizeof(char) * length) + 1);
      tmp = sqlite3_column_text(query, 1);
      if(tmp) {
        memcpy(nodes[i].name, tmp, sizeof(char) * length);
      }
    }
    nodes[i].node_id = sqlite3_column_int(query, 0);
    nodes[i].rssi = sqlite3_column_int(query, 2);
    nodes[i].last_seen = sqlite3_column_int(query, 3);

    i++;
  } while(rc == SQLITE_ROW);
  assert(i > 1);

  if(rc != SQLITE_DONE) {
    fprintf(stderr, "[Spect] could not run query: %s\n", sqlite3_errstr(rc));
    free(nodes);
    sqlite3_finalize(query);
    return false;
  }

  fprintf(stderr, "[Spect] Loaded %d network nodes\n", i-1);
  *nodes_out = nodes;
  *node_count_out = i-1;
  sqlite3_finalize(query);
  return true;
}

void Spect::network_nodes_unload(struct Spect::NetworkNode* nodes)
{
  for(int i = 0; i < MAX_NODES; i++) {
    struct Spect::NetworkNode* node = &nodes[i];
    if(node->name) free(node->name);
    node->name = NULL;
  }
  free(nodes);
  nodes = NULL;
}
