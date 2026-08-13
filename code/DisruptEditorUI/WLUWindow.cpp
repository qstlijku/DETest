#include "ImguiWindows.h"

#include "Common.h"
#include "DDRenderInterface.h"
#include "debug_draw.hpp"
#include "Entity.h"
#include "FileHandler.h"
#include "glm/glm.hpp"
#include "Hash.h"
#include "imgui.h"
#include "World.h"
#include <SDL_log.h>

static std::shared_ptr<wluFile> currentWlu;

void UI::displayWLU() {
	RenderInterface& renderInterface = RenderInterface::instance();

	//Draw Layer Window
	ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(1150.f, 5.f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Layers")) {
		//Wlu List
		ImGui::PushItemWidth(-1.f);
		static char searchWluBuffer[255] = { 0 };
		ImGui::InputText("##SearchWLU", searchWluBuffer, sizeof(searchWluBuffer));

		ImVec2 size = ImGui::GetWindowContentRegionMax();
		size.y -= 75;
		size.x -= 5;
		if (ImGui::BeginListBox("##WLU List")) {
			for (auto it = world.wlus.begin(); it != world.wlus.end(); ++it) {
				if (it->first.find(searchWluBuffer) != std::string::npos) {
					bool selected = currentWlu == it->second;
					if (ImGui::Selectable(it->first.c_str(), selected))
						currentWlu = it->second;
				}
			}
			ImGui::EndListBox();
		}
		ImGui::PopItemWidth();

		if (currentWlu) {
			wluFile& wlu = *currentWlu;

			if (ImGui::Button("Save")) {
				SDL_RWops* fp = FH::openFileWrite("worlds/" + settings.worldName + "/generated/wlu/" + wlu.shortName + "." + settings.wluExtension);
				wlu.serialize(fp);
				SDL_RWclose(fp);
			}
			ImGui::SameLine();
			std::string xmlFileName = wlu.shortName + ".xml";
			if (ImGui::Button("XML")) {
				FILE* fp = fopen(xmlFileName.c_str(), "wb");
				tinyxml2::XMLPrinter printer(fp);
				wlu.root.serializeXML(printer);
				fclose(fp);
			}
			ImGui::SameLine();
			if (ImGui::Button("Import XML")) {
				tinyxml2::XMLDocument doc;
				doc.LoadFile(xmlFileName.c_str());
				wlu.root.deserializeXML(doc.RootElement());
			}
			ImGui::SameLine();
			ImGui::Checkbox("Draw", &wlu.forceRender);
		}

		if (currentWlu) {
			wluFile& wlu = *currentWlu;
			wlu.draw();

			Node* Entities = wlu.root.findFirstChild("Entities");
			if (Entities) {

				for (Node& entityRef : Entities->children) {
					bool needsCross = true;

					Node* entityPtr = &entityRef;
					Attribute* ArchetypeGuid = entityRef.getAttribute("ArchetypeGuid");
					if (ArchetypeGuid) {
						uint32_t uid = Hash::getFilenameHash((const char*)ArchetypeGuid->buffer.data());
						entityPtr = findEntityByUID(uid);
						if (!entityPtr) {
							SDL_Log("Could not find %s\n", ArchetypeGuid->buffer.data());
							SDL_assert_release(false && "Could not lookup entity by archtype, check that dlc_solo is loaded first before other packfiles");
							entityPtr = &entityRef;
						}
					}
					Node& entity = *entityPtr;

					Attribute* hidName = entity.getAttribute("hidName");
					glm::vec3 pos = entity.getAttrValue<glm::vec3>("hidPos");
					glm::vec3 angles = entity.getAttrValue<glm::vec3>("hidAngles");

					//
					Node* hidBBox = entity.findFirstChild("hidBBox");

					Node* Components = entity.findFirstChild("Components");
					SDL_assert_release(Components);

					Node* CGraphicComponent = Components->findFirstChild("CGraphicComponent");
					if (CGraphicComponent) {
						Attribute* XBG = CGraphicComponent->getAttribute(0x3182766C);

						/*if (XBG && XBG->buffer.size() > 5) {
							auto &model = loadXBG((char*)XBG->buffer.data());
							renderInterface.model.use();

							glm::mat4 modelMatrix = glm::translate(glm::mat4(1), pos);
							modelMatrix = glm::rotate(modelMatrix, angles.x, glm::vec3(1, 0, 0));
							modelMatrix = glm::rotate(modelMatrix, angles.y, glm::vec3(0, 1, 0));
							modelMatrix = glm::rotate(modelMatrix, angles.z, glm::vec3(0, 0, 1));

							glm::mat4 MVP = renderInterface.VP * modelMatrix;
							glUniformMatrix4fv(renderInterface.model.uniforms["MVP"], 1, GL_FALSE, &MVP[0][0]);
							model.draw();
						}*/
					}

					Node* CProximityTriggerComponent = Components->findFirstChild("CProximityTriggerComponent");
					if (CProximityTriggerComponent) {
						needsCross = false;
						glm::vec3 extent = *(glm::vec3*)CProximityTriggerComponent->getAttribute("vectorSize")->buffer.data();
						dd::box(&pos.x, red, extent.x, extent.y, extent.z);
					}

					if (hidBBox && false) {
						glm::vec3 boxMin = *((glm::vec3*)hidBBox->getAttribute("vectorBBoxMin")->buffer.data());
						glm::vec3 boxMax = *((glm::vec3*)hidBBox->getAttribute("vectorBBoxMax")->buffer.data());
						glm::vec3 boxExtent = boxMax - boxMin;
						dd::box(&pos.x, blue, boxExtent.x, boxExtent.y, boxExtent.z);
					}

					Node* PatrolDescription = entity.findFirstChild("PatrolDescription");
					if (PatrolDescription) {
						needsCross = false;
						Node* PatrolPointList = PatrolDescription->findFirstChild("PatrolPointList");

						glm::vec3 last;
						for (Node& PatrolPoint : PatrolPointList->children) {
							glm::vec3 pos = *(glm::vec3*)PatrolPoint.getAttribute("vecPos")->buffer.data();

							if (last != glm::vec3())
								dd::line(&last[0], &pos[0], red);
							else
								dd::projectedText((char*)hidName->buffer.data(), &pos.x, red, &renderInterface.sceneCB.ViewProjection[0][0], 0, 0, renderInterface.sceneCB.windowSize.x, renderInterface.sceneCB.windowSize.y, 0.5f);
							last = pos;
						}
					}

					Node* RaceDescription = entity.findFirstChild("RaceDescription");
					if (RaceDescription) {
						needsCross = false;
						Node* RacePointList = RaceDescription->findFirstChild("RacePointList");

						glm::vec3 last;
						for (Node& RacePoint : RacePointList->children) {
							glm::vec3 pos = *(glm::vec3*)RacePoint.getAttribute("vecPos")->buffer.data();
							float fShortcutRadius = *(float*)RacePoint.getAttribute("fShortcutRadius")->buffer.data();

							dd::sphere((float*)& pos.x, red, fShortcutRadius);

							if (last != glm::vec3())
								dd::line(&last[0], &pos[0], red);
							else
								dd::projectedText((char*)hidName->buffer.data(), &pos.x, red, &renderInterface.sceneCB.ViewProjection[0][0], 0, 0, renderInterface.sceneCB.windowSize.x, renderInterface.sceneCB.windowSize.y, 0.5f);
							last = pos;
						}
					}

					if (glm::distance(pos, renderInterface.camera.location) < settings.drawDistance)
						dd::projectedText((char*)hidName->buffer.data(), &pos.x, white, &renderInterface.sceneCB.ViewProjection[0][0], 0, 0, renderInterface.sceneCB.windowSize.x, renderInterface.sceneCB.windowSize.y, 0.5f);
					if (needsCross)
						dd::cross(&pos.x, 0.25f);

				}
			}
		}
	}
	ImGui::End();
}
