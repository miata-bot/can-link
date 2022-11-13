#pragma once

#include <stdlib.h>
#include <stdbool.h>

#include "sqlite3.h"

const int MAX_NODES                    = 10;
const int MAX_HW_CHANNELS              = 2;
const int MAX_HW_TRIGGERS              = 4;
const int MAX_HW_FRIENDLY_NAME_STR_LEN = 120;

const int MAX_SECTIONS                 = 8;
const int MAX_EFFECT_SLOTS             = 8;
const int MAX_EFFECT_SLOT_NAME_STR_LEN = 120;

const int SPECT_DATABSE_ACTION_INSERT = 0b00000001;
const int SPECT_DATABSE_ACTION_UPDATE = 0b00000010;
const int SPECT_DATABSE_ACTION_DELETE = 0b00000100;

namespace Spect 
{
  struct Connection {
    sqlite3* db;
  };

  enum DatabaseAction {
    SPECT_INSERT = SPECT_DATABSE_ACTION_INSERT,
    SPECT_UPDATE = SPECT_DATABSE_ACTION_UPDATE,
    SPECT_DELETE = SPECT_DATABSE_ACTION_DELETE,
    SPECT_NONE   = 0
  };

  struct Config {
    enum DatabaseAction action;
    int version;
    int network_id;
    int network_identity_id;
    int network_leader_id;
  };

  struct NetworkNode {
    enum DatabaseAction action;
    int   node_id;
    char* name;
    int   rssi;
    int   last_seen;
  };

  struct HardwareChannel {
    enum DatabaseAction action;
    int   id;
    bool  strip_enable;
    bool  rgb_enable;
    int   strip_length;
    char* friendly_name;
  };

  struct HardwareTrigger {
    enum DatabaseAction action;
    int   id;
    bool  enable;
    char* friendly_name;
  };

  struct EffectSlot {
    enum DatabaseAction action;
    int id;
    struct EffectMode {
      bool enable;
      bool radio_sync;
      bool can_sync;
    } mode;
  };

  struct Script {
    enum DatabaseAction action;
    int   id;
    char* name;
    char* description;
    char* content;
  };

  struct ScriptListNode {
    struct Script* script;
    struct ScriptListNode* next;
  };

  struct ScriptList {
    struct ScriptListNode* start;
    int count;
  };

  struct Section {
    enum DatabaseAction action;
    int id;
    int start;
    int end;
  };

  struct SlotSection {
    enum DatabaseAction action;
    int section_id;
    int slot_id;
    struct Section* section;
    struct Slot* slot;
  };
  
  void Open(struct Connection**, char*);
  void Close(struct Connection*);

  bool config_load(struct Connection*, struct Config**);
  void config_save(struct Connection*, struct Config*);
  void config_unload(struct Config*);

  bool network_nodes_load(struct Connection*, struct NetworkNode**, int*);
  void network_nodes_unload(struct NetworkNode*);

  bool hardware_channels_load(struct Connection*, struct HardwareChannel**);
  void hardware_channels_unload(struct HardwareChannel*);
  void hardware_chnanel_save(struct Connection*, struct HardwareChannel*);

  bool hardware_triggers_load(struct Connection*, struct HardwareTrigger**);
  void hardware_triggers_unload(struct HardwareTrigger*);
  void hardware_trigger_save(struct Connection*, struct HardwareTrigger*);

  bool script_list_load(struct Connection*, struct ScriptList**);
  void script_list_unload(struct ScriptList*);
  bool script_list_save(struct Connection*, struct ScriptList*);

  bool effect_slot_load(struct Connection*, struct EffectSlot**);
  void effect_slot_unload(struct EffectSlot*);
  void effect_slot_save(struct Connection*, struct EffectSlot*);

  bool section_load(struct Connection*, struct Section**);
  void section_unload(struct Section*);
  void section_save(struct Connection*, struct Section*);

  bool slot_section_load(struct Connection*, struct SlotSection**);
  void slot_section_unload(struct SlotSection*);
  void slot_section_save(struct Connection*, struct SlotSection*);
};