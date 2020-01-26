#include "realTreeFile.h"

#include "IBinaryArchive.h"

void realTreeFile::open(IBinaryArchive& fp) {
	fp.padding = fp.PADDING_NONE;

	fp.serializeConstant<uint32_t>(1414677829);//TREE
	fp.serializeConstant<uint32_t>(8);

	//RTcManager::LoadSkeletal((RTsSkeletalData *,void *,int))
	for (int i = 0; i < unk1.size(); ++i)
		fp.serialize(unk1[i]);

	SDL_assert_release(unk1[0] == 0x113);//Game checks
	SDL_assert_release((unk1[2] & 0xFFFF0000) >> 16 <= 3);//Game checks

	if (unk1[3] != 0) {
		unk2.resize(unk1[3]);
		fp.memBlock(unk2.data(), 1, unk1[3]);
		SDL_assert_release(false);
	}

	unk3.resize(unk1[4]);
	fp.memBlock(unk3.data(), 1, unk1[4]);
	//RTcManager::SetSkeletalPointer(RTcSkeleton *, char *&, char **)

	//Read Materials
	uint32_t count = materials.size();
	fp.serialize(count);
	materials.resize(count);
	for (uint32_t i = 0; i < count; ++i) {
		fp.serializeConstant<uint32_t>(i);
		fp.serialize(materials[i]);
	}
	SDL_assert_release(SDL_RWtell(fp.fp) == SDL_RWsize(fp.fp));
}
