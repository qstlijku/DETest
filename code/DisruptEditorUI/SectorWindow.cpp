#include "ImguiWindows.h"

#include "CSector.h"
#include "World.h"
#include "imgui.h"
#include "Common.h"
#include "SDL.h"
#include "DDRenderInterface.h"
#include "debug_draw.hpp"

static glm::vec3 Intersect(glm::vec3 planeP, glm::vec3 planeN, glm::vec3 rayP, glm::vec3 rayD) {
	float d = glm::dot(planeP, -planeN);
	float t = -(d + rayP.z * planeN.z + rayP.y * planeN.y + rayP.x * planeN.x) / (rayD.z * planeN.z + rayD.y * planeN.y + rayD.x * planeN.x);
	return rayP + t * rayD;
}

void UI::displaySector() {
	if (!settings.openWindows["Sector"])
		return;
	if (!ImGui::Begin("Sector", &settings.openWindows["Sector"], ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::End();
		return;
	}

	if (ImGui::Button("Save")) {

	}
	ImGui::SameLine();
	if (ImGui::Button("Export OBJ")) {

	}
	ImGui::SameLine();
	if (ImGui::Button("Import OBJ")) {

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
	glm::vec3 rayFrom(RenderInterface::instance().camera.location);
	glm::vec3 rayTo;
	{
		glm::ivec2 mouse;
		uint32_t mouseButton = SDL_GetMouseState(&mouse.x, &mouse.y);
		glm::vec2 mousePos = mouse;

		float top = 1.f;
		float bottom = -1.f;
		float nearPlane = RenderInterface::instance().camera.near_plane;
		float farPlane = RenderInterface::instance().camera.far_plane;
		float fov = RenderInterface::instance().camera.fov;

		glm::vec3 camTarget(RenderInterface::instance().camera.lookingAt);

		glm::vec3 rayForward = glm::normalize(camTarget - rayFrom);
		rayForward *= farPlane;

		glm::vec3 vertical(RenderInterface::instance().camera.up);

		glm::vec3 hor = glm::normalize(glm::cross(rayForward, vertical));
		vertical = glm::normalize(glm::cross(hor, rayForward));

		float tanfov = tanf(0.5f * fov);

		hor *= 2.f * farPlane * tanfov;
		vertical *= 2.f * farPlane * tanfov;

		float width = RenderInterface::instance().windowSize.x;
		float height = RenderInterface::instance().windowSize.y;
		float aspect = width / height;

		hor *= aspect;

		glm::vec3 rayToCenter = rayFrom + rayForward;
		glm::vec3 dHor = hor * 1.f / width;
		glm::vec3 dVert = vertical * 1.f / height;

		rayTo = rayToCenter - 0.5f * hor + 0.5f * vertical;
		rayTo += mousePos.x * dHor;
		rayTo -= mousePos.y * dVert;
		rayTo = glm::normalize(rayTo);
	}

	glm::vec3 col = Intersect(glm::vec3(0, 0, 70.f), glm::vec3(0, 0, 1), rayFrom, rayTo);

	if (col.x >= -2048 && col.x <= 2048 && col.y >= -2560 && col.y <= 2560) {
		ImGui::Text("Col %f %f %f", col.x, col.y, col.z);
		if (squareBrush) {

		} else {
			dd::sphere(&col.x, red, radius);
			dd::sphere(&col.x, yellow, radius * hardness);
		}
	}
	

	ImGui::End();
}
