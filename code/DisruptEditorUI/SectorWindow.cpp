#include "ImguiWindows.h"

#include "CSector.h"
#include "World.h"
#include "imgui.h"
#include "Common.h"
#include "SDL.h"
#include "DDRenderInterface.h"
#include "debug_draw.hpp"

void UI::displaySector() {
	if (!settings.openWindows["Sector"])
		return;
	if (!ImGui::Begin("Sector", &settings.openWindows["Sector"], ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::End();
		return;
	}

	if (ImGui::Button("Save")) {

	}

	//Brush Settings
	static int currentBrush = 0;
	static bool squareBrush = false;
	static float radius = 10.f;
	static float hardness = 1.f;
	static float targetHeight = 70.f;
	bool modifyer = SDL_GetModState() & KMOD_LCTRL;
	static glm::vec3 cursorPos;

	ImGui::Text("Brushes");
	if (ImGui::Button("Raise/Lower"))
		currentBrush = 0;
	ImGui::SameLine();
	if (ImGui::Button("Flatten"))
		currentBrush = 1;
	ImGui::SameLine();
	if (ImGui::Button("Set Height"))
		currentBrush = 2;
	ImGui::SameLine();
	if (ImGui::Button("Ramp"))
		currentBrush = 3;
	ImGui::SameLine();
	if (ImGui::Button("Hole"))
		currentBrush = 4;

	ImGui::Separator();
	ImGui::Text("Brush Settings");
	ImGui::Checkbox("Square Brush", &squareBrush);
	ImGui::SliderFloat("Brush Size", &radius, 1.f, 128.f);
	ImGui::SliderFloat("Brush Hardness", &hardness, 0.f, 1.f);
	if(currentBrush == 2)
		ImGui::DragFloat("Target Height", &targetHeight, 0.f, 255.f);

	//Get Cursor Ray
	glm::vec3 col = world.terrainCursor;

	if (col != glm::vec3(0)) {
		ImGui::Text("Col %f %f %f", col.x, col.y, col.z);
		if (squareBrush) {

		} else {
			dd::sphere(&col.x, red, radius);
			dd::sphere(&col.x, yellow, radius * hardness);
		}
	}
	

	ImGui::End();
}
