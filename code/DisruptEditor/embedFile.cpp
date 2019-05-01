#include "embedFile.h"

#include "IBinaryArchive.h"
#include <SDL_assert.h>
#include "Serialization.h"

void embedFile::open(IBinaryArchive & fp) {
	fp.padding = fp.PADDING_NONE;

	uint32_t magic = 0xAE27FBCC;
	fp.serialize(magic);
	SDL_assert_release(magic == 0xAE27FBCC);

	//If 0xAE27FBCC, then CEmbeddedResourceContainer::ProcessEmbbeddedData((void *,int))
	if (magic == 0xAE27FBCC) {
		fp.serializeNdVectorExternal(unk1);
		db.open(fp);
		SDL_assert_release(fp.tell() == fp.size());
	}

	//If 0xA2363453, then CEmbeddedResourceContainer::ProcessVectorRessourceData((void *,int))
	//Unused
}

void embedFile::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(db);
}

void embedFile::Unk1::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
}

void embedFile::Unk1::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
}
