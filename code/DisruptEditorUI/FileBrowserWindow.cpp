#include "ImguiWindows.h"

#include "imgui.h"
#include "imgui_memory_editor.h"
#include "imgui_stdlib.h"
#include "Common.h"
#include "DB.h"
#include "FileHandler.h"
#include "noc_file_dialog.h"
#include "ResourceLoader.h"
#include "materialFile.h"
#include "DriverShaders.h"
#include "xbgFile.h"
#include "xbtFile.h"
#include <Serialization.h>
#include <SDL.h>

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/BufferBuilder.h>
#include <GLTFSDK/GLTFResourceWriter.h>
#include <GLTFSDK/GLBResourceWriter.h>
#include <GLTFSDK/IStreamWriter.h>
#include <GLTFSDK/Serialize.h>

static std::string currentFile;
static std::vector<uint8_t> currentFileData;
static char searchBuffer[120] = { 0 };

static void recurseNodeEntryCollection(NodeEntryCollection& collection, std::string path = "") {
	for (auto& it : collection) {
		if (it.second.Children.empty()) {
			std::string fullPath = path + it.first;
			if (fullPath.find(searchBuffer) == std::string::npos)
				continue;

			std::string_view type = FH::getTypeFromExtension(fullPath.c_str());
			if (type.empty() || type == "CArchetypeResource" || type == "CParticlesSystemParamResource")
				continue;

			ImGui::TreeNodeEx(it.first.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
			if (ImGui::IsItemClicked()) {
				currentFile = fullPath;

				SDL_RWops* fp = FH::openFile(currentFile.c_str());
				if (fp) {
					currentFileData.resize(SDL_RWsize(fp));
					SDL_RWread(fp, currentFileData.data(), 1, currentFileData.size());

					SDL_RWclose(fp);
				}
			}
		} else {
			bool open = ImGui::TreeNode(it.first.c_str());
			if (ImGui::BeginPopupContextItem()) {
				ImGui::EndPopup();
			}
			if (open) {
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

	ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));

	ImGui::BeginChild("##FileList");
	recurseNodeEntryCollection(DB::instance().root);
	ImGui::EndChild();

	ImGui::NextColumn();
	ImGui::BeginChild("##FileEditor");

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
		} else if (type == "CMaterialResource") {
			std::shared_ptr<materialFile> mat = loadMaterial(currentFile.c_str());

			if (ImGui::Button("Save")) {
				SDL_RWops* fp = FH::openFileWrite(currentFile);
				if (fp) {
					CBinaryArchiveWriter writer(fp);
					mat->open(writer);
					SDL_RWclose(fp);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("XML")) {
				std::string xml = serializeToXML(*mat);
				SDL_SetClipboardText(xml.c_str());
			}
			ImGui::SameLine();
			if (ImGui::Button("XML Import")) {
				char *str = SDL_GetClipboardText();
				unserializeFromXML(*mat, str);
				SDL_free(str);
			}

			ImGui::InputText("Material Name", &mat->name);
			if (ImGui::BeginCombo("Material Descriptor", mat->shaderName.c_str()))  {
				for (auto &it : DriverShaders::instance().materialDescriptors) {
					const char* name = it->RootElement()->Attribute("name");
					bool is_selected = (mat->shaderName == name);
					if (ImGui::Selectable(name, is_selected))
						mat->shaderName = name;
					if (is_selected)
						ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
				}
				ImGui::EndCombo();
			}

			//List the Material UI Parameters for the Selected Shader
			tinyxml2::XMLDocument* materialDescriptor = NULL;
			for (auto& it : DriverShaders::instance().materialDescriptors) {
				if (mat->shaderName == it->RootElement()->Attribute("name")) {
					materialDescriptor = it.get();
					break;
				}
			}
			if (materialDescriptor) {
				for (tinyxml2::XMLElement* it = materialDescriptor->RootElement()->FirstChildElement("parameter"); it != NULL; it = it->NextSiblingElement("parameter")) {
					std::string_view name = it->Attribute("name");
					std::string_view type = it->Attribute("type");

					ImGui::PushID(name.data());

					materialFile::SCommand defaultCommand;
					defaultCommand.name = name.data();
					materialFile::SCommand* command = &defaultCommand;
					if (mat->findCommand(name.data()))
						command = mat->findCommand(name.data());

					bool added = false;

					if (type == "bool") {
						defaultCommand.type = 6;
						it->QueryBoolAttribute("defaultvalue", &defaultCommand.unks6);
						added = ImGui::Checkbox(name.data(), &command->unks6);
					} else if (type == "int") {
						defaultCommand.type = 5;
						it->QueryIntAttribute("defaultvalue", &defaultCommand.unks5);
						added = ImGui::InputInt(name.data(), &command->unks5);
					} else if (type == "float") {
						defaultCommand.type = 1;
						added = ImGui::InputFloat(name.data(), &command->unks1);
					} else if (type == "float2") {
						defaultCommand.type = 2;
						added = ImGui::InputFloat2(name.data(), &command->unks2.x);
					} else if (type == "float3") {
						defaultCommand.type = 3;
						added = ImGui::InputFloat3(name.data(), &command->unks3.x);
					} else if (type == "float4") {
						defaultCommand.type = 4;
						added = ImGui::InputFloat4(name.data(), &command->unks4.x);
					} else if (type == "color3") {
						defaultCommand.type = 3;
						added = ImGui::ColorEdit3(name.data(), &command->unks3.x);
					} else if (type == "color4") {
						defaultCommand.type = 4;
						added = ImGui::ColorEdit4(name.data(), &command->unks4.x);
					} else if (type == "sampler2D") {
						defaultCommand.type = 10;//TODO: ambiguous
						added = ImGui::InputText(name.data(), &command->path);
					} else if (type == "gradient") {
						ImGui::Text("TODO: Gradient");
					} else if (type == "samplerState") {
						defaultCommand.type = 7;
						defaultCommand.unks7 = it->Attribute("defaultValue");
						std::string value = command->unks7.getReverseName();
						if (ImGui::BeginCombo(name.data(), value.c_str())) {
							for (tinyxml2::XMLElement* it = DriverShaders::instance().samplerStates->RootElement()->FirstChildElement("samplerstate"); it != NULL; it = it->NextSiblingElement("samplerstate")) {
								const char* name = it->Attribute("name");
								bool is_selected = (value == name);
								if (ImGui::Selectable(name, is_selected)) {
									command->unks7 = name;
									added = true;
								}
								if (is_selected)
									ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}
					} else {
						ImGui::Text("Tell Jon to implement %s", type.data());
					}

					if (command != &defaultCommand) {
						ImGui::SameLine();
						if (ImGui::Button("Reset"))
							mat->deleteCommand(name.data());
					} else if (added)
						mat->commands.push_back(defaultCommand);

					ImGui::PopID();
				}
			}

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

	ImGui::EndChild();
	ImGui::End();
}
