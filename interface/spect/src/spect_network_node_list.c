#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sqlite3.h"

#include "spect.h"

bool spect_network_list_load(
  struct Spect_Connection* conn, 
  struct Spect_Network_Node_List** network_node_list_out
)
{
  assert(conn); assert(conn->db); assert(*network_node_list_out == NULL);
  struct Spect_Network_Node_List* network_node_list = NULL;
  struct Spect_Network_Node_List_Node *node = NULL, *dummy_node = NULL;
  const char* tmp;
  int name_length = 0;

  sqlite3_stmt* stmt = NULL;
  int rc; int count = 0;

  rc = sqlite3_prepare_v2(conn->db, "SELECT "\
  "nodes.id, nodes.rssi, nodes.last_seen, network_nodes.name "\
  "FROM nodes "\
  "INNER JOIN network_nodes ON network_nodes.node_id = nodes.id "\
  "INNER JOIN config ON config.network_id = network_nodes.network_id;"
    , -1, &stmt, NULL);
  if(rc != SQLITE_OK) goto error;

  network_node_list = malloc(sizeof(*network_node_list));
  assert(network_node_list);
  memset(network_node_list, 0, sizeof(*network_node_list));

  // create a dummy node that is a pointer to the first node
  // this will get freed later.
  dummy_node = malloc(sizeof(*dummy_node));
  assert(dummy_node);
  node = dummy_node;
  do {
    assert(node);
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_ROW) break;
    node->next = malloc(sizeof(*dummy_node));
    assert(node->next);
    memset(node->next, 0, sizeof(*dummy_node));
    node = node->next;
    node->node = malloc(sizeof(struct Spect_Network_Node));
    assert(node->node);
    memset(node->node, 0, sizeof(struct Spect_Network_Node));

    node->node->id = sqlite3_column_int(stmt, 0);
    node->node->rssi = sqlite3_column_int(stmt, 1);
    node->node->last_seen = sqlite3_column_int(stmt, 2);
    tmp = sqlite3_column_text(stmt, 3);
    name_length = sqlite3_column_bytes(stmt, 3);
    node->node->name = malloc((name_length + 1) * sizeof(const char)); 
    assert(node->node->name);
    memset((void*)node->node->name, 0, (name_length + 1) * sizeof(const char));
    memcpy((void*)node->node->name, tmp, name_length);
  } while(rc == SQLITE_ROW);
  network_node_list->first = dummy_node->next;
  free(dummy_node);

  if(rc != SQLITE_DONE) goto error;
  *network_node_list_out = network_node_list;
  sqlite3_finalize(stmt);
  return true;

error:
  fprintf(stderr, "[Spect] Could not load network node list: %s\n", sqlite3_errstr(rc));
  if(stmt != NULL) sqlite3_finalize(stmt);
  if(network_node_list) spect_network_list_unload(network_node_list);
  network_node_list = NULL;
  *network_node_list_out = NULL;
  return false;
}

void spect_network_list_unload(struct Spect_Network_Node_List* network_node_list)
{
  assert(network_node_list);
  struct Spect_Network_Node_List_Node *node = NULL, *tmp = NULL;
  node = network_node_list->first;
  do {
    if(node->node->name) free((void*)node->node->name);
    node->node->name = NULL;
    free(node->node);
    node->node = NULL;
    tmp = node->next;
    free(node);
    node = tmp;
  } while(node);
  free(network_node_list);
  network_node_list = NULL;
}