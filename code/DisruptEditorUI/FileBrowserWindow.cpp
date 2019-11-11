#include "ImguiWindows.h"

#include "imgui.h"
#include "imgui_memory_editor.h"
#include "Common.h"
#include "DB.h"
#include "FileHandler.h"
#include "noc_file_dialog.h"
#include "ResourceLoader.h"
#include "xbgFile.h"
#include "xbtFile.h"
#include <Serialization.h>
#include <SDL_clipboard.h>

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

			std::shared_ptr<xbgFile> xbg = loadXBG(currentFile);

			static int selLod = 0;
			ImGui::SliderInt("Lod", &selLod, 0, xbg->lods.size() - 1);

			ID3D11DeviceContext *context = RenderInterface::instance().g_pd3dDeviceContext;
			context->VSSetShader(RenderInterface::instance().model.pVertexShader, NULL, NULL);
			context->PSSetShader(RenderInterface::instance().model.pPixelShader, NULL, NULL);
			RenderInterface::instance().objectCB.Model = glm::mat4(1.f);
			xbg->draw(context, selLod);

			if (ImGui::Button("XML")) {
				std::string xml = serializeToXML(*xbg);
				SDL_SetClipboardText(xml.c_str());
			}
		} else if (type == "CGeometryMipResource") {
			ImGui::Text("You can't edit a mip resource, find its xbg!");
		} else if (type == "CTextureResource") {
			std::shared_ptr<xbtFile> xbt = loadTexture(currentFile.c_str());
			ImGui::Image(xbt->pResource, ImGui::GetContentRegionAvail());
		} else if (type == "CSkeletonResource") {
			if (ImGui::Button("XML")) {
				SDL_RWops* fp = FH::openFile(currentFile.c_str());
				if (fp) {
					Node root = readFCB(fp);
					SDL_RWclose(fp);

					tinyxml2::XMLPrinter printer;
					root.serializeXML(printer);

					SDL_SetClipboardText(printer.CStr());
				}
			}
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
