#include "ImguiWindows.h"

#include "imgui.h"
#include "Common.h"

void UI::displayFileBrowser() {
	if (!settings.openWindows["FileBrowser"])
		return;
	if (!ImGui::Begin("File Browser", &settings.openWindows["FileBrowser"], 0)) {
		ImGui::End();
		return;
	}

	ImGui::End();
}
