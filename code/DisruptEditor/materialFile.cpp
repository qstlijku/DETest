#include "materialFile.h"

#include <stdio.h>
#include <SDL_assert.h>
#include <SDL_log.h>
#include "IBinaryArchive.h"
#include "FileHandler.h"
#include "Serialization.h"
#include "ResourceLoader.h"
#include "xbtFile.h"

std::string materialFile::getCommandPath(const char* name) {
	CStringID hash(name);

	for (auto& it : commands) {
		if (it.name == hash && (it.type == 8 || it.type == 9 || it.type == 10))
			return it.path;
	}

	return std::string();
}

materialFile::SCommand* materialFile::findCommand(CStringID name) {
	for (auto& it : commands) {
		if (it.name == name)
			return &it;
	}
	return NULL;
}

void materialFile::deleteCommand(CStringID name) {
	for (auto it = commands.begin(); it != commands.end(); ++it) {
		if (it->name == name) {
			commands.erase(it);
			return;
		}
	}
}

void materialFile::bind(ID3D11DeviceContext* context) {
	auto& diffuse = loadTexture(getCommandPath("DiffuseTexture1").c_str());

	ID3D11ShaderResourceView* views[] = {
			diffuse->pResource,
	};
	context->PSSetShaderResources(0, 1, views);
}

bool materialFile::open(IBinaryArchive &fp) {
	fp.beginOffset = 0x20;

	fp.serialize(magic);
	SDL_assert_release(magic == 5062996);
	fp.serialize(version);
	SDL_assert_release(version == 7);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	//Game skips the above

	fp.markHeader();

	fp.serialize(name);
	fp.serialize(shaderName);

	initSettings.read(fp);

	uint16_t count = commands.size();
	fp.serialize(count);
	commands.resize(count);
	for (uint16_t i = 0; i < count; ++i)
		commands[i].read(fp);

	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVector<CMaterialResource::CGradient, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>, false>]
	if (fp.isReading()) {
		if (fp.tell() != fp.size())
			fp.serializeNdVector(gradients);
	} else {
		if (!gradients.empty()) {
			fp.serializeNdVector(gradients);
		}
	}

	fp.finish();

	return true;
}

void materialFile::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(magic);
	REGISTER_MEMBER(version);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);

	REGISTER_MEMBER(name);
	REGISTER_MEMBER(shaderName);

	REGISTER_MEMBER(initSettings);
	REGISTER_MEMBER(commands);
	REGISTER_MEMBER(gradients);
}

void materialFile::SInitSettings::read(IBinaryArchive & fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
}

void materialFile::SInitSettings::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(unk8);
}


void materialFile::CGradient::read(IBinaryArchive & fp) {
	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVector<ndVec_tpl<float, (int)4>, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>, false>]
	fp.serializeNdVector_pod(vecs);

	fp.serialize(id.id);
	fp.serialize(unk1);
	fp.serialize(unk2);
}

void materialFile::CGradient::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(vecs);
	REGISTER_MEMBER(id);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
}

void materialFile::SCommand::read(IBinaryArchive & fp) {
	fp.serialize(type);
	if (type - 1 <= 0xA) {
		fp.serialize(unk1);
		fp.serialize(name.id);
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
		fp.serialize(unks11.id);
		fp.serialize(unks11_2);
		break;
	case 0:
		break;
	default:
		SDL_assert_release(false);
		break;
	}

}

void materialFile::SCommand::registerMembers(MemberStructure & ms) {
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
