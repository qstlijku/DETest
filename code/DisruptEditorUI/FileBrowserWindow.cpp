#include "ImguiWindows.h"

#include "imgui.h"
#include "imgui_memory_editor.h"
#include "imgui_stdlib.h"
#include "Common.h"
#include "DB.h"
#include "FileHandler.h"
#include <portable-file-dialogs.h>
#include "ResourceLoader.h"
#include "materialFile.h"
#include "DriverShaders.h"
#include "xbgFile.h"
#include "xbtFile.h"
#include "batchFile.h"
#include <Serialization.h>
#include <SDL.h>
#include <fbxsdk.h>
#include <xbgMipFile.h>

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

			ID3D11DeviceContext *context = RenderInterface::instance().g_pd3dDeviceContext.Get();
			context->VSSetShader(RenderInterface::instance().model.pVertexShader, NULL, NULL);
			context->PSSetShader(RenderInterface::instance().model.pPixelShader, NULL, NULL);
			RenderInterface::instance().objectCB.Model = glm::mat4(1.f);
			xbg->draw(context, selLod);

			if (ImGui::Button("XML")) {
				std::string xml = serializeToXML(*xbg);
				SDL_SetClipboardText(xml.c_str());
			}
			if (ImGui::Button("Save XBG")) {
				SDL_RWops* fp = FH::openFileWrite(currentFile.c_str());
				SDL_RWwrite(fp, currentFileData.data(), 1, currentFileData.size());
				SDL_RWclose(fp);
			}
			if (ImGui::Button("Export")) {
				auto selection = pfd::save_file("Select a fbx to export to", "", {"FBX Files (fbx)", "*.fbx"}, true).result();
				if (!selection.empty()) {
					FbxManager* lSdkManager = FbxManager::Create();
					FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
					lSdkManager->SetIOSettings(ios);
					FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");

					FbxScene* lScene = FbxScene::Create(lSdkManager, currentFile.c_str());
					FbxNode* lRootNode = lScene->GetRootNode();

					//Add The Lods
					int count = 0;
					for (auto& lod : xbg->lods) {
						char name[15];
						snprintf(name, sizeof(name), "LOD%u", count);
						FbxNode* lChild = FbxNode::Create(lScene, name);
						lRootNode->AddChild(lChild);

						for (auto& mesh : lod.meshes) {
							if (mesh.primitiveType != 0)
								continue;

							// Create a node for our mesh in the scene.
							FbxNode* lMeshNode = FbxNode::Create(lScene, "meshNode");

							// Create a mesh.
							FbxMesh* lMesh = FbxMesh::Create(lScene, "mesh");

							// Set the node attribute of the mesh node.
							lMeshNode->SetNodeAttribute(lMesh);

							// Add the mesh node to the lod node in the scene.
							lChild->AddChild(lMeshNode);

							std::vector<FbxVector4> verticies;
							std::vector<FbxVector4> uvs;

							UINT offset = mesh.drawCall.vertexBufferByteOffset;
							UINT stride = mesh.vertexStride;
							uint8_t* vPtr = xbg->buffers[count].vertexData.data() + offset;
							uint16_t* iPtr = (uint16_t*)(xbg->buffers[count].indexData.data() + (mesh.drawCall.indexBufferStartIndex * 2));
							lMesh->InitControlPoints(mesh.drawCall.indexCount);
							FbxVector4* lControlPoints = lMesh->GetControlPoints();

							// Create UV for Diffuse channel
							FbxGeometryElementUV* lUVDiffuseElement = lMesh->CreateElementUV("DiffuseUV");
							FBX_ASSERT(lUVDiffuseElement != NULL);
							lUVDiffuseElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
							lUVDiffuseElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

							for (int i = 0; i < mesh.drawCall.indexCount; ++i) {
								uint16_t index = iPtr[i];

								uint8_t* ptr = vPtr + (stride * index);
								int16_t v[3];
								int16_t u[2];
								memcpy(v, ptr, sizeof(v));
								memcpy(u, ptr + 8, sizeof(u));

								glm::vec3 fv = glm::vec3(v[0], v[1], v[2]);
								fv *= xbg->geomParams.unk2;
								fv += xbg->geomParams.unk1;

								glm::vec2 fu = glm::vec2(u[0], u[1]);
								fu *= xbg->geomParams.unk5;
								fu += xbg->geomParams.unk4;

								lControlPoints[i] = FbxVector4(fv.x, fv.y, fv.z);
								lUVDiffuseElement->GetDirectArray().Add(FbxVector2(fu.x, fu.y));
							}
							lUVDiffuseElement->GetIndexArray().SetCount(mesh.drawCall.indexCount);

							for (int i = 0; i < mesh.drawCall.indexCount / 3; i++) {
								//we won't use the default way of assigning textures, as we have
								//textures on more than just the default (diffuse) channel.
								lMesh->BeginPolygon(-1, -1, false);

								for (int j = 0; j < 3; j++) {
									//this function points 
									lMesh->AddPolygon((i * 3) + j);
									lUVDiffuseElement->GetIndexArray().SetAt(i * 3 + j, j);
								}

								lMesh->EndPolygon();
							}

							//Write Material
							auto material = loadMaterial(xbg->materialResources.materials[mesh.matID].file.c_str());
							auto diffuse = loadTexture(material->getCommandPath("DiffuseTexture1").c_str());

							FbxSurfacePhong* lMaterial = FbxSurfacePhong::Create(lScene, xbg->materialResources.materials[mesh.matID].file.c_str());
							lMeshNode->AddMaterial(lMaterial);
							FbxFileTexture* lTexture = FbxFileTexture::Create(lScene, material->getCommandPath("DiffuseTexture1").c_str());
							lTexture->SetRelativeFileName(material->getCommandPath("DiffuseTexture1").c_str());
							lMaterial->Diffuse.ConnectSrcObject(lTexture);
						}

						++count;
					}

					bool lExportStatus = lExporter->Initialize(selection.c_str(), -1, lSdkManager->GetIOSettings());
					if (!lExportStatus) {
						SDL_Log("Call to FbxExporter::Initialize() failed.\n");
						SDL_Log("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
					}
					lExporter->Export(lScene);
					lExporter->Destroy();
				}
			}

			if (ImGui::Button("Import")) {
				auto selection = pfd::select_folder("Select the folder to import from").result();
				if (!selection.empty()) {
					std::string xml = serializeToXML(*xbg);
				}
			}

			displayImGui(*xbg);
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
			tinyxml2::XMLDocument* materialDescriptor = nullptr;
			for (auto& it : DriverShaders::instance().materialDescriptors) {
				if (mat->shaderName == it->RootElement()->Attribute("name")) {
					materialDescriptor = it.get();
					break;
				}
			}
			if (materialDescriptor) {
				for (tinyxml2::XMLElement* it = materialDescriptor->RootElement()->FirstChildElement("parameter"); it != nullptr; it = it->NextSiblingElement("parameter")) {
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
							for (tinyxml2::XMLElement* it = DriverShaders::instance().samplerStates->RootElement()->FirstChildElement("samplerstate"); it != nullptr; it = it->NextSiblingElement("samplerstate")) {
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
		} else if (type == "CSkeletonResource" || type == "lib" || type == "obj") {
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
