#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <signal.h>

#include "spect.h"

struct Spect_Connection* conn = NULL;
struct Spect_Config* config = NULL;
struct Spect_Network_Node_List* network_node_list = NULL;

static bool done = false;

void handle_sigint(int arg) {
  (void)arg;
  done = true;
}

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  struct Spect_Network_Node_List_Node* network_node_list_node = NULL;
  signal(SIGINT, handle_sigint);

  if(!spect_connection_open(&conn, "database.db")) {
    fprintf(stderr, "[shell] Failed to open spect database\n");
    return 1;
  }
  assert(conn);
  fprintf(stderr, "[shell] opened database\n");

  if(!spect_config_load(conn, &config)) {
    fprintf(stderr, "[shell] Failed to open config\n");
    goto error;
  }
  assert(config);
  fprintf(stderr, "[shell] opened config version=%d network=%d network_identity=%d network_leader=%d\n", 
    config->version, 
    config->network_id, 
    config->network_identity_id, 
    config->network_leader_id
  );

  if(!spect_network_list_load(conn, &network_node_list)) {
    fprintf(stderr, "[shell] Failed to get network nodes\n");
    goto error;
  }
  fprintf(stderr, "[shell] Loaded network nodes\n");
  network_node_list_node = network_node_list->first;
  do {
    fprintf(stderr, "[shell] Node id=%d [name=%s, rssi=%d, last_seen=%d]\n", 
      network_node_list_node->node->id, 
      network_node_list_node->node->name, 
      network_node_list_node->node->rssi,
      network_node_list_node->node->last_seen
    );
    network_node_list_node = network_node_list_node->next;
  } while(network_node_list_node);
#if 0
  while(done == false) {

  }
#endif
error:
  fprintf(stderr, "[shell] unloading network nodes\n");
  if(network_node_list) spect_network_list_unload(network_node_list);

  fprintf(stderr, "[shell] unloading config\n");
  if(config) spect_config_unload(config);

  fprintf(stderr, "[shell] closing database\n");
  if(conn) spect_connection_close(conn);

  fprintf(stderr, "[shell] shutting down\n");
  return EXIT_SUCCESS;

}