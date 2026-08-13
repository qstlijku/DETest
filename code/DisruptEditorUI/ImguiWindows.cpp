#include "ImguiWindows.h"

#include "imgui.h"
#include "Common.h"
#include "Hash.h"
#include "World.h"
#include "Version.h"
#include "FileHandler.h"

#include <Windows.h>
#include <shellapi.h>
#include <SDL.h>
#include <list>

void UI::displayTopMenu() {
	std::map<std::string, bool> &windows = settings.openWindows;

	ImGui::BeginMainMenuBar();
	if (ImGui::MenuItem("File Browser"))
		windows["FileBrowser"] ^= true;
	if (ImGui::MenuItem("Entity Library"))
		windows["EntityLibrary"] ^= true;
	if (ImGui::MenuItem("CBatch"))
		windows["CBATCH"] ^= true;
	if (ImGui::MenuItem("Sector"))
		windows["Sector"] ^= true;
	/*if (ImGui::MenuItem("Domino"))
		windows["Domino"] ^= true;*/
	if (ImGui::MenuItem("DARE"))
		windows["DARE"] ^= true;
	/*if (ImGui::MenuItem("Sequence"))
		windows["CSequence"] ^= true;
	if (ImGui::MenuItem("Move"))
		windows["Move"] ^= true;
	if (ImGui::MenuItem("LocString"))
		windows["LocString"] ^= true;
	if (ImGui::MenuItem("SpawnPoint"))
		windows["SpawnPoint"] ^= true;*/
	if (ImGui::BeginMenu("Hasher")) {
		static char buffer[255] = { '\0' };
		ImGui::InputText("##UID", buffer, sizeof(buffer));
		uint32_t fnv = Hash::getFilenameHash(buffer);
		uint64_t fnv64 = Hash::getFilenameHash64(buffer);
		uint32_t crc = Hash::getHash(buffer);

		char outbuffer[255];
		snprintf(outbuffer, sizeof(outbuffer), "%u", fnv);
		ImGui::InputText("FNV##UIDOUT", outbuffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);
		snprintf(outbuffer, sizeof(outbuffer), "%u", crc);
		ImGui::InputText("CRC##UIDOUT", outbuffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);
		snprintf(outbuffer, sizeof(outbuffer), "%llu", fnv64);
		ImGui::InputText("FNV64##UIDOUT", outbuffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Settings")) {
		ImGui::DragFloat3("CameraPos", &RenderInterface::instance().camera.location.x);
		ImGui::DragFloat3("LookingAt", &RenderInterface::instance().camera.lookingAt.x);
		//ImGui::DragFloat3("Up", &RenderInterface::instance().camera.up.x);
		ImGui::DragFloat("Theta", &RenderInterface::instance().camera.theta);
		ImGui::DragFloat("Phi", &RenderInterface::instance().camera.phi);
		ImGui::DragFloat("Radius", &RenderInterface::instance().camera.radius);
		ImGui::DragFloat("Lon", &RenderInterface::instance().camera.lon);
		ImGui::DragFloat("Lat", &RenderInterface::instance().camera.lat);
		ImGui::DragFloat("Near Plane", &settings.near_plane, 0.02f, 0.001f, 10.f);
		ImGui::DragFloat("Far Plane", &settings.far_plane, 1.f, 10.f, 6500.f);
		ImGui::DragFloat("Fov", &settings.fov, 0.02f, 0.001f, 3.14159f);
		ImGui::DragFloat("Camera Fly Multiplier", &settings.flyMultiplier, 0.02f, 0.1f, 10.f);
		ImGui::DragFloat("Draw Distance", &settings.drawDistance, 64.f, 0.f, 4096.f);
		ImGui::Checkbox("Draw Splines", &settings.drawSplines);
		ImGui::Checkbox("Draw Terrain", &settings.drawTerrain);
		ImGui::Checkbox("Draw Near", &settings.displayNear);
		ImGui::Checkbox("Draw Far", &settings.displayFar);
		ImGui::Checkbox("Draw World", &settings.displayWorld);
		ImGui::Checkbox("Draw Details", &settings.drawDetails);
		ImGui::Checkbox("Draw Models", &settings.drawModels);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Credits")) {
		ImGui::Text("Disrupt Editor v%s", DE_VERSIONSTR);
		if (ImGui::Selectable("Check for updates"))
			ShellExecute(0, 0, L"https://github.com/j301scott/DisruptEditor/releases", 0, 0, SW_SHOW);
		ImGui::Separator();
		if (ImGui::Selectable("Watch Dogs Modding Discord"))
			ShellExecute(0, 0, L"https://discord.gg/rTQcfDD", 0, 0, SW_SHOW);
		ImGui::Separator();
		if (ImGui::Selectable("FCBastard - Fireboyd78"))
			ShellExecute(0, 0, L"https://github.com/Fireboyd78", 0, 0, SW_SHOW);
		if (ImGui::Selectable("Premake help - Force67"))
			ShellExecute(0, 0, L"https://github.com/Force67", 0, 0, SW_SHOW);
		if (ImGui::Selectable("Gibbed.Disrupt - Gibbed"))
			ShellExecute(0, 0, L"https://github.com/gibbed", 0, 0, SW_SHOW);
		if (ImGui::Selectable("Disrupt Editor - Jon"))
			ShellExecute(0, 0, L"https://github.com/j301scott", 0, 0, SW_SHOW);
		if (ImGui::Selectable("Gibbed.Disrupt Updates - MrWasdennnoch"))
			ShellExecute(0, 0, L"https://github.com/wasdennnoch", 0, 0, SW_SHOW);
		ImGui::EndMenu();
	}

	if (settings.devTools && ImGui::MenuItem("Dev")) {
		windows["DevTools"] ^= true;
	}

	ImGui::EndMainMenuBar();
}

std::list<std::string> xbgs;
int item_selected_idx;

void UI::handleFNVFile(const char* file) {
	FILE* fp = fopen(file, "r");
	char line[512];
	while (fgets(line, sizeof(line), fp)) {
		line[strlen(line) - 1] = '\0';
		std::string lineStr(line);
		long x = lineStr.rfind(".xbg");
		if (x == strlen(line) - 4)
			xbgs.push_back(lineStr);
	}
	int i = 0;
	// one-time initialization of list box here
	for (std::string filename : xbgs)
	{
		long x = filename.rfind("char01.xbg");
		if (x > 0)
			item_selected_idx = i;
		i++;
	}
	fclose(fp);
	world.xbgs = xbgs;
}

bool first = true;

void UI::displayTempWindows() {
	if (xbgs.size() == 0)
	{
		std::string base = SDL_GetBasePath();
		// Initialize list for the first time
		handleFNVFile((base + "res/Watch Dogs.filelist").c_str());
	}
	if (ImGui::Begin("XBGs"))
	{
		if (ImGui::BeginListBox("Filelist"))
		{
			int n = 0;
			for (std::string file : xbgs)
			{
				const bool is_selected = (item_selected_idx == n);
				if (ImGui::Selectable(file.c_str(), is_selected))
				{
					item_selected_idx = n;
					world.selectedModel = file;
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
					if (first)
					{
						ImGui::SetScrollHereY();
						first = false;
					}
				}
				n++;
			}
			ImGui::EndListBox();
		}

		const char* items[] = { "XYZ", "XZY", "YXZ", "YZX", "ZXY", "ZYX"};
		static int item_selected_idx = 5; // Here we store our selection data as an index.

		// Pass in the preview value visible before opening the combo (it could technically be different contents or not pulled from items[])
		const char* combo_preview_value = items[item_selected_idx];
		if (ImGui::BeginCombo("rotation order", combo_preview_value))
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				const bool is_selected = (item_selected_idx == n);
				if (ImGui::Selectable(items[n], is_selected))
				{
					item_selected_idx = n;
					world.rotationOrder = items[n];
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}
	ImGui::End();
}

	/*ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(80.f, 80.f), ImGuiCond_FirstUseEver);
		if (windows["LocString"] && ImGui::Begin("LocString", &windows["LocString"], 0)) {
			auto &locStrings = Dialog::instance().locStrings;
			auto &soundidlinelinks = Dialog::instance().soundidlinelinks;

			static spkFile file;
			char search[500] = {0};
			ImGui::InputText("Search", search, sizeof(search));

			for (auto it : locStrings) {
				//ImGui::Text("%i %s", it.first, it.second.c_str());

				if (it.second.find(search) == std::string::npos && strlen(search) != 0)
					continue;

				if (soundidlinelinks.count(it.first)) {
					char imguiline[500];
					char buffer[500];
					snprintf(imguiline, sizeof(imguiline), "%i %08x %s", it.first, soundidlinelinks[it.first], it.second.c_str());
					snprintf(buffer, sizeof(buffer), "soundbinary\\%08x.spk", soundidlinelinks[it.first]);
					if (getAbsoluteFilePath(buffer).empty()) {
						soundidlinelinks.erase(it.first);
						continue;
					}
					if (ImGui::Selectable(imguiline)) {
						file.sbao.layers.clear();

						Audio::instance().stopAll();
						file.open(getAbsoluteFilePath(buffer).c_str());
						if(!file.sbao.layers.empty())
							file.sbao.layers[0].play(false);
					}
				}
			}

			ImGui::End();
		}

	ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(80.f, 80.f), ImGuiCond_FirstUseEver);
	if (windows["SpawnPoint"] && ImGui::Begin("SpawnPoint List", &windows["SpawnPoint"], 0)) {
		for (tinyxml2::XMLElement *SpawnPoint = world.spawnPointList->RootElement()->FirstChildElement(); SpawnPoint; SpawnPoint = SpawnPoint->NextSiblingElement()) {
			//glm::vec3 Position = SpawnPoint->Attribute("Position");
		}

		ImGui::End();
	}

	ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(80.f, 80.f), ImGuiCond_FirstUseEver);
	if (windows["CSequence"] && ImGui::Begin("CSequence", &windows["CSequence"], 0)) {
		static std::string currentFile;
		static cseqFile file;
		ImGui::Text("%s", currentFile.c_str());
		ImGui::SameLine();
		if (ImGui::Button("Open")) {
			//currentFile = noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, "cseq\0*.cseq\0", getAbsoluteFilePath("sequences").c_str(), nullptr);
			//file.open(currentFile.c_str());
		}

		ImGui::End();
	}*/

void UI::displayWindows() {
#define DEFINE_WINDOW(x) display ##x ();
#include "Windows.def"
#undef DEFINE_WINDOW
}
