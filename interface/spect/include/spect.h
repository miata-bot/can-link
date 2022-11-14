#ifndef _SPECT_H
#define _SPECT_H

#include <stdint.h>
#include <stdbool.h>

#include "sqlite3.h"

struct Spect_Connection {
  sqlite3* db;
};

struct Spect_Config {
  int version;
  int network_id;
  int network_identity_id;
  int network_leader_id;
};

struct Spect_Network_Node {
  int id;
  int rssi;
  int last_seen;
  const char* name;
};

struct Spect_Network_Node_List_Node {
  struct Spect_Network_Node_List_Node* next;
  struct Spect_Network_Node* node;
};

struct Spect_Network_Node_List {
  struct Spect_Network_Node_List_Node* first;
  int count;
};

bool spect_connection_open(struct Spect_Connection**, const char*);
void spect_connection_close(struct Spect_Connection*);

bool spect_config_load(struct Spect_Connection*, struct Spect_Config**);
void spect_config_unload(struct Spect_Config*);

bool spect_network_list_load(struct Spect_Connection*, struct Spect_Network_Node_List**);
void spect_network_list_unload(struct Spect_Network_Node_List*);

#endif
