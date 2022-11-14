#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "imgui.h"
#include "TextEditor.h"

#include "provision.h"
#include "spect.h"

struct ScriptEditorWindowState {
  bool                          selected;
  int                           selected_id;
  TextEditor*                   editor;
  std::string*                  editor_buffer;
  struct Spect::ScriptListNode* script_node;
};

void script_editor_write_buffer(struct ScriptEditorWindowState* script_editor_window_state)
{
  assert(script_editor_window_state->script_node->script);
  assert(script_editor_window_state->script_node->script->content);
  memset(script_editor_window_state->script_node->script->content, 0, 32000 * sizeof(char));
  memcpy(
    script_editor_window_state->script_node->script->content,
    script_editor_window_state->editor->GetText().c_str(),
    script_editor_window_state->editor->GetText().length()
  );
}

void script_editor_handle_tab(
	struct Spect::ScriptListNode* node,
	struct ScriptEditorWindowState* script_editor_window_state,
	int old_id, int id
)
{
	if(old_id == -1) {
		assert(script_editor_window_state->script_node == NULL);
		if(script_editor_window_state->editor_buffer) delete script_editor_window_state->editor_buffer;
		script_editor_window_state->editor_buffer = new std::string(node->script->content);
		script_editor_window_state->editor->SetText(*script_editor_window_state->editor_buffer);
		script_editor_window_state->script_node = node;
		script_editor_window_state->selected_id = id;
	} else if(old_id != id) {
		script_editor_write_buffer(script_editor_window_state);
		delete script_editor_window_state->editor_buffer;
		script_editor_window_state->editor_buffer = new std::string(node->script->content);
		script_editor_window_state->editor->SetText(*script_editor_window_state->editor_buffer);
		if(node->script->action == Spect::DatabaseAction::SPECT_NONE) node->script->action = Spect::DatabaseAction::SPECT_UPDATE;
		script_editor_window_state->script_node = node;
		script_editor_window_state->selected_id = id;
	} else {/* nothing to see here? */}
}

void script_editor_save_script(
	struct Spect::ScriptList* script_list,
	struct ScriptEditorWindowState* script_editor_window_state,
	char*  script_name,
	char*  script_description
) 
{
	struct Spect::ScriptListNode* node = NULL;
	node = script_list->start;
	Spect::ScriptListNode* last = NULL;
	while(node) { last = node; node = node->next; }
	node = last;
	assert(node);
	node->next = (struct Spect::ScriptListNode*) malloc(sizeof(struct Spect::ScriptListNode));
	assert(node->next);
	memset(node->next, 0, sizeof(struct Spect::ScriptListNode));

	node = node->next;
	node->script = (struct Spect::Script*) malloc(sizeof(struct Spect::Script));
	assert(node->script);
	memset(node->script, 0, sizeof(struct Spect::Script));
	node->script->name = (char*) malloc(MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));
	assert(node->script->name);
	memset(node->script->name, 0, MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));
	memcpy(node->script->name, script_name, strnlen(script_name, MAX_EFFECT_SLOT_NAME_STR_LEN));
	memset(script_name, 0, MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));
	node->script->description = (char*) malloc(MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));
	assert(node->script->description);
	memset(node->script->description, 0, MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));
	memcpy(node->script->description, script_description, strnlen(script_description, MAX_EFFECT_SLOT_NAME_STR_LEN));
	memset(script_description, 0, MAX_EFFECT_SLOT_NAME_STR_LEN * sizeof(char));
	node->script->content = (char*) malloc(32000 * sizeof(char));
	assert(node->script->content);
	memset(node->script->content, 0, 32000 * sizeof(char));
	node->script->action = Spect::DatabaseAction::SPECT_INSERT;
	
	script_list->count++;
	assert(script_editor_window_state->script_node);
	memset(script_editor_window_state->script_node->script->content, 0, 32000 * sizeof(char));
	memcpy(
		script_editor_window_state->script_node->script->content,
		script_editor_window_state->editor->GetText().c_str(),
		script_editor_window_state->editor->GetText().length()
	);
	delete script_editor_window_state->editor_buffer;
	script_editor_window_state->editor_buffer = new std::string(node->script->content);
	script_editor_window_state->editor->SetText(*script_editor_window_state->editor_buffer);
	script_editor_window_state->script_node = node;
	script_editor_window_state->selected_id = script_list->count;
}
