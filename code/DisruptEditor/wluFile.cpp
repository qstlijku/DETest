#include "wluFile.h"

#include <stdio.h>
#include <string.h>
#include <SDL_assert.h>
#include <SDL_log.h>
#include <stdlib.h>

#include "tinyxml2.h"
#include "Hash.h"
#include "Common.h"
#include "imgui.h"
#include "Entity.h"
#include "ImGuizmo.h"
#include "DDRenderInterface.h"
#include "FileHandler.h"
#include "DB.h"
#include "Types.h"
#include "IBinaryArchive.h"
#include <glm\gtx\euler_angles.hpp>

bool wluFile::open(SDL_RWops* fp) {
	if (!fp)
		return false;

	CBinaryArchiveReader reader(fp);
	reader.padding = reader.PADDING_NONE;

	reader.serialize(wluhead.magic);
	reader.serialize(wluhead.size);
	reader.serialize(wluhead.unknown1);
	reader.serialize(wluhead.unknown2);

	bool ret;
	if (wluhead.magic == 1111577413) {
		//WD1 File
		ret = openWD1(reader);
		isWD2 = false;
	} else if (wluhead.magic == 4129362901) {
		//WD2 File
		ret = openWD2(reader);
		isWD2 = true;
	}

	//Read CityLifeObjects
	Node* CityLifeObjects = root.findFirstChild("CityLifeObjects");
	if (CityLifeObjects) {
		Attribute* CCityLifeObjectManagerData = CityLifeObjects->getAttribute("CCityLifeObjectManagerData");
		if (CCityLifeObjectManagerData) {
			uint32_t InPlaceOffset = CityLifeObjects->getAttrValue<uint32_t>("InPlaceOffset");
			SDL_RWops* cloFP = SDL_RWFromConstMem(CCityLifeObjectManagerData->buffer.data(), CCityLifeObjectManagerData->buffer.size());
			CBinaryArchiveReader reader(cloFP);
			reader.markHeader();
			reader.markInPlaceOffset(InPlaceOffset);
			cityLifeObjectManagerData.read(reader);
			SDL_RWclose(cloFP);
		}
	}

	return ret;
}

bool wluFile::openWD1(IBinaryArchive &fp) {
	SDL_assert_release(wluhead.unknown1 == 3 || wluhead.unknown1 == 0 || wluhead.unknown1 == 1 || wluhead.unknown1 == 2);
	SDL_assert_release(wluhead.unknown2 == 0);

	size_t size = fp.size() - sizeof(wluhead);

	//Pad size to 4 bytes
	//TODO Figure out size
	//SDL_assert_release(wluhead.base.size == size || wluhead.base.size == size-1 || wluhead.base.size == size - 2 || wluhead.base.size == size - 3);

	//2296 size
	//2265 wlu base size + 16

	root = readFCB(fp.fp);

	fp.pad(4);
	//SDL_RWseek(fp, wluhead.size + sizeof(wluhead), RW_SEEK_SET);

	/*size_t offset = fp.tell();
	size_t extraBegin = offset;
	if (offset != size + sizeof(wluhead)) {
		handleHeaders(fp, size + sizeof(wluhead));

		offset = fp.tell();
		SDL_assert_release(offset == size + sizeof(wluhead));
	}

	//Read in Extra Data
	extraData.resize(size + sizeof(wluhead) - extraBegin);
	if (!extraData.empty()) {
		SDL_RWseek(fp.fp, extraBegin, RW_SEEK_SET);
		fp.memBlock(extraData.data(), 1, extraData.size());
	}*/

	//Handle .embed
	/*fp = fopen((filename + ".embed").c_str(), "rb");
	if (fp) {
	uint32_t magic, size;
	fread(&magic, sizeof(magic), 1, fp);
	fread(&size, sizeof(size), 1, fp);

	fseek(fp, 1, SEEK_CUR);
	fseek(fp, size * 37, SEEK_CUR);
	SDL_assert_release(feof(fp));

	fclose(fp);
	}*/

	return true;
}

bool wluFile::openWD2(IBinaryArchive &fp) {
	SDL_assert_release(wluhead.magic == 4129362901);
	SDL_assert_release(wluhead.unknown2 == 0);

	root = readFCB(fp.fp);

	return true;
}

void wluFile::handleHeaders(IBinaryArchive &fp, size_t size) {
	//Read Magic
	char magic[5];
	fp.memBlock(magic, 4, 1);
	magic[4] = '\0';
	SDL_RWseek(fp.fp, -4, RW_SEEK_CUR);

	//LAUQ - LoadQuality
	//ROAD - 

	if (magic == std::string("DAOR")) {
		roadHeader road;
		fp.memBlock(&road, sizeof(road), 1);
		SDL_RWseek(fp.fp, road.size, RW_SEEK_CUR);
		SDL_Log("Road %i\n", road.size);
		fp.pad(16);
	} else if (magic == std::string("LAUQ")) {
		qualityHeader qual;
		fp.memBlock(&qual, sizeof(qual), 1);
		SDL_RWseek(fp.fp, qual.size, RW_SEEK_CUR);
		SDL_Log("Qual %i\n", qual.size);
	} else {
		size_t offset = fp.tell();
		SDL_assert_release(false);
	}

	if (fp.tell() != size) {
		handleHeaders(fp, size);
	}
}

glm::mat4 wluFile::posRotToMat(const glm::vec3& pos, glm::vec3& rot) {
	glm::mat4 ret = glm::translate(glm::mat4(1), pos);

	float cosX = cosf(rot.x);
	float sinX = sinf(rot.x);
	float cosY = cosf(rot.y);
	float sinY = sinf(rot.y);
	float cosZ = cosf(rot.z);
	float sinZ = sinf(rot.z);

	glm::mat4 Rotation;
	Rotation[0][0] = cosY * cosZ;
	Rotation[0][1] = cosY * sinZ;
	Rotation[0][2] = -sinY;
	Rotation[0][3] = 0.f;
	Rotation[1][0] = sinY * cosZ * sinX - cosX * sinZ;
	Rotation[1][1] = sinY * sinZ * sinX + cosX * cosZ;
	Rotation[1][2] = cosY * sinX;
	Rotation[1][3] = 0;
	Rotation[2][0] = sinY * cosZ * cosX + sinX * sinZ;
	Rotation[2][1] = sinY * sinZ * cosX - sinX * cosZ;
	Rotation[2][2] = cosY * cosX;
	Rotation[2][3] = 0;
	Rotation[3][0] = 0;
	Rotation[3][1] = 0;
	Rotation[3][2] = 0;
	Rotation[3][3] = 1;

	ret *= Rotation;
	return ret;
}

glm::vec3 wluFile::matToRot(const glm::mat4& mat) {
	const float* m = &mat[0][0];

	float a = -1.f;
	if (m[2] - 1.f <= 0.f)
		a = -m[2];
	float b = asinf(a);
	if(0.01 <= glm::abs(glm::abs(b) - 1.5707964f))
		return glm::vec3(atan2f(m[6], m[10]), b, atan2f(m[1], m[0]));
	else
		return glm::vec3(0.f, b, atan2f(m[4], m[5]));
}

void wluFile::serialize(SDL_RWops* fp) {
	CBinaryArchiveWriter aw(fp);
	
	if (isWD2) {
		SDL_RWwrite(fp, &wluhead, sizeof(wluhead), 1);
		writeFCBB(fp, root);
		wluhead.size = SDL_RWtell(fp) - sizeof(wluhead);
		aw.pad(16);
		SDL_RWseek(fp, 0, RW_SEEK_SET);
		SDL_RWwrite(fp, &wluhead, sizeof(wluhead), 1);
	} else {
		wluhead.magic = 1111577413;
		//wluhead.base.unknown1 = wluhead.base.unknown2 = 0;

		SDL_RWwrite(fp, &wluhead, sizeof(wluhead), 1);

		writeFCBB(fp, root);

		SDL_RWseek(fp, 0, RW_SEEK_END);
		aw.pad(4);
		wluhead.size = SDL_RWtell(fp) - sizeof(wluhead);

		//Write Extra Data
		//fwrite(extraData.data(), 1, extraData.size(), fp);

		SDL_RWseek(fp, 0, RW_SEEK_SET);
		SDL_RWwrite(fp, &wluhead, sizeof(wluhead), 1);
	}
}


void EditTransform(const float *cameraView, float *cameraProjection, float* matrix) {
	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
	static bool useSnap = false;
	static float snap[3] = { 1.f, 1.f, 1.f };

	if (ImGui::IsKeyPressed(ImGuiKey_Z))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_E))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;

	if (mCurrentGizmoOperation != ImGuizmo::SCALE) {
		if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
			mCurrentGizmoMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
			mCurrentGizmoMode = ImGuizmo::WORLD;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_S))
		useSnap = !useSnap;
	ImGui::Checkbox("", &useSnap);
	ImGui::SameLine();

	switch (mCurrentGizmoOperation) {
		case ImGuizmo::TRANSLATE:
			ImGui::InputFloat3("Snap", &snap[0]);
			break;
		case ImGuizmo::ROTATE:
			ImGui::InputFloat("Angle Snap", &snap[0]);
			break;
	}
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL);
}

void wluFile::draw(bool drawImgui) {
	Node *Entities = root.findFirstChild("Entities");
	if (!Entities) return;

	//Draw List of Entites
	ImGui::PushItemWidth(-1.f);
	static char searchWluBuffer[255] = { 0 };
	ImGui::InputText("##Search", searchWluBuffer, sizeof(searchWluBuffer));
	bool foundSelectedEntity = false;
	if (ImGui::BeginListBox("##Entity List")) {
		for (Node& entity : Entities->children) {
			Attribute* hidName = entity.getAttribute("hidName");

			char tempName[255] = { '\0' };
			snprintf(tempName, sizeof(tempName), "%s##%p", hidName->buffer.data(), &entity);

			if (std::string(tempName).find(searchWluBuffer) == std::string::npos) continue;

			bool selected = &entity == selectedEntity;

			if (ImGui::Selectable(tempName, selected, ImGuiSelectableFlags_AllowDoubleClick)) {
				selectedEntity = &entity;
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
				glm::vec3 pos = entity.getAttrValue<glm::vec3>("hidPos");
				RenderInterface::instance().camera.location = pos + glm::vec3(1, 1, 1);
				RenderInterface::instance().camera.lat = -36.8799820f;
				RenderInterface::instance().camera.lon = -500.110077;
			}

			if (&entity == selectedEntity)
				foundSelectedEntity = true;
		}
		ImGui::EndListBox();
	}
	ImGui::PopItemWidth();

	char imGuiBuffer[1024];

	if (!foundSelectedEntity)
		selectedEntity = NULL;

	if(selectedEntity) {
		ImGui::Separator();
		Node &entity = *selectedEntity;

		bool didChange = false;

		Attribute *hidName = entity.getAttribute("hidName");
		Attribute *hidPos = entity.getAttribute("hidPos");
		glm::vec3 pos = entity.getAttrValue<glm::vec3>("hidPos");
		glm::vec3 angles = entity.getAttrValue<glm::vec3>("hidAngles");
		glm::mat4 matrix = wluFile::posRotToMat(pos, angles);
		RenderInterface &renderInterface = RenderInterface::instance();
		EditTransform(&renderInterface.sceneCB.View[0][0], &renderInterface.sceneCB.Projection[0][0], &matrix[0][0]);
		pos = glm::vec3(matrix[3]);
		angles = matToRot(matrix);
		entity.setAttrValue<glm::vec3>("hidPos", pos);
		entity.setAttrValue<glm::vec3>("hidAngles", angles);
		ImGui::Separator();

		//Iterate through Entity Attributes
		Attribute *ArchetypeGuid = entity.getAttribute("ArchetypeGuid");
		if (ArchetypeGuid) {
			uint32_t uid = Hash::getFilenameHash((const char*)ArchetypeGuid->buffer.data());
			char temp[40];
			snprintf(temp, sizeof(temp), "UID: %u", uid);
			if (ImGui::Selectable(temp)) {
				snprintf(temp, sizeof(temp), "%u", uid);
				ImGui::SetClipboardText(temp);
			}
		}

		for (Attribute &attr : entity.attributes) {
			char name[1024];
			snprintf(name, sizeof(name), "%s##%p", attr.name.getReverseName().c_str(), &attr);

			Types::Type type = Types::getHashType(attr.name.id);

			switch (type) {
				case Types::STRING:
				{
					char temp[1024];
					strncpy(temp, (char*)attr.buffer.data(), sizeof(temp));
					if (ImGui::InputText(name, temp, sizeof(temp))) {
						attr.buffer.resize(strlen(temp) + 1);
						strcpy((char*)attr.buffer.data(), temp);
					}
					break;
				}
				case Types::FLOAT:
					ImGui::DragFloat(name, (float*)attr.buffer.data());
					break;
				case Types::UINT64:
					ImGui::DragScalar(name, ImGuiDataType_U64, (uint64_t*)attr.buffer.data(), 1);
					break;
				case Types::VEC2:
					ImGui::DragFloat2(name, (float*)attr.buffer.data());
					break;
				case Types::VEC3:
					ImGui::DragFloat3(name, (float*)attr.buffer.data());
					break;
				case Types::VEC4:
					ImGui::DragFloat4(name, (float*)attr.buffer.data());
					break;
				case Types::BOOL:
					if (attr.buffer.size() == 0) {
						bool a = false;
						ImGui::Checkbox(name, &a);
						if (a) {
							attr.buffer.resize(1);
							attr.buffer[0] = 1;
						}
					}
					else {
						ImGui::Checkbox(name, (bool*)attr.buffer.data());
					}
					break;
				default:
					ImGui::LabelText(name, "BinHex %u", attr.buffer.size());
					break;
			}
		}

		Node* PatrolDescription = entity.findFirstChild("PatrolDescription");
		if (PatrolDescription && ImGui::TreeNode("PatrolDescription")) {
			Node* PatrolPointList = PatrolDescription->findFirstChild("PatrolPointList");
			for (Node &PatrolPoint : PatrolPointList->children) {
				snprintf(imGuiBuffer, sizeof(imGuiBuffer), "##%p", &PatrolPoint);
				ImGui::DragFloat3(imGuiBuffer, (float*)PatrolPoint.getAttribute("vecPos")->buffer.data());
			}
			ImGui::TreePop();
		}

		const ImGuiIO& io = ImGui::GetIO();
		if(io.WantCaptureKeyboard || io.WantCaptureMouse)
			renderDirty = true;
	}
}
