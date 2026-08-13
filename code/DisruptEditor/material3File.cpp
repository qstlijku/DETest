#include "material3File.h"

#include <stdio.h>
#include <SDL_assert.h>
#include <SDL_log.h>
#include "IBinaryArchive.h"
#include "FileHandler.h"
#include "Serialization.h"
#include "ResourceLoader.h"
#include "xbtFile.h"
#include <NBCF.h>

std::vector<std::string> material3File::getTexturePaths() {
	std::vector<std::string> paths;
	for (auto& it : commands) {
		if (it.type == 8 || it.type == 9 || it.type == 10)
			paths.push_back(it.path);
	}

	return paths;
}

std::string material3File::getCommandPath(const char* name) {
	CStringID hash(name);

	for (Node& param : root.children)
	{
		uint32_t nam = param.getAttrValue<uint32_t>("name");
		if (nam == hash)
		{
			Attribute* attr = param.getAttribute("value");
			std::string val(attr->buffer.begin(), attr->buffer.end());
			return val;
		}
	}

	/*
	for (auto& it : commands) {
		if (it.name == hash && (it.type == 8 || it.type == 9 || it.type == 10))
			return it.path;
	}*/

	return std::string();
}

material3File::SCommand* material3File::findCommand(CStringID name) {
	for (auto& it : commands) {
		if (it.name == name)
			return &it;
	}
	return NULL;
}

void material3File::deleteCommand(CStringID name) {
	for (auto it = commands.begin(); it != commands.end(); ++it) {
		if (it->name == name) {
			commands.erase(it);
			return;
		}
	}
}

void material3File::bind(ID3D11DeviceContext* context) {
	auto diffuse = loadTexture(getCommandPath("DiffuseTexture1").c_str());

	ID3D11ShaderResourceView* views[] = {
			diffuse->pResource,
	};
	context->PSSetShaderResources(0, 1, views);
}

bool material3File::open(IBinaryArchive &fp) {
	fp.serialize(magic);
	SDL_assert_release(magic == 5062996);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(pad);
	fp.serialize(version);
	SDL_assert_release(version == 6 || version == 9);

	root = readFCB(fp.fp);

	auto val = getCommandPath("DiffuseTexture1");

	/*
	uint16_t count = commands.size();
	fp.serialize(count);
	commands.resize(count);
	for (uint16_t i = 0; i < count; ++i)
		commands[i].read(fp);
	*/
	return true;
}

void material3File::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(magic);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(version);

	REGISTER_MEMBER(commands);
}

void material3File::SCommand::read(IBinaryArchive & fp) {
	fp.serialize(type);
	if (type - 1 <= 0xA) {
		fp.serialize(unk1);
		fp.serialize(name);
	}

	switch (type) {//Switch 12 cases
	case 1:
		fp.serialize(unks1);
		break;
	case 2:
		fp.serialize(unks2);
		break;
	case 3:
		fp.serialize(unks3);
		break;
	case 4:
		fp.serialize(unks4);
		break;
	case 5:
		fp.serialize(unks5);
		break;
	case 6:
		fp.serialize(unks6);
		break;
	case 7:
		fp.serialize(unks7.id);
		break;
	case 8:
	case 9:
	case 10:
		fp.serialize(path);
		break;
	case 11:
		fp.serialize(unks11);
		//fp.serialize(unks11_2);
		break;
	case 0:
		break;
	default:
		SDL_assert_release(false);
		break;
	}

}

void material3File::SCommand::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(type);
	if (type - 1 <= 0xA) {
		REGISTER_MEMBER(unk1);
		REGISTER_MEMBER(name);
	}

	switch (type) {//Switch 12 cases
	case 1:
		ms.registerMember("Value", unks1);
		break;
	case 2:
		ms.registerMember("Value", unks2);
		break;
	case 3:
		ms.registerMember("Value", unks3);
		break;
	case 4:
		ms.registerMember("Value", unks4);
		break;
	case 5:
		ms.registerMember("Value", unks5);
		break;
	case 6:
		ms.registerMember("Value", unks6);
		break;
	case 7:
		ms.registerMember("Value", unks7);
		break;
	case 8:
	case 9:
	case 10:
		ms.registerMember("Value", path);
		break;
	case 11:
		ms.registerMember("Value1", unks11);
		ms.registerMember("Value2", unks11_2);
		break;
	default:
		break;
	}
}
