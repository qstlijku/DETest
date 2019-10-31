#include "ImguiWindows.h"

#include "imgui.h"
#include "imgui_memory_editor.h"
#include "Common.h"
#include "DB.h"
#include "FileHandler.h"
#include "noc_file_dialog.h"

static std::string currentFile;
static std::vector<uint8_t> currentFileData;

static void recurseNodeEntryCollection(NodeEntryCollection& collection, std::string path = "") {
	for (auto& it : collection) {
		if (it.second.Children.empty()) {
			std::string_view type = FH::getTypeFromExtension((path + it.first).c_str());
			if (type.empty() || type == "CArchetypeResource" || type == "CParticlesSystemParamResource")
				continue;

			ImGui::TreeNodeEx(it.first.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
			if (ImGui::IsItemClicked()) {
				currentFile = path + it.first;

				SDL_RWops* fp = FH::openFile(currentFile.c_str());
				if (fp) {
					currentFileData.resize(SDL_RWsize(fp));
					SDL_RWread(fp, currentFileData.data(), 1, currentFileData.size());

					SDL_RWclose(fp);
				}
			}
		} else {
			if (ImGui::TreeNode(it.first.c_str())) {
				recurseNodeEntryCollection(it.second.Children, path + it.first + '\\');

				ImGui::TreePop();
			}
		}
	}
}

void UI::displayFileBrowser() {
	if (!settings.openWindows["FileBrowser"])
		return;
	if (!ImGui::Begin("File Browser", &settings.openWindows["FileBrowser"], 0)) {
		ImGui::End();
		return;
	}

	ImGui::Columns(2);

	/*static char searchBuffer[120] = { 0 };
	ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));*/

	ImGui::BeginChild("##FileList");
	recurseNodeEntryCollection(DB::instance().root);
	ImGui::EndChild();

	ImGui::NextColumn();

	if (!currentFile.empty()) {
		std::string_view type = FH::getTypeFromExtension(currentFile.c_str());

		ImGui::Text("%s", currentFile.c_str());
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(FH::getFileLocations(currentFile.c_str()).c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		ImGui::Text("%s", type.data());

		if (type == "CGeometryResource") {
			ImVec2 size = ImGui::GetContentRegionAvail();



			ImGui::Image(0, size);
		} else if (type == "CGeometryMipResource") {
			ImGui::Text("You can't edit a mip resource, find its xbg!");
		} else {
			if (ImGui::Button("Save")) {
				SDL_RWops* fp = FH::openFileWrite(currentFile.c_str());
				SDL_RWwrite(fp, currentFileData.data(), 1, currentFileData.size());
				SDL_RWclose(fp);
			}
			static MemoryEditor mem_edit;
			mem_edit.DrawContents(currentFileData.data(), currentFileData.size());
		}

		
	}

	ImGui::End();
}
