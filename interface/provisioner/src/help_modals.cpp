#include "imgui.h"
#include "imgui_utils.h"

#include "gitparams.h"
#include "spect.h"

void RenderAboutModal(struct Spect::Config* config, bool* show_about_modal)
{
  // Always center this window when appearing
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  auto color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

  if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("ConeRGB Configurator");
    ImGui::Separator();

    ImGui::Text("Author: ");
    ImGui::SameLine();
    ImGui::TextColored(color, "Connor Rigby");

    ImGui::Text("Copyright: ");
    ImGui::SameLine();
    ImGui::TextColored(color, "Connor Rigby 2022");
    ImGui::SameLine(); ImGui::TextURL("Apache 2", "https://www.apache.org/licenses/LICENSE-2.0", false, false);

    ImGui::Text("Version: ");
    ImGui::SameLine();
    ImGui::TextColored(color, "%s", git_describe());

    ImGui::Text("Commit: ");
    ImGui::SameLine();
    ImGui::TextColored(color, "%s", git_commit());

    ImGui::Text("Build Date: ");
    ImGui::SameLine();
    ImGui::TextColored(color, "%s", build_epoch());
    ImGui::Spacing();

    ImGui::TextURL("Report an issue", "https://github.com/miata-bot/can-link/issues/new", false, false);

    if(config) {
      ImGui::Spacing(); ImGui::Separator();
      ImGui::Text("Spect Database");
      ImGui::Separator();

      ImGui::Text("Database Version: ");
      ImGui::SameLine();
      ImGui::TextColored(color, "%d", config->version);
      ImGui::Spacing(); ImGui::Separator();
    }

    if (ImGui::Button("OK", ImVec2(120, 0))) { 
      *show_about_modal = false;
      ImGui::CloseCurrentPopup(); 
    }
    ImGui::EndPopup();
  }
}

void RenderLicenseModal(bool* show_license_modal)
{
  // Always center this window when appearing
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("3rd Party Licenses", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Open Source Licenses");
    ImGui::Text("This program uses the following 3rd party libraries");
    ImGui::Separator();

    ImGui::TextURL("Simple DirectMedia Layer", "https://www.libsdl.org/", false, false);
    ImGui::SameLine(); 
    ImGui::Text("License: "); ImGui::SameLine();
    ImGui::TextURL("Zlib", "https://www.libsdl.org/license.php", false, false);
    ImGui::Separator();

    ImGui::TextURL("Dear ImGui", "https://github.com/ocornut/imgui", false, false);
    ImGui::SameLine();
    ImGui::Text("License: "); ImGui::SameLine();
    ImGui::TextURL("MIT", "https://raw.githubusercontent.com/ocornut/imgui/master/LICENSE.txt", false, false);
    ImGui::Separator();

    ImGui::TextURL("ImGuiColorTextEdit", "https://github.com/BalazsJako/ImGuiColorTextEdit/", false, false);
    ImGui::SameLine();
    ImGui::Text("License: "); ImGui::SameLine();
    ImGui::TextURL("MIT", "https://raw.githubusercontent.com/BalazsJako/ImGuiColorTextEdit/master/LICENSE", false, false);
    ImGui::Separator();

    ImGui::TextURL("nativefiledialog-extended", "https://github.com/btzy/nativefiledialog-extended", false, false);
    ImGui::SameLine();
    ImGui::Text("License: "); ImGui::SameLine();
    ImGui::TextURL("Zlib", "https://raw.githubusercontent.com/btzy/nativefiledialog-extended/master/LICENSE", false, false);
    ImGui::Separator();

    ImGui::TextURL("SQLite", "https://www.sqlite.org/", false, false);
    ImGui::SameLine();
    ImGui::Text("License: "); ImGui::SameLine();
    ImGui::TextURL("Public Domain", "https://www.sqlite.org/copyright.html", false, false);
    ImGui::Separator();

    ImGui::Text("This program is released open source under the Apache-2 license");
    ImGui::Separator();
    ImGui::TextURL("LICENSE", "https://github.com/miata-bot/can-link/blob/main/interface/provisioner/LICENSE.txt", false, false);

    if (ImGui::Button("OK", ImVec2(120, 0))) { 
      *show_license_modal = false;
      ImGui::CloseCurrentPopup(); 
    }
    ImGui::EndPopup();
  }
}