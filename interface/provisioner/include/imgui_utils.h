// Credit: https://gist.github.com/dougbinks/ef0962ef6ebe2cadae76c4e9f0586c69

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#include <ShellApi.h>
#endif

#include "imgui.h"
namespace ImGui
{
  // TODO: This is probably not the best place for this function
  // TODO: This function wont work in emscripten
  static void OpenURL(const char* path)
  {
  #ifdef _WIN32
      // Note: executable path must use backslashes!
      ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWNORMAL);
  #else
  #if __APPLE__
      const char* open_executable = "open";
  #else
      const char* open_executable = "xdg-open";
  #endif
      char command[256];
      snprintf(command, 256, "%s \"%s\"", open_executable, path);
      system(command);
  #endif
  }

	inline void AddUnderLine( ImColor col_ )
	{
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		min.y = max.y;
		ImGui::GetWindowDrawList()->AddLine( min, max, col_, 1.0f );
	}

  inline void TextURL( const char* name_, const char* URL_, uint8_t SameLineBefore_, uint8_t SameLineAfter_ )
  {
    if( 1 == SameLineBefore_ ){ ImGui::SameLine( 0.0f, ImGui::GetStyle().ItemInnerSpacing.x ); }
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
        ImGui::Text( name_ );
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered())
		{
			if( ImGui::IsMouseClicked(0) )
			{
				OpenURL(URL_);
			}
			AddUnderLine( ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] );
			ImGui::SetTooltip("  Open in browser\n%s", URL_ );
		}
		else
		{
			AddUnderLine( ImGui::GetStyle().Colors[ImGuiCol_Button] );
		}
		if( 1 == SameLineAfter_ ){ ImGui::SameLine( 0.0f, ImGui::GetStyle().ItemInnerSpacing.x ); }
  }
}