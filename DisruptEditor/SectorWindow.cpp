#include "ImguiWindows.h"

#include "CSector.h"
#include "World.h"
#include "imgui.h"
#include "Common.h"

void UI::displaySector() {
	if (!settings.openWindows["Sector"])
		return;
	if (!ImGui::Begin("Materials", &settings.openWindows["Sector"], ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::End();
		return;
	}

	if (ImGui::Button("Save")) {

	}

	ImGui::End();
}
