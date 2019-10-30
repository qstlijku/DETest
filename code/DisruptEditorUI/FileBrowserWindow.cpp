#include "ImguiWindows.h"

#include "imgui.h"
#include "imgui_memory_editor.h"
#include "Common.h"
#include "DB.h"
#include "FileHandler.h"
#include "sqlite_modern_cpp.h"
#include "noc_file_dialog.h"

struct NodeEntry;

class NodeEntryCollection : public std::map<std::string, NodeEntry> {
public:
	void AddEntry(std::string sEntry, int wBegIndex = 0);
};

struct NodeEntry {
	std::string Key;
	NodeEntryCollection Children;
};

void NodeEntryCollection::AddEntry(std::string sEntry, int wBegIndex) {
	if (wBegIndex < sEntry.size()) {
		int wEndIndex = sEntry.find("\\", wBegIndex);
		if (wEndIndex == std::string::npos)
			wEndIndex = sEntry.size();
		std::string sKey = sEntry.substr(wBegIndex, wEndIndex - wBegIndex);
		if (!sKey.empty()) {
			NodeEntry& oItem = (*this)[sKey];

			if(oItem.Key.empty())
				oItem.Key = sKey;

			// Now add the rest to the new item's children
			oItem.Children.AddEntry(sEntry, wEndIndex + 1);
		}
	}
}

static NodeEntryCollection root;
static std::string currentFile;
static std::vector<uint8_t> currentFileData;

static void populateFolders() {
	root.clear();
	*DB::instance().db << "SELECT DISTINCT path FROM files WHERE type != 'CArchetypeResource' AND type != 'CParticlesSystemParamResource' AND type != '';" >> [&](std::string file) {
		if(FH::fileExists(file.c_str()))
			root.AddEntry(file);
	};
}

static void recurseNodeEntryCollection(NodeEntryCollection& collection, std::string path = "") {
	for (auto& it : collection) {
		if (it.second.Children.empty()) {
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

	if (root.empty())
		populateFolders();

	ImGui::Columns(2);

	/*static char searchBuffer[120] = { 0 };
	ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));*/

	ImGui::BeginChild("##FileList");
	recurseNodeEntryCollection(root);
	ImGui::EndChild();

	ImGui::NextColumn();

	if (!currentFile.empty()) {
		ImGui::Text("%s", currentFile.c_str());
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(FH::getFileLocations(currentFile.c_str()).c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		ImGui::Text("%s", FH::getTypeFromExtension(currentFile.c_str()));

		if (ImGui::Button("Save")) {
			SDL_RWops* fp = FH::openFileWrite(currentFile.c_str());
			SDL_RWwrite(fp, currentFileData.data(), 1, currentFileData.size());
			SDL_RWclose(fp);
		}

		static MemoryEditor mem_edit;
		mem_edit.DrawContents(currentFileData.data(), currentFileData.size());
	}

	ImGui::End();
}
