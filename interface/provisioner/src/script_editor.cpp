#include "imgui.h"
#include "ImGuiColorTextEdit/TextEditor.h"

#include "provision.h"

TextEditor editor;

void RenderScriptEditor()
{
	auto lang = TextEditor::LanguageDefinition::Lua();
  static const char* ppnames[] = {};
  static const char* ppvalues[] = { };
  for (int i = 0; i < sizeof(ppnames) / sizeof(ppnames[0]); ++i)
  {
		TextEditor::Identifier id;
		id.mDeclaration = ppvalues[i];
		lang.mPreprocIdentifiers.insert(std::make_pair(std::string(ppnames[i]), id));
	}
  static const char* identifiers[] = {};
  static const char* idecls[] = {};
	for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = std::string(idecls[i]);
		lang.mIdentifiers.insert(std::make_pair(std::string(identifiers[i]), id));
	}
	editor.SetLanguageDefinition(lang);

	// TextEditor::ErrorMarkers markers;
	// markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
	// markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
	// editor.SetErrorMarkers(markers);
  ImGui::Begin("TextEdit");
  editor.Render("TextEditor");
  ImGui::End();
}