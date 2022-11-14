#include <assert.h>
#include <stdio.h>

#include "imgui.h"
#include "imgui_utils.h"
#include "TextEditor.h"
#include "nfd.h"

#include "gitparams.h"
#include "provision.h"
#include "file_dialog.h"
#include "spect.h"

// modal states
bool show_save_modal = false;
bool show_about_modal = false;
bool show_license_modal = false;

// window states
bool show_demo_window = false;

// global state
bool edits_pending = false;

char* config_file_path = NULL;
struct Spect::Connection* conn = NULL;
struct Spect::Config* config = NULL;

struct Spect::NetworkNode* network_nodes = NULL;
int network_node_count = 0;

struct Spect::HardwareChannel* hardware_channels = NULL;

struct Spect::HardwareTrigger* hardware_triggers = NULL;

struct Spect::EffectSlot* effect_slots = NULL;
struct Spect::Section* sections = NULL;

struct Spect::ScriptList* script_list = NULL;

struct NodeWindowState {
  bool selected;
};

struct NodeWindowState* node_windows = NULL;

struct HardwareWindowState {
  bool selected;
} hardware_window_state;

struct EffectSlotWindowState {
  bool selected;
} effect_slot_window_state;

struct ScriptEditorWindowState {
  bool                          selected;
  int                           selected_id;
  TextEditor*                   editor;
  std::string*                  editor_buffer;
  struct Spect::ScriptListNode* script_node;
} script_editor_window_state;

static int friendly_name_input_callback(ImGuiInputTextCallbackData* data)
{
  (void)data;
  edits_pending = true;
  return 0;
}

void node_window_state_init(void)
{
  node_windows = (struct NodeWindowState*)malloc(sizeof(struct NodeWindowState) * network_node_count);
  assert(node_windows);
  memset(node_windows, 0, sizeof(struct NodeWindowState) * network_node_count);
}

void node_window_state_deinit(void)
{
  if(node_windows) free(node_windows);
  node_windows = NULL;
}

void hardware_window_state_init(void)
{
  memset(&hardware_window_state, 0, sizeof(struct HardwareWindowState));
  hardware_window_state.selected = true;
}

void hardware_window_state_deinit(void)
{
  memset(&hardware_window_state, 0, sizeof(struct HardwareWindowState));
}

void effect_slot_window_state_init(void)
{
  memset(&effect_slot_window_state, 0, sizeof(struct EffectSlotWindowState));
  effect_slot_window_state.selected = true;
}

void effect_slot_window_state_deinit(void)
{
  memset(&effect_slot_window_state, 0, sizeof(struct EffectSlotWindowState)); 
}

void script_editor_window_state_init(void)
{
  memset(&script_editor_window_state, 0, sizeof(struct ScriptEditorWindowState)); 
  script_editor_window_state.selected = true;
  script_editor_window_state.editor = new TextEditor();
  script_editor_window_state.script_node = NULL;
	auto lang = TextEditor::LanguageDefinition::Lua();
	script_editor_window_state.editor->SetLanguageDefinition(lang);
  script_editor_window_state.editor_buffer = new std::string();
  script_editor_window_state.selected_id = -1;
}

void script_editor_window_state_deinit(void)
{
  if(script_editor_window_state.editor) delete script_editor_window_state.editor;
  script_editor_window_state.editor = NULL;
  if(script_editor_window_state.editor_buffer) delete script_editor_window_state.editor_buffer;
  script_editor_window_state.editor_buffer = NULL;
  script_editor_window_state.script_node = NULL; // dont free this, it's a ref
  script_editor_window_state.selected = false;

  if(script_list == NULL) return; // probably not correct...
  assert(script_list && "Script List must be valid to deinit script_editor");
}

// Save the current buffer into the current node
void script_editor_write_buffer(
  struct ScriptEditorWindowState* script_editor_window_state
);

// Saves a new script into the script linked list
void script_editor_save_script(
	struct Spect::ScriptList* script_list,
	struct ScriptEditorWindowState* script_editor_window_state,
	char*  script_name,
	char*  script_description
);

// handles populating the buffer of the script editor
void script_editor_handle_tab(
	struct Spect::ScriptListNode* node,
	struct ScriptEditorWindowState* script_editor_window_state,
	int old_id, int id
);

void db_open(void)
{
  Spect::Open(&conn, config_file_path);
  if(conn)
  {
    if(!Spect::config_load(conn, &config)) {
      fprintf(stderr, "Failed to load config\n");
    }

    if(!Spect::network_nodes_load(conn, &network_nodes, &network_node_count)) {
      fprintf(stderr, "Failed to load nodes\n");
    }

    if(!Spect::hardware_channels_load(conn, &hardware_channels)) {
      fprintf(stderr, "Failed to load hardware channels\n");
    }

    if(!Spect::hardware_triggers_load(conn, &hardware_triggers)) {
      fprintf(stderr, "Failed to load hardware triggers\n");
    }

    if(!Spect::script_list_load(conn, &script_list)) {
      fprintf(stderr, "Failed to load scripts\n");
    }

    if(!Spect::effect_slot_load(conn, &effect_slots)) {
      fprintf(stderr, "Failed to load effect slots\n");
    }

#if 0
    if(!Spect::section_load(conn, &sections)) {
      fprintf(stderr, "Failed to load sections\n");
    }
#endif

    fprintf(stderr, "Loaded DB\n");

    node_window_state_init();
    hardware_window_state_init();
    effect_slot_window_state_init();
    script_editor_window_state_init();
  }
}

// unload in the opposite order of load
void db_close(void)
{
  script_editor_window_state_deinit();
  effect_slot_window_state_deinit();
  hardware_window_state_deinit();
  node_window_state_deinit();

#if 0
  if(sections) {
    Spect::section_unload(sections);
    sections = NULL;
  }
#endif

  if(effect_slots) {
    Spect::effect_slot_unload(effect_slots);
    effect_slots = NULL;
  }

  if(script_list) {
    Spect::script_list_unload(script_list);
    script_list = NULL;
  }

  if(hardware_triggers) {
    Spect::hardware_triggers_unload(hardware_triggers);
    hardware_triggers = NULL;
  }

  if(hardware_channels) {
    Spect::hardware_channels_unload(hardware_channels);
    hardware_channels = NULL;
  }

  if(network_nodes) {
    Spect::network_nodes_unload(network_nodes);
    network_nodes = NULL;
    network_node_count = 0;
  }

  if(config) {
    Spect::config_unload(config);
    config = NULL;
  }

  if(conn) {
    Spect::Close(conn);
    conn = NULL;
  }

  if(config_file_path) {
    path_free(config_file_path);
    config_file_path = NULL;
  }
}

void db_save()
{
  Spect::config_save(conn, config);
  if(script_editor_window_state.script_node) script_editor_write_buffer(&script_editor_window_state);
  Spect::script_list_save(conn, script_list);
  edits_pending = false;
  show_save_modal = false;
  fprintf(stderr, "Saved DB\n");
}

void Provision::Init(int argc, char** argv)
{
  // TODO: allow passing db file on the cmdline
  (void)argc;
  (void)argv;
#ifndef __EMSCRIPTEN__
  // TODO: check result
  NFD_Init();
#endif
}

void RenderEditModal() {
  // Always center this window when appearing
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("Edits Pending", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    ImGui::Text("Edits still pending");
    ImGui::Separator();

    if (ImGui::Button("Save", ImVec2(120, 0))) { 
      db_save();
      db_close();
      show_save_modal = false;
      ImGui::CloseCurrentPopup(); 
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("Discard Edits", ImVec2(120, 0))) {
      db_close();
      show_save_modal = false;
      edits_pending = false;
      ImGui::CloseCurrentPopup(); 
    }
    ImGui::EndPopup();
  }
}

void RenderAboutModal(struct Spect::Config*, bool*);
void RenderLicenseModal(bool*);

void RenderMainMenuBar(bool* requestDone)
{
  if(ImGui::BeginMainMenuBar()) 
  {
    if(ImGui::BeginMenu("File")) 
    {
      // Show the `Open` dialog if there is no config file opened alread
      if(config_file_path == NULL) {
        if(ImGui::MenuItem("Open Config File")) {
          config_file_path = path_open_for_write();
          if(config_file_path) db_open();
        }
      // otherwise show the `Close` dialog
      } else {
        if(ImGui::MenuItem("Save Config File")) db_save();
        ImGui::Separator();
        if(ImGui::MenuItem("Close Config File")) {
          if(edits_pending) {
            show_save_modal = true;
            ImGui::EndMenu();
            ImGui::EndMainMenuBar();
            return;
          }
          db_close();
        }
      }

      ImGui::Separator();
#ifdef DEBUG
      ImGui::MenuItem("Debug Options", NULL, false, false);
      ImGui::MenuItem("Show ImGui Demo", NULL, &show_demo_window);
      if(ImGui::MenuItem("Show Save Modal"))  show_save_modal = true;
      ImGui::Separator();
#endif
      if(ImGui::MenuItem("Quit", NULL)) {
        if(edits_pending) {
          show_save_modal = true;
          ImGui::EndMenu();
          ImGui::EndMainMenuBar();
          return;
        }
        *requestDone = true;
      }

      ImGui::EndMenu();
    }

    if(ImGui::BeginMenu("Window", conn != NULL))
    {
      ImGui::MenuItem("Hardware Config", NULL, &hardware_window_state.selected, (
        (hardware_channels != NULL) &&
        (hardware_triggers != NULL)
      ));
      ImGui::EndMenu();
    }

    if(ImGui::BeginMenu("Help"))
    {
      if(ImGui::MenuItem("3rd Party Licenses")) show_license_modal = true;
      if(ImGui::MenuItem("About")) show_about_modal = true;
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}

void RenderNavigationWindow()
{
  int flags = ImGuiWindowFlags_None;
  if(edits_pending) flags |= ImGuiWindowFlags_UnsavedDocument;

  if(!ImGui::Begin("Editor", NULL, flags)) {
    ImGui::End();
    return;
  }

  if(
    (config_file_path  == NULL) || 
    (conn              == NULL) || 
    (config            == NULL) ||
    (network_nodes     == NULL) ||
    (hardware_channels == NULL) ||
    (effect_slots      == NULL)
  ) {
    ImGui::Text("Open a config file first");
    ImGui::End();
    return;
  }

  if(ImGui::CollapsingHeader("Network", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Version");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), "%d", config->version);

    ImGui::Text("Network");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), "%d", config->network_id);

    ImGui::Text("Identity");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), "%d", config->network_identity_id);

    ImGui::Text("Leader");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), "%d", config->network_leader_id);
  }

  if(ImGui::CollapsingHeader("Network Nodes", ImGuiTreeNodeFlags_DefaultOpen)) {
    for(int i = 0; i < network_node_count; i++) {
      static char buffer[120];
      bool selected = node_windows[i].selected;
      snprintf(buffer, 120, "##%d", i);
      if(ImGui::Selectable(buffer, selected))
        node_windows[i].selected = true;

      ImGui::SameLine();

      snprintf(buffer, 120, "Node %d", network_nodes[i].node_id);

      ImGui::Text(buffer);
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), "%s", network_nodes[i].name);
      if(network_nodes[i].node_id == config->network_leader_id) {
        ImGui::SameLine();
        ImGui::Text("(leader)");
      }
    }
    ImGui::Separator();
  }

  if(ImGui::CollapsingHeader("Lighting Channels", ImGuiTreeNodeFlags_DefaultOpen)) {
    for(int i = 0; i < MAX_HW_CHANNELS; i++) {
      ImGui::Text("%s %d", hardware_channels[i].friendly_name, hardware_channels[i].id);
    }
  }

  if(ImGui::CollapsingHeader("Input Channels", ImGuiTreeNodeFlags_DefaultOpen)) {
    for(int i = 0; i < MAX_HW_TRIGGERS; i++) {
      ImGui::Text("%s %d", hardware_triggers[i].friendly_name, hardware_triggers[i].id);
    }
  }

  if(ImGui::CollapsingHeader("Effect Slots", ImGuiTreeNodeFlags_DefaultOpen)) {
    for(int i = 0; i < MAX_EFFECT_SLOTS; i++) {
      ImGui::Text("Slot %d", effect_slots[i].id);
    }
  }

  ImGui::End();
}

void RenderHardwareChannelsWindow()
{
  static char buffer[120];
  if(hardware_window_state.selected) {
    ImGui::SetNextWindowSize(ImVec2(250, 90), ImGuiCond_FirstUseEver);
    if(!ImGui::Begin("Lighting Channel Config", &hardware_window_state.selected)) {
      ImGui::End();
      return;
    }

    ImGui::Text("Lighting Channels");
    for(int i = 0; i < MAX_HW_CHANNELS; i++) {
      snprintf(buffer, 120, "Lighting Channel %d##%d", hardware_channels[i].id,hardware_channels[i].id);
      if(ImGui::CollapsingHeader(buffer, ImGuiTreeNodeFlags_DefaultOpen)) {
        snprintf(buffer, 120, "Friendly Name##lighting%d", hardware_channels[i].id);
        ImGui::InputText(
          buffer, hardware_channels[i].friendly_name, 
          MAX_HW_FRIENDLY_NAME_STR_LEN, ImGuiInputTextFlags_CallbackEdit, 
          friendly_name_input_callback, NULL);

        snprintf(buffer, 120, "Strip Enable##lighting%d", hardware_channels[i].id);
        ImGui::Checkbox(buffer, &hardware_channels[i].strip_enable);

        ImGui::BeginDisabled(hardware_channels[i].strip_enable == false);
        snprintf(buffer, 120, "Strip Length##lighting%d", hardware_channels[i].id);
        ImGui::InputInt(buffer, &hardware_channels[i].strip_length, 1, 20, ImGuiInputTextFlags_CharsDecimal);
        ImGui::EndDisabled();

        snprintf(buffer, 120, "RGB Enable##lighting%d", hardware_channels[i].id);
        ImGui::Checkbox(buffer, &hardware_channels[i].rgb_enable);
      }
      ImGui::Separator();
      ImGui::Spacing();
    }
    ImGui::End();
  }
}

void RenderHardwareTriggersWindow()
{
  static char buffer[120];
  if(hardware_window_state.selected) {
    ImGui::SetNextWindowSize(ImVec2(250, 90), ImGuiCond_FirstUseEver);
    if(!ImGui::Begin("Trigger Channel Config", &hardware_window_state.selected)) {
      ImGui::End();
      return;
    }

    ImGui::Text("Input Channels");
    for(int i = 0; i < MAX_HW_TRIGGERS; i++) {
      snprintf(buffer, 120, "Input Channel %d##%d", hardware_triggers[i].id, hardware_triggers[i].id);
      if(ImGui::CollapsingHeader(buffer, ImGuiTreeNodeFlags_DefaultOpen)) {
        snprintf(buffer, 120, "Enable##trigger%d", hardware_triggers[i].id);
        ImGui::Checkbox(buffer, &hardware_triggers[i].enable);

        ImGui::BeginDisabled(hardware_triggers[i].enable == false);

        snprintf(buffer, 120, "Friendly Name##trigger%d", hardware_triggers[i].id);
        ImGui::InputText(
          buffer, hardware_triggers[i].friendly_name, 
          MAX_HW_FRIENDLY_NAME_STR_LEN, ImGuiInputTextFlags_CallbackEdit, 
          friendly_name_input_callback, NULL);
        ImGui::EndDisabled();
        ImGui::Separator();
        ImGui::Spacing();
      }
    }

    ImGui::End();
  }
}

void RenderEffectSlotsWindow(void)
{
  // static char buffer[120];
  if(effect_slot_window_state.selected) {
    ImGui::SetNextWindowSize(ImVec2(250, 90), ImGuiCond_FirstUseEver);

    if(!ImGui::Begin("Effect Slots")) {
      ImGui::End();
      return;
    }

    for(int i = 0; i < MAX_EFFECT_SLOTS; i++) {
      // snprintf(buffer, 120, "Script Name##%d", effect_slots[i].id);
      // ImGui::InputText(
      //   buffer, effect_slots[i].script_name, 
      //   MAX_HW_FRIENDLY_NAME_STR_LEN, ImGuiInputTextFlags_CallbackEdit, 
      //   friendly_name_input_callback, NULL);
    }
    ImGui::End();
  }
}

void RenderScriptEditor()
{
  static char buffer[120];
  if(script_editor_window_state.selected) {
    ImGui::SetNextWindowSize(ImVec2(250, 90), ImGuiCond_FirstUseEver);
    if(!ImGui::Begin("TextEdit")) {
      ImGui::End();
      return;
    }

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    ImGui::BeginTabBar("text editor", tab_bar_flags);
    int old_id = script_editor_window_state.selected_id;
    int i = 0;
    Spect::ScriptListNode* node = NULL;
    assert(script_list);
    node = script_list->start;
    do {
      assert(node);
      assert(node->script);
      assert(node->script->name);
      if(
        node->script->action == Spect::DatabaseAction::SPECT_INSERT  ||
        node->script->action == Spect::DatabaseAction::SPECT_UPDATE
      ) tab_bar_flags |= ImGuiTabItemFlags_UnsavedDocument;
      else {tab_bar_flags = ImGuiTabBarFlags_None;}
      snprintf(buffer, 120, "%s", node->script->name);
      if(ImGui::BeginTabItem(buffer, &script_editor_window_state.selected, tab_bar_flags)) {
        ImGui::Text(node->script->description);
        script_editor_handle_tab(node, &script_editor_window_state, old_id, i);
        script_editor_window_state.editor->Render("ScriptEditor");
        if(script_editor_window_state.editor->IsTextChanged()) {
          edits_pending = true;
          script_editor_window_state.script_node->script->action = Spect::DatabaseAction::SPECT_UPDATE;
        }
        ImGui::EndTabItem();
      }

      i++;
      node = node->next;
    } while(node && i < script_list->count);

    if(ImGui::BeginTabItem("Add", NULL, ImGuiTabItemFlags_Trailing)) {
      static char script_name[MAX_EFFECT_SLOT_NAME_STR_LEN];
      static char script_description[MAX_EFFECT_SLOT_NAME_STR_LEN];
      ImGui::Text("Add New Script");
      ImGui::Separator();

      ImGui::InputText("Name", script_name, MAX_EFFECT_SLOT_NAME_STR_LEN);
      ImGui::InputTextMultiline("Description", script_description, MAX_EFFECT_SLOT_NAME_STR_LEN);

      ImGui::Separator();

      if(ImGui::Button("Save")) {
        script_editor_save_script(script_list, 
          &script_editor_window_state, 
          script_name, 
          script_description
        );
      }
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
  }
}

void Provision::Render(bool* requestDone)
{
  // todo: store default layout somewhere?
  const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

  if(show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
  if(show_save_modal) ImGui::OpenPopup("Edits Pending");
  if(show_about_modal) ImGui::OpenPopup("About");
  if(show_license_modal) ImGui::OpenPopup("3rd Party Licenses");

  RenderMainMenuBar(requestDone);
  RenderNavigationWindow();
  static char buffer[120];
  for(int i = 0; i < network_node_count; i++) {
    if(!node_windows[i].selected) continue;
    ImGui::SetNextWindowSize(ImVec2(250, 90), ImGuiCond_FirstUseEver);
    snprintf(buffer, 120, "Node %s ID=%d RSSI=%d ###node-edit-%d", network_nodes[i].name, network_nodes[i].node_id, network_nodes[i].rssi, i);
    if(!ImGui::Begin(buffer, &node_windows[i].selected)) {
      ImGui::End();
      return;
    }
    ImGui::Text("Node Actions");
    ImGui::Separator();
    ImGui::Button("Ping");
    ImGui::SameLine();
    
    ImGui::BeginDisabled(network_nodes[i].node_id == config->network_leader_id);
    if(ImGui::Button("Make Leader")) {
      edits_pending = true;
      config->network_leader_id = network_nodes[i].node_id;
    }
    ImGui::EndDisabled();

    ImGui::Separator();
    static float color[3] = {0.0f, 0.0f, 0.0f};
    ImGui::ColorPicker3("Solid", color);

    ImGui::End();
  }
  
  // Render Windows
  RenderHardwareChannelsWindow();
  RenderHardwareTriggersWindow();
  RenderEffectSlotsWindow();
  RenderScriptEditor();

  // Render Modals
  RenderEditModal();
  RenderAboutModal(config, &show_about_modal);
  RenderLicenseModal(&show_license_modal);
}

void Provision::Shutdown(void)
{
  path_free(config_file_path);
  config_file_path = NULL;
  db_close();

  NFD_Quit();
}
