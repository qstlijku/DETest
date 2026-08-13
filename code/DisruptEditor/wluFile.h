#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <atomic>
#include <stdint.h>
#include "tinyxml2.h"
#include <string>
#include "Vector.h"
#include "NBCF.h"
#include <d3d11.h>
#include "DDRenderInterface.h"
#include <CLODictionary.h>
#include <WorldRenderer.h>
#include <glm/gtx/quaternion.hpp>

class IBinaryArchive;
class SplineLoftHiRes;
class SplineLoftLowRes;

#pragma pack(push, 1)
struct wluHeader {
	uint32_t magic;
	uint32_t size;
	uint32_t unknown1;//Always 0 or 1 or 2 or 3
	uint32_t unknown2;//Always 0
};

struct qualityHeader {
	char magic[4];
	uint16_t size;
	uint8_t unknown[10];
};

struct roadHeader {
	char magic[4];
	uint32_t size;
	uint8_t unknown[8];
};
#pragma pack(pop)

class wluFile {
public:
	wluFile() {};
	bool open(SDL_RWops* fp);
	void serialize(SDL_RWops* fp);

	void draw(bool drawImgui = false);
	
	Node root;
	CCityLifeDataAndStateHandler cityLifeObjectManagerData;

	std::string shortName; //ex. wlu_data_01_loop_vigilante_01

	bool openWD1(IBinaryArchive &fp);
	bool openWD2(IBinaryArchive &fp);

	bool isWD2 = false;
	wluHeader wluhead;
	Vector<uint8_t> extraData;
	void handleHeaders(IBinaryArchive &fp, size_t size);
	Node* selectedEntity = NULL;

	//Drawing
	std::shared_ptr<WorldRenderer::Bucket> gBucket;
	enum WluType {
		WLU_NEAR, WLU_FAR, WLU_WORLD, WLU_OTHER
	};
	WluType wluType = WLU_OTHER;

	//Only applies for WLU_NEAR, WLU_FAR
	glm::vec2 bbMin = glm::vec2(0), bbMax = glm::vec2(0);
	uint32_t sectorID;
	bool forceRender = false;
	std::atomic_bool renderDirty = true;
	std::vector<std::shared_ptr<SplineLoftHiRes>> hiResSplines;
	std::vector<std::shared_ptr<SplineLoftLowRes>> lowResSplines;

	static glm::mat4 posRotToMat(const glm::vec3& pos, glm::vec3& rot);
	static glm::vec3 matToRot(const glm::mat4& mat);
};

