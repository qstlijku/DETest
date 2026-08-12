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

	const ImGuiIO& io = ImGui::GetIO();

	if (ImGui::Button("Save")) {
		for (auto& it : world.sectors) {
			if (it.isSaveDirty) {
				it.save();
			}
			it.isSaveDirty = false;
		}
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
	/*ImGui::SameLine();
	if (ImGui::Button("Hole"))
		currentBrush = 4;*/

	ImGui::Separator();
	ImGui::Text("Brush Settings");
	//ImGui::Checkbox("Square Brush", &squareBrush);
	ImGui::SliderFloat("Brush Size", &radius, 1.f, 128.f);
	ImGui::SliderFloat("Brush Hardness", &hardness, 0.f, 1.f);
	if(currentBrush == 2)
		ImGui::DragFloat("Target Height", &targetHeight, 0.f, 255.f);

	//Get Cursor Ray
	glm::vec3 col = world.terrainCursor;

	if (col != glm::vec3(0)) {
		ImGui::Text("Col %f %f %f", col.x, col.y, col.z);

		float sectorSize = 64 * 4;

		//Get Sector we're on
		for (auto& it : world.sectors) {
			glm::vec3 bbMin(it.xPos, it.yPos, 0.f);
			bbMin *= 64;
			bbMin -= glm::vec3(world.GridsWorldOffset, 0.f);
			glm::vec3 bbMax(bbMin + sectorSize);
			
			bool isIn = (col.x >= bbMin.x && col.x <= bbMax.x) &&
				(col.y >= bbMin.y && col.y <= bbMax.y) &&
				(col.z >= bbMin.z && col.z <= bbMax.z);
			if (isIn) {
				dd::aabb(&bbMin.x, &bbMax.x, red);
				ImGui::Text("Sector %u %u %u", it.sectorID, it.xPos, it.yPos);

				glm::vec3 secPos = col - bbMin;

				ImGui::Text("InnerSector %f %f", secPos.x, secPos.y);

				auto hi = it.getHiRes();
				auto& map = hi->getMap(secPos.x / 64.f, secPos.y / 64.f);

				glm::ivec3 intPos(secPos);
				intPos %= 64;

				if (io.MouseDown[0]) {
					map.SetZ(intPos.x, intPos.y, 80.f);
					it.isSaveDirty = true;
				}

				break;
			}
		}

		if (squareBrush) {

		} else {
			dd::sphere(&col.x, red, radius);
			dd::sphere(&col.x, yellow, radius * hardness);
		}
	}
	

	ImGui::End();
}
