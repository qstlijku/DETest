#include "CResourceDataBase.h"

#include "Serialization.h"

void CResourceDataBase::open(IBinaryArchive& fp) {
	fp.padding = fp.PADDING_NONE;
	fp.serializeNdVector(unk1);

	uint32_t size = files.size();
	fp.serialize(size);
	files.resize(size);
	for (uint32_t i = 0; i < size; ++i) {
		fp.serialize(files[i].file);
	}

	size = files.size();
	fp.serialize(size);
	SDL_assert_release(size == files.size());
	for (uint32_t i = 0; i < size; ++i) {
		fp.serialize(files[i].refType);
	}

	fp.serializeNdVector(types);

	for (auto& it : unk1)
		SDL_assert_release(it.unk1 < files.size());

	for (auto& it : unk1)
		SDL_assert_release(it.unk2 < files.size());

	for (auto& it : files)
		SDL_assert_release(it.refType < types.size());
}

void CResourceDataBase::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(files);
	REGISTER_MEMBER(types);
}

void CResourceDataBase::Unk1::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
}

void CResourceDataBase::Unk1::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
}

void CResourceDataBase::ResourceFile::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(file);
	REGISTER_MEMBER(refType);
}
