#pragma once

#include <stdint.h>
#include <NBCF.h>
#include "Vector.h"
#include <string>
#include "CStringID.h"
#include "glm/glm.hpp"

class IBinaryArchive;
class MemberStructure;
struct ID3D11DeviceContext;

class material3File {
public:
	//Header
	uint32_t magic; // 54 41 4D 00
	uint32_t unk2;
	uint32_t unk3;
	uint32_t unk4;
	uint16_t pad;
	uint16_t version; //9

	Node root;

	struct SCommand {
		uint8_t type = 0;

		//Only for types greater than 10?
		uint8_t unk1 = 0;
		CStringID name;

		//1: float
		float unks1;

		//2: float2,
		glm::vec2 unks2;

		//3: float3, color3
		glm::vec3 unks3;

		//4: float4, color4
		glm::vec4 unks4;

		//5: int
		int32_t unks5;

		//6: bool
		bool unks6;

		//7: samplerState
		CStringID unks7;

		//For 8-10
		//8: sampler2D
		std::string path;

		//11: sampler2D, eg. RaindropSplashesTexture
		CStringID unks11;
		int32_t unks11_2;

		void read(IBinaryArchive &fp);
		void registerMembers(MemberStructure &ms);
	};
	Vector<SCommand> commands;

	std::vector<std::string> getTexturePaths();

	std::string getCommandPath(const char* name);
	SCommand* findCommand(CStringID name);
	void deleteCommand(CStringID name);
	void bind(ID3D11DeviceContext* context);

	bool open(IBinaryArchive &fp);
	void registerMembers(MemberStructure &ms);
};
